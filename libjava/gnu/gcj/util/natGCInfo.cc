/* natGCInfo.cc -- Native portion of support for creating heap dumps.
   Copyright (C) 2007  Free Software Foundation

   This file is part of libgcj.

   This software is copyrighted work licensed under the terms of the
   Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
   details.  */


#include <config.h>

#include <gcj/cni.h>

#include <gnu/gcj/util/GCInfo.h>

#ifdef HAVE_PROC_SELF_MAPS
//
// If /proc/self/maps does not exist we assume we are doomed and do nothing.
//
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "gc/gc.h"
#include "gc/gc_gcj.h"
#include "gc/gc_mark.h"

static int gc_ok = 1;

struct gc_debug_info
{
  int used;
  int free;
  int wasted;
  int blocks;
  FILE* fp;
};

static void
GC_print_debug_callback(GC_hblk_s *h, GC_word user_data)
{
  size_t bytes;

  gc_debug_info *pinfo = (gc_debug_info *)user_data;

  fprintf(pinfo->fp, "ptr = %#lx, kind = %d, size = %zd, marks = %d\n",
	  (unsigned long)h, GC_get_kind_and_size(h, &bytes), bytes, GC_count_set_marks_in_hblk(h));
}

struct print_hblkfl_s {
  FILE *fp;
  GC_word total_free;
  int prev_index;
};

static void GC_CALLBACK print_hblkfl_file_item(struct GC_hblk_s *h, int i,
					       GC_word client_data)
{
  GC_word sz;
  print_hblkfl_s *pdata = (print_hblkfl_s *)client_data;

  if (pdata->prev_index != i) {
    fprintf(pdata->fp, "Free list %d:\n", i);
    pdata->prev_index = i;
  }

  sz = GC_size(h);
  fprintf(pdata->fp, "\t0x%lx size %lu ", (unsigned long)h,
	   (unsigned long)sz);
  pdata->total_free += sz;

  if (GC_is_black_listed(h, GC_get_hblk_size()) != 0)
    fprintf(pdata->fp, "start black listed\n");
  else if (GC_is_black_listed(h, sz) != 0)
    fprintf(pdata->fp, "partially black listed\n");
  else
    fprintf(pdata->fp, "not black listed\n");
}

static void
GC_print_hblkfreelist_file(FILE *fp)
{
  print_hblkfl_s data;
  data.fp = fp;
  data.total_free = 0;
  data.prev_index = -1;
    
  fprintf(fp, "---------- Begin free map ----------\n");
  GC_iterate_free_hblks(print_hblkfl_file_item, (GC_word)&data);
  fprintf(fp, "Total of %lu bytes on free list\n",
	   (unsigned long)data.total_free);
  fprintf(fp, "---------- End free map ----------\n");
}

static int GC_dump_count = 1;

static void
GC_print_debug_info_file(FILE* fp)
{
  gc_debug_info info;

  memset(&info, 0, sizeof info);
  info.fp = fp;

  if (gc_ok)
    GC_gcollect();
  fprintf(info.fp, "---------- Begin block map ----------\n");
  GC_apply_to_all_blocks(GC_print_debug_callback, (GC_word)(void*)(&info));
  //fprintf(fp, "#Total used %d free %d wasted %d\n", info.used, info.free, info.wasted);
  //fprintf(fp, "#Total blocks %d; %dK bytes\n", info.blocks, info.blocks*4);
  fprintf(info.fp, "---------- End block map ----------\n");

  //fprintf(fp, "\n***Free blocks:\n");
  //GC_print_hblkfreelist();
}

namespace
{
  class  __attribute__ ((visibility ("hidden"))) GC_enumerator
  {
  public:
    GC_enumerator(const char *name);
    void enumerate();
  private:
    FILE* fp;
    int bytes_fd;

    void print_address_map();
    void enumerate_callback(GC_hblk_s *h);
    static void enumerate_callback_adaptor(GC_hblk_s *h, GC_word dummy);
  };
}

GC_enumerator::GC_enumerator(const char *name)
{
  bytes_fd = -1;
  fp = fopen (name, "w");
  if (!fp)
    {
      printf ("GC_enumerator failed to open [%s]\n", name);
      return;
    }
  printf ("GC_enumerator saving summary to [%s]\n", name);

  // open heap file
  char bytes_name[strlen(name) + 10];
  sprintf (bytes_name, "%s.bytes", name);
  bytes_fd = open (bytes_name, O_CREAT|O_TRUNC|O_WRONLY, 0666);
  if (bytes_fd <= 0)
    {
      printf ("GC_enumerator failed to open [%s]\n", bytes_name);
      return;
    }
  printf ("GC_enumerator saving heap contents to [%s]\n", bytes_name);
}

/*
  sample format of /proc/self/maps

  0063b000-00686000 rw-p 001fb000 03:01 81993      /avtrex/bin/dumppropapp
  00686000-0072e000 rwxp 00000000 00:00 0 

  These are parsed below as:
  start   -end      xxxx xxxxxxxx  a:b xxxxxxxxxxxxxxx

*/


void
GC_enumerator::print_address_map()
{
  FILE* fm;
  char buffer[128];

  fprintf(fp, "---------- Begin address map ----------\n");

  fm = fopen("/proc/self/maps", "r");
  if (fm == NULL)
    {
#ifdef HAVE_STRERROR_R
      if (0 == strerror_r (errno, buffer, sizeof buffer))
        fputs (buffer, fp);
#else
      fputs (strerror (errno), fp);
#endif
    }
  else
    {
      while (fgets (buffer, sizeof buffer, fm) != NULL)
        {
          fputs (buffer, fp);
          char *dash = strchr(buffer, '-');
          char *colon = strchr(buffer, ':');
          if (dash && colon && ((ptrdiff_t)strlen(buffer) > (colon - buffer) + 2))
            {
              char *endp;
              unsigned long start = strtoul(buffer, NULL, 16);
              unsigned long end   = strtoul(dash + 1, &endp, 16);
              unsigned long a     = strtoul(colon - 2, NULL, 16);
              unsigned long b     = strtoul(colon + 1, NULL, 16);
              // If it is an anonymous mapping 00:00 and both readable
              // and writeable then dump the contents of the mapping
              // to the bytes file.  Each block has a header of three
              // unsigned longs:
              // 0 - The number sizeof(unsigned long) to detect endianness and
              //     structure layout.
              // 1 - The offset in VM.
              // 2 - The Length in bytes.
              // Followed by the bytes.
              if (!a && !b && endp < colon && 'r' == endp[1] && 'w' == endp[2])
                {
                  unsigned long t = sizeof(unsigned long);
                  write(bytes_fd, (void*)&t, sizeof(t));
                  write(bytes_fd, (void*)&start, sizeof(start));
                  t = end - start;
                  write(bytes_fd, (void*)&t, sizeof(t));
                  write(bytes_fd, (void*)start, (end - start));
                }
            }
        } 
      fclose(fm);
    }
  fprintf(fp, "---------- End address map ----------\n");
  fflush(fp);
}

void
GC_enumerator::enumerate()
{
  print_address_map();
  fprintf(fp, "---------- Begin object map ----------\n");
  if (gc_ok)
    GC_gcollect();
  GC_apply_to_all_blocks(enumerate_callback_adaptor, 
			 (GC_word)(void*)(this));
  fprintf(fp, "---------- End object map ----------\n");
  fflush(fp); 

  GC_print_debug_info_file(fp);
  fflush(fp); 
  GC_print_hblkfreelist_file(fp);
  fflush(fp); 

  close(bytes_fd);
  fclose(fp);

  GC_clear_stack(0);
}

void
GC_enumerator::enumerate_callback_adaptor(GC_hblk_s *h,
					  GC_word dummy)
{
  GC_enumerator* pinfo = (GC_enumerator*)dummy;
  pinfo->enumerate_callback(h);
}

void
GC_enumerator::enumerate_callback(GC_hblk_s *h)
{
  size_t bytes;
  int i;

  for (i = 0; i == 0 || (i + bytes <= GC_get_hblk_size()); i += bytes)
    {
      int inUse = GC_is_marked((char*)h+i);                   // in use
      char *ptr = (char*)h+i;                                 // address
      int kind = GC_get_kind_and_size(h, &bytes);             // kind
      void *klass = 0;
      void *data = 0;
      if (kind == GC_gcj_kind
          || kind == GC_gcj_debug_kind
          || kind == GC_gcj_debug_kind+1)
        {
          void* v = *(void **)ptr;
          if (v)
            {
              klass = *(void **)v;
              data = *(void **)(ptr + sizeof(void*));
            }
        }
      if (inUse)
        fprintf (fp, "used = %d, ptr = %#lx, size = %zd, kind = %d, "
                 "klass = %#lx, data = %#lx\n", 
                 inUse, (unsigned long)ptr, bytes, kind,
                 (unsigned long)klass, (unsigned long)data);
    }
}

/*
 * Fill in a char[] with low bytes of the string characters.  These
 * methods may be called while an OutOfMemoryError is being thrown, so
 * we cannot call nice java methods to get the encoding of the string.
 */
static void
J2A(::java::lang::String* str, char *dst)
{
  jchar * pchars = JvGetStringChars(str);
  jint len = str->length();
  int i;
  for (i=0; i<len; i++)
    dst[i] = (char)pchars[i];
  dst[i] = 0;
}

void
::gnu::gcj::util::GCInfo::dump0 (::java::lang::String * name)
{
  char n[name->length() + 1];
  J2A(name, n);
  
  char temp[name->length() + 20];
  sprintf(temp, "%s%03d", n, GC_dump_count++);
  FILE* fp = fopen(temp, "w");

  GC_print_debug_info_file(fp);

  fclose(fp);
}

void
::gnu::gcj::util::GCInfo::enumerate0 (::java::lang::String * name)
{
  char n[name->length() + 1];
  J2A(name, n);
  char temp[name->length() + 20];
  sprintf(temp, "%s%03d", n, GC_dump_count++);

  GC_enumerator x(temp);
  x.enumerate();
}

static char *oomDumpName = NULL;

static void *
nomem_handler(size_t size)
{
  if (oomDumpName)
    {
      char temp[strlen(oomDumpName) + 20];
      sprintf(temp, "%s%03d", oomDumpName, GC_dump_count++);
      printf("nomem_handler(%zd) called\n", size);
      gc_ok--;
      GC_enumerator x(temp);
      x.enumerate();
      gc_ok++;
    }
  return (void*)0;
}

void
::gnu::gcj::util::GCInfo::setOOMDump0 (::java::lang::String * name)
{
  char *oldName = oomDumpName;
  oomDumpName = NULL;
  free (oldName);
  
  if (NULL == name)
    return;
  
  char *n = (char *)malloc(name->length() + 1);

  J2A(name, n);
  oomDumpName = n;
  GC_set_oom_fn(nomem_handler);
}

#else  // HAVE_PROC_SELF_MAPS

void
::gnu::gcj::util::GCInfo::dump0 (::java::lang::String * name)
{
  // Do nothing if dumping not supported.
}

void
::gnu::gcj::util::GCInfo::enumerate0 (::java::lang::String * name)
{
  // Do nothing if dumping not supported.
}

void
::gnu::gcj::util::GCInfo::setOOMDump0 (::java::lang::String * name)
{
  // Do nothing if dumping not supported.
}

#endif // HAVE_PROC_SELF_MAPS

