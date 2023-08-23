// link.cc - Code for linking and resolving classes and pool entries.

/* Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
   Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

/* Author: Kresten Krab Thorup <krab@gnu.org>  */

#include <config.h>
#include <platform.h>

#include <stdio.h>

#ifdef USE_LIBFFI
#include <ffi.h>
#endif

#include <java-interp.h>

// Set GC_DEBUG before including gc.h!
#ifdef LIBGCJ_GC_DEBUG
# define GC_DEBUG
#endif
#include <gc.h>

#include <jvm.h>
#include <gcj/cni.h>
#include <string.h>
#include <limits.h>
#include <java-cpool.h>
#include <execution.h>
#ifdef INTERPRETER
#include <jvmti.h>
#include "jvmti-int.h"
#endif
#include <java/lang/Class.h>
#include <java/lang/String.h>
#include <java/lang/StringBuffer.h>
#include <java/lang/Thread.h>
#include <java/lang/InternalError.h>
#include <java/lang/VirtualMachineError.h>
#include <java/lang/VerifyError.h>
#include <java/lang/NoSuchFieldError.h>
#include <java/lang/NoSuchMethodError.h>
#include <java/lang/ClassFormatError.h>
#include <java/lang/IllegalAccessError.h>
#include <java/lang/InternalError.h>
#include <java/lang/AbstractMethodError.h>
#include <java/lang/NoClassDefFoundError.h>
#include <java/lang/IncompatibleClassChangeError.h>
#include <java/lang/VerifyError.h>
#include <java/lang/VMClassLoader.h>
#include <java/lang/reflect/Modifier.h>
#include <java/security/CodeSource.h>

using namespace gcj;

template<typename T>
struct aligner
{
  char c;
  T field;
};

#define ALIGNOF(TYPE) (offsetof (aligner<TYPE>, field))

// This returns the alignment of a type as it would appear in a
// structure.  This can be different from the alignment of the type
// itself.  For instance on x86 double is 8-aligned but struct{double}
// is 4-aligned.
int
_Jv_Linker::get_alignment_from_class (jclass klass)
{
  if (klass == JvPrimClass (byte))
    return ALIGNOF (jbyte);
  else if (klass == JvPrimClass (short))
    return ALIGNOF (jshort);
  else if (klass == JvPrimClass (int)) 
    return ALIGNOF (jint);
  else if (klass == JvPrimClass (long))
    return ALIGNOF (jlong);
  else if (klass == JvPrimClass (boolean))
    return ALIGNOF (jboolean);
  else if (klass == JvPrimClass (char))
    return ALIGNOF (jchar);
  else if (klass == JvPrimClass (float))
    return ALIGNOF (jfloat);
  else if (klass == JvPrimClass (double))
    return ALIGNOF (jdouble);
  else
    return ALIGNOF (jobject);
}

void
_Jv_Linker::resolve_field (_Jv_Field *field, java::lang::ClassLoader *loader)
{
  if (! field->isResolved ())
    {
      _Jv_Utf8Const *sig = (_Jv_Utf8Const *) field->type;
      jclass type = _Jv_FindClassFromSignature (sig->chars(), loader);
      if (type == NULL)
	throw new java::lang::NoClassDefFoundError(field->name->toString());
      field->type = type;
      field->flags &= ~_Jv_FIELD_UNRESOLVED_FLAG;
    }
}

// A helper for find_field that knows how to recursively search
// superclasses and interfaces.
_Jv_Field *
_Jv_Linker::find_field_helper (jclass search, _Jv_Utf8Const *name,
			       _Jv_Utf8Const *type_name, jclass type,
			       jclass *declarer)
{
  while (search)
    {
      // From 5.4.3.2.  First search class itself.
      for (int i = 0; i < search->field_count; ++i)
	{
	  _Jv_Field *field = &search->fields[i];
	  if (! _Jv_equalUtf8Consts (field->name, name))
	    continue;

          // Checks for the odd situation where we were able to retrieve the
          // field's class from signature but the resolution of the field itself
          // failed which means a different class was resolved.
          if (type != NULL)
            {
              try
                {
                  resolve_field (field, search->loader);
                }
              catch (java::lang::Throwable *exc)
                {
                  java::lang::LinkageError *le = new java::lang::LinkageError
	            (JvNewStringLatin1 
                      ("field type mismatch with different loaders"));

                  le->initCause(exc);

                  throw le;
                }
            }

	  // Note that we compare type names and not types.  This is
	  // bizarre, but we do it because we want to find a field
	  // (and terminate the search) if it has the correct
	  // descriptor -- but then later reject it if the class
	  // loader check results in different classes.  We can't just
	  // pass in the descriptor and check that way, because when
	  // the field is already resolved there is no easy way to
	  // find its descriptor again.
	  if ((field->isResolved ()
               ? _Jv_equalUtf8Classnames (type_name, field->type->name)
               : _Jv_equalUtf8Classnames (type_name,
                                          (_Jv_Utf8Const *) field->type)))
	    {
	      *declarer = search;
	      return field;
	    }
	}

      // Next search direct interfaces.
      for (int i = 0; i < search->interface_count; ++i)
	{
	  _Jv_Field *result = find_field_helper (search->interfaces[i], name,
						 type_name, type, declarer);
	  if (result)
	    return result;
	}

      // Now search superclass.
      search = search->superclass;
    }

  return NULL;
}

bool
_Jv_Linker::has_field_p (jclass search, _Jv_Utf8Const *field_name)
{
  for (int i = 0; i < search->field_count; ++i)
    {
      _Jv_Field *field = &search->fields[i];
      if (_Jv_equalUtf8Consts (field->name, field_name))
	return true;
    }
  return false;
}

// Find a field.
// KLASS is the class that is requesting the field.
// OWNER is the class in which the field should be found.
// FIELD_TYPE_NAME is the type descriptor for the field.
// Fill FOUND_CLASS with the address of the class in which the field
// is actually declared.
// This function does the class loader type checks, and
// also access checks.  Returns the field, or throws an
// exception on error.
_Jv_Field *
_Jv_Linker::find_field (jclass klass, jclass owner,
			jclass *found_class,
			_Jv_Utf8Const *field_name,
			_Jv_Utf8Const *field_type_name)
{
  // FIXME: this allocates a _Jv_Utf8Const each time.  We should make
  // it cheaper.
  // Note: This call will resolve the primitive type names ("Z", "B", ...) to
  // their Java counterparts ("boolean", "byte", ...) if accessed via
  // field_type->name later.  Using these variants of the type name is in turn
  // important for the find_field_helper function.  However if the class
  // resolution failed then we can only use the already given type name.
  jclass field_type 
    = _Jv_FindClassFromSignatureNoException (field_type_name->chars(),
                                             klass->loader);

  _Jv_Field *the_field
    = find_field_helper (owner, field_name,
                         (field_type
                           ? field_type->name :
                             field_type_name ),
                           field_type, found_class);

  if (the_field == 0)
    {
      java::lang::StringBuffer *sb = new java::lang::StringBuffer();
      sb->append(JvNewStringLatin1("field "));
      sb->append(owner->getName());
      sb->append(JvNewStringLatin1("."));
      sb->append(_Jv_NewStringUTF(field_name->chars()));
      sb->append(JvNewStringLatin1(" was not found."));
      throw new java::lang::NoSuchFieldError (sb->toString());
    }

  // Accept it when the field's class could not be resolved.
  if (field_type == NULL)
    // Silently ignore that we were not able to retrieve the type to make it
    // possible to run code which does not access this field.
    return the_field;

  if (_Jv_CheckAccess (klass, *found_class, the_field->flags))
    {
      // Note that the field returned by find_field_helper is always
      // resolved.  However, we still use the constraint mechanism
      // because this may affect other lookups.
      _Jv_CheckOrCreateLoadingConstraint (field_type, (*found_class)->loader);
    }
  else
    {
      java::lang::StringBuffer *sb
	= new java::lang::StringBuffer ();
      sb->append(klass->getName());
      sb->append(JvNewStringLatin1(": "));
      sb->append((*found_class)->getName());
      sb->append(JvNewStringLatin1("."));
      sb->append(_Jv_NewStringUtf8Const (field_name));
      throw new java::lang::IllegalAccessError(sb->toString());
    }

  return the_field;
}

// Check loading constraints for method.
void
_Jv_Linker::check_loading_constraints (_Jv_Method *method, jclass self_class,
				       jclass other_class)
{
  JArray<jclass> *klass_args;
  jclass klass_return;

  _Jv_GetTypesFromSignature (method, self_class, &klass_args, &klass_return);
  jclass *klass_arg = elements (klass_args);
  java::lang::ClassLoader *found_loader = other_class->loader;

  _Jv_CheckOrCreateLoadingConstraint (klass_return, found_loader);
  for (int i = 0; i < klass_args->length; i++)
    _Jv_CheckOrCreateLoadingConstraint (*(klass_arg++), found_loader);
}

_Jv_Method *
_Jv_Linker::resolve_method_entry (jclass klass, jclass &found_class,
				  int class_index, int name_and_type_index,
				  bool init, bool is_iface)
{
  _Jv_Constants *pool = &klass->constants;
  jclass owner = resolve_pool_entry (klass, class_index).clazz;

  if (init && owner != klass)
    _Jv_InitClass (owner);

  _Jv_ushort name_index, type_index;
  _Jv_loadIndexes (&pool->data[name_and_type_index],
		   name_index,
		   type_index);

  _Jv_Utf8Const *method_name = pool->data[name_index].utf8;
  _Jv_Utf8Const *method_signature = pool->data[type_index].utf8;

  _Jv_Method *the_method = 0;
  found_class = 0;

  // We're going to cache a pointer to the _Jv_Method object
  // when we find it.  So, to ensure this doesn't get moved from
  // beneath us, we first put all the needed Miranda methods
  // into the target class.
  wait_for_state (klass, JV_STATE_LOADED);

  // First search the class itself.
  the_method = search_method_in_class (owner, klass,
				       method_name, method_signature);

  if (the_method != 0)
    {
      found_class = owner;
      goto end_of_method_search;
    }

  // If we are resolving an interface method, search the
  // interface's superinterfaces (A superinterface is not an
  // interface's superclass - a superinterface is implemented by
  // the interface).
  if (is_iface)
    {
      _Jv_ifaces ifaces;
      ifaces.count = 0;
      ifaces.len = 4;
      ifaces.list = (jclass *) _Jv_Malloc (ifaces.len
					   * sizeof (jclass *));

      get_interfaces (owner, &ifaces);

      for (int i = 0; i < ifaces.count; i++)
	{
	  jclass cls = ifaces.list[i];
	  the_method = search_method_in_class (cls, klass, method_name, 
					       method_signature);
	  if (the_method != 0)
	    {
	      found_class = cls;
	      break;
	    }
	}

      _Jv_Free (ifaces.list);

      if (the_method != 0)
	goto end_of_method_search;
    }

  // Finally, search superclasses. 
  the_method = (search_method_in_superclasses 
		(owner->getSuperclass (), klass, method_name, 
		 method_signature, &found_class));
  

 end_of_method_search:
  if (the_method == 0)
    {
      java::lang::StringBuffer *sb = new java::lang::StringBuffer();
      sb->append(JvNewStringLatin1("method "));
      sb->append(owner->getName());
      sb->append(JvNewStringLatin1("."));
      sb->append(_Jv_NewStringUTF(method_name->chars()));
      sb->append(JvNewStringLatin1(" with signature "));
      sb->append(_Jv_NewStringUTF(method_signature->chars()));
      sb->append(JvNewStringLatin1(" was not found."));
      throw new java::lang::NoSuchMethodError (sb->toString());
    }

  // if (found_class->loader != klass->loader), then we must actually
  // check that the types of arguments correspond.  JVMS 5.4.3.3.
  if (found_class->loader != klass->loader)
    check_loading_constraints (the_method, klass, found_class);
  
  return the_method;
}

_Jv_Mutex_t _Jv_Linker::resolve_mutex;

void
_Jv_Linker::init (void)
{
  _Jv_MutexInit (&_Jv_Linker::resolve_mutex);
}

// Locking in resolve_pool_entry is somewhat subtle.  Constant
// resolution is idempotent, so it doesn't matter if two threads
// resolve the same entry.  However, it is important that we always
// write the resolved flag and the data together, atomically.  It is
// also important that we read them atomically.
_Jv_word
_Jv_Linker::resolve_pool_entry (jclass klass, int index, bool lazy)
{
  using namespace java::lang::reflect;

  if (GC_base (klass) && klass->constants.data
      && ! GC_base (klass->constants.data))
    // If a class is heap-allocated but the constant pool is not this
    // is a "new ABI" class, i.e. one where the initial constant pool
    // is in the read-only data section of an object file.  Copy the
    // initial constant pool from there to a new heap-allocated pool.
    {
      jsize count = klass->constants.size;
      if (count)
	{
	  _Jv_word* constants
	    = (_Jv_word*) _Jv_AllocRawObj (count * sizeof (_Jv_word));
	  memcpy ((void*)constants,
		  (void*)klass->constants.data,
		  count * sizeof (_Jv_word));
	  klass->constants.data = constants;
	}
    }

  _Jv_Constants *pool = &klass->constants;

  jbyte tags;
  _Jv_word data;
  tags = read_cpool_entry (&data, pool, index);

  if ((tags & JV_CONSTANT_ResolvedFlag) != 0)
    return data;

  switch (tags & ~JV_CONSTANT_LazyFlag)
    {
    case JV_CONSTANT_Class:
      {
	_Jv_Utf8Const *name = data.utf8;

	jclass found;
	if (name->first() == '[')
	  found = _Jv_FindClassFromSignatureNoException (name->chars(),
		                                         klass->loader);
        else
	  found = _Jv_FindClassNoException (name, klass->loader);

        // If the class could not be loaded a phantom class is created. Any
        // function that deals with such a class but cannot do something useful
        // with it should just throw a NoClassDefFoundError with the class'
        // name.
	if (! found)
	  {
	    if (lazy)
	      {
		found = _Jv_NewClass(name, NULL, NULL);
		found->state = JV_STATE_PHANTOM;
		tags |= JV_CONSTANT_ResolvedFlag;
		data.clazz = found;
		break;
	      }
	    else
	      throw new java::lang::NoClassDefFoundError (name->toString());
	  }

	// Check accessibility, but first strip array types as
	// _Jv_ClassNameSamePackage can't handle arrays.
	jclass check;
	for (check = found;
	     check && check->isArray();
	     check = check->getComponentType())
	  ;
	if ((found->accflags & Modifier::PUBLIC) == Modifier::PUBLIC
	    || (_Jv_ClassNameSamePackage (check->name,
					  klass->name)))
	  {
	    data.clazz = found;
	    tags |= JV_CONSTANT_ResolvedFlag;
	  }
	else
	  {
	    java::lang::StringBuffer *sb = new java::lang::StringBuffer ();
	    sb->append(klass->getName());
	    sb->append(JvNewStringLatin1(" can't access class "));
	    sb->append(found->getName());
	    throw new java::lang::IllegalAccessError(sb->toString());
	  }
      }
      break;

    case JV_CONSTANT_String:
      {
	jstring str;
	str = _Jv_NewStringUtf8Const (data.utf8);
	data.o = str;
	tags |= JV_CONSTANT_ResolvedFlag;
      }
      break;

    case JV_CONSTANT_Fieldref:
      {
	_Jv_ushort class_index, name_and_type_index;
	_Jv_loadIndexes (&data,
			 class_index,
			 name_and_type_index);
	jclass owner = (resolve_pool_entry (klass, class_index, true)).clazz;

        // If a phantom class was resolved our field reference is
        // unusable because of the missing class.
        if (owner->state == JV_STATE_PHANTOM)
          throw new java::lang::NoClassDefFoundError(owner->getName());

	// We don't initialize 'owner', but we do make sure that its
	// fields exist.
	wait_for_state (owner, JV_STATE_PREPARED);

	_Jv_ushort name_index, type_index;
	_Jv_loadIndexes (&pool->data[name_and_type_index],
			 name_index,
			 type_index);

	_Jv_Utf8Const *field_name = pool->data[name_index].utf8;
	_Jv_Utf8Const *field_type_name = pool->data[type_index].utf8;

	jclass found_class = 0;
	_Jv_Field *the_field = find_field (klass, owner, 
					   &found_class,
					   field_name,
					   field_type_name);
	// Initialize the field's declaring class, not its qualifying
	// class.
	_Jv_InitClass (found_class);
	data.field = the_field;
	tags |= JV_CONSTANT_ResolvedFlag;
      }
      break;

    case JV_CONSTANT_Methodref:
    case JV_CONSTANT_InterfaceMethodref:
      {
	_Jv_ushort class_index, name_and_type_index;
	_Jv_loadIndexes (&data,
			 class_index,
			 name_and_type_index);

	_Jv_Method *the_method;
	jclass found_class;
	the_method = resolve_method_entry (klass, found_class,
					   class_index, name_and_type_index,
					   true,
					   tags == JV_CONSTANT_InterfaceMethodref);
      
	data.rmethod
	  = klass->engine->resolve_method(the_method,
					  found_class,
					  ((the_method->accflags
					    & Modifier::STATIC) != 0));
	tags |= JV_CONSTANT_ResolvedFlag;
      }
      break;
    }

  write_cpool_entry (data, tags, pool, index);

  return data;
}

// This function is used to lazily locate superclasses and
// superinterfaces.  This must be called with the class lock held.
void
_Jv_Linker::resolve_class_ref (jclass klass, jclass *classref)
{
  jclass ret = *classref;

  // If superclass looks like a constant pool entry, resolve it now.
  if (ret && (uaddr) ret < (uaddr) klass->constants.size)
    {
      if (klass->state < JV_STATE_LINKED)
	{
	  _Jv_Utf8Const *name = klass->constants.data[(uaddr) *classref].utf8;
	  ret = _Jv_FindClass (name, klass->loader);
	  if (! ret)
	    {
	      throw new java::lang::NoClassDefFoundError (name->toString());
	    }
	}
      else
	ret = klass->constants.data[(uaddr) classref].clazz;
      *classref = ret;
    }
}

// Find a method declared in the cls that is referenced from klass and
// perform access checks if CHECK_PERMS is true.
_Jv_Method *
_Jv_Linker::search_method_in_class (jclass cls, jclass klass, 
				    _Jv_Utf8Const *method_name, 
				    _Jv_Utf8Const *method_signature,
				    bool check_perms)
{
  using namespace java::lang::reflect;

  for (int i = 0;  i < cls->method_count;  i++)
    {
      _Jv_Method *method = &cls->methods[i];
      if (   (!_Jv_equalUtf8Consts (method->name,
				    method_name))
	  || (!_Jv_equalUtf8Consts (method->signature,
				    method_signature)))
	continue;

      if (!check_perms || _Jv_CheckAccess (klass, cls, method->accflags))
	return method;
      else
	{
	  java::lang::StringBuffer *sb = new java::lang::StringBuffer();
	  sb->append(klass->getName());
	  sb->append(JvNewStringLatin1(": "));
	  sb->append(cls->getName());
	  sb->append(JvNewStringLatin1("."));
	  sb->append(_Jv_NewStringUTF(method_name->chars()));
	  sb->append(_Jv_NewStringUTF(method_signature->chars()));
	  throw new java::lang::IllegalAccessError (sb->toString());
	}
    }
  return 0;
}

// Like search_method_in_class, but work our way up the superclass
// chain.
_Jv_Method *
_Jv_Linker::search_method_in_superclasses (jclass cls, jclass klass, 
					   _Jv_Utf8Const *method_name, 
					   _Jv_Utf8Const *method_signature,
					   jclass *found_class, bool check_perms)
{
  _Jv_Method *the_method = NULL;

  for ( ; cls != 0; cls = cls->getSuperclass ())
    {
      the_method = search_method_in_class (cls, klass, method_name,
					   method_signature, check_perms);
      if (the_method != 0)
	{
	  if (found_class)
	    *found_class = cls;
	  break;
	}
    }
  
  return the_method;
}

#define INITIAL_IOFFSETS_LEN 4
#define INITIAL_IFACES_LEN 4

static _Jv_IDispatchTable null_idt = {SHRT_MAX, 0, {}};

// Generate tables for constant-time assignment testing and interface
// method lookup. This implements the technique described by Per Bothner
// <per@bothner.com> on the java-discuss mailing list on 1999-09-02:
// http://gcc.gnu.org/ml/java/1999-q3/msg00377.html
void
_Jv_Linker::prepare_constant_time_tables (jclass klass)
{  
  if (klass->isPrimitive () || klass->isInterface ())
    return;

  // Short-circuit in case we've been called already.
  if ((klass->idt != NULL) || klass->depth != 0)
    return;

  // Calculate the class depth and ancestor table. The depth of a class 
  // is how many "extends" it is removed from Object. Thus the depth of 
  // java.lang.Object is 0, but the depth of java.io.FilterOutputStream 
  // is 2. Depth is defined for all regular and array classes, but not 
  // interfaces or primitive types.
   
  jclass klass0 = klass;
  jboolean has_interfaces = false;
  while (klass0 != &java::lang::Object::class$)
    {
      if (klass0->interface_count)
	has_interfaces = true;
      klass0 = klass0->superclass;
      klass->depth++;
    }

  // We do class member testing in constant time by using a small table 
  // of all the ancestor classes within each class. The first element is 
  // a pointer to the current class, and the rest are pointers to the 
  // classes ancestors, ordered from the current class down by decreasing 
  // depth. We do not include java.lang.Object in the table of ancestors, 
  // since it is redundant.  Note that the classes pointed to by
  // 'ancestors' will always be reachable by other paths.

  klass->ancestors = (jclass *) _Jv_AllocBytes (klass->depth
						* sizeof (jclass));
  klass0 = klass;
  for (int index = 0; index < klass->depth; index++)
    {
      klass->ancestors[index] = klass0;
      klass0 = klass0->superclass;
    }

  if ((klass->accflags & java::lang::reflect::Modifier::ABSTRACT) != 0)
    return;

  // Optimization: If class implements no interfaces, use a common
  // predefined interface table.
  if (!has_interfaces)
    {
      klass->idt = &null_idt;
      return;
    }

  _Jv_ifaces ifaces;
  ifaces.count = 0;
  ifaces.len = INITIAL_IFACES_LEN;
  ifaces.list = (jclass *) _Jv_Malloc (ifaces.len * sizeof (jclass *));

  int itable_size = get_interfaces (klass, &ifaces);

  if (ifaces.count > 0)
    {
      // The classes pointed to by the itable will always be reachable
      // via other paths.
      int idt_bytes = sizeof (_Jv_IDispatchTable) + (itable_size 
						     * sizeof (void *));
      klass->idt = (_Jv_IDispatchTable *) _Jv_AllocBytes (idt_bytes);
      klass->idt->itable_length = itable_size;

      jshort *itable_offsets = 
	(jshort *) _Jv_Malloc (ifaces.count * sizeof (jshort));

      generate_itable (klass, &ifaces, itable_offsets);

      jshort cls_iindex = find_iindex (ifaces.list, itable_offsets,
				       ifaces.count);

      for (int i = 0; i < ifaces.count; i++)
	{
	  ifaces.list[i]->ioffsets[cls_iindex] = itable_offsets[i];
	}

      klass->idt->iindex = cls_iindex;	    

      _Jv_Free (ifaces.list);
      _Jv_Free (itable_offsets);
    }
  else 
    {
      klass->idt->iindex = SHRT_MAX;
    }
}

// Return index of item in list, or -1 if item is not present.
inline jshort
_Jv_Linker::indexof (void *item, void **list, jshort list_len)
{
  for (int i=0; i < list_len; i++)
    {
      if (list[i] == item)
        return i;
    }
  return -1;
}

// Find all unique interfaces directly or indirectly implemented by klass.
// Returns the size of the interface dispatch table (itable) for klass, which 
// is the number of unique interfaces plus the total number of methods that 
// those interfaces declare. May extend ifaces if required.
jshort
_Jv_Linker::get_interfaces (jclass klass, _Jv_ifaces *ifaces)
{
  jshort result = 0;
  
  for (int i = 0; i < klass->interface_count; i++)
    {
      jclass iface = klass->interfaces[i];

      /* Make sure interface is linked.  */
      wait_for_state(iface, JV_STATE_LINKED);

      if (indexof (iface, (void **) ifaces->list, ifaces->count) == -1)
        {
	  if (ifaces->count + 1 >= ifaces->len)
	    {
	      /* Resize ifaces list */
	      ifaces->len = ifaces->len * 2;
	      ifaces->list
		= (jclass *) _Jv_Realloc (ifaces->list,
					  ifaces->len * sizeof(jclass));
	    }
	  ifaces->list[ifaces->count] = iface;
	  ifaces->count++;

	  result += get_interfaces (klass->interfaces[i], ifaces);
	}
    }

  if (klass->isInterface())
    {
      // We want to add 1 plus the number of interface methods here.
      // But, we take special care to skip <clinit>.
      ++result;
      for (int i = 0; i < klass->method_count; ++i)
	{
	  if (klass->methods[i].name->first() != '<')
	    ++result;
	}
    }
  else if (klass->superclass)
    result += get_interfaces (klass->superclass, ifaces);
  return result;
}

// Fill out itable in klass, resolving method declarations in each ifaces.
// itable_offsets is filled out with the position of each iface in itable,
// such that itable[itable_offsets[n]] == ifaces.list[n].
void
_Jv_Linker::generate_itable (jclass klass, _Jv_ifaces *ifaces,
			       jshort *itable_offsets)
{
  void **itable = klass->idt->itable;
  jshort itable_pos = 0;

  for (int i = 0; i < ifaces->count; i++)
    { 
      jclass iface = ifaces->list[i];
      itable_offsets[i] = itable_pos;
      itable_pos = append_partial_itable (klass, iface, itable, itable_pos);

      /* Create ioffsets table for iface */
      if (iface->ioffsets == NULL)
	{
	  // The first element of ioffsets is its length (itself included).
	  jshort *ioffsets = (jshort *) _Jv_AllocBytes (INITIAL_IOFFSETS_LEN
							* sizeof (jshort));
	  ioffsets[0] = INITIAL_IOFFSETS_LEN;
	  for (int i = 1; i < INITIAL_IOFFSETS_LEN; i++)
	    ioffsets[i] = -1;

	  iface->ioffsets = ioffsets;
	}
    }
}

// Format method name for use in error messages.
jstring
_Jv_GetMethodString (jclass klass, _Jv_Method *meth,
		     jclass derived)
{
  using namespace java::lang;
  StringBuffer *buf = new StringBuffer (klass->name->toString());
  buf->append (jchar ('.'));
  buf->append (meth->name->toString());
  buf->append ((jchar) ' ');
  buf->append (meth->signature->toString());
  if (derived)
    {
      buf->append(JvNewStringLatin1(" in "));
      buf->append(derived->name->toString());
    }
  return buf->toString();
}

void
_Jv_ThrowNoSuchMethodError ()
{
  throw new java::lang::NoSuchMethodError;
}

#if defined USE_LIBFFI && FFI_CLOSURES && defined(INTERPRETER)
// A function whose invocation is prepared using libffi. It gets called
// whenever a static method of a missing class is invoked. The data argument
// holds a reference to a String denoting the missing class.
// The prepared function call is stored in a class' atable.
void
_Jv_ThrowNoClassDefFoundErrorTrampoline(ffi_cif *,
                                        void *,
                                        void **,
                                        void *data)
{
  throw new java::lang::NoClassDefFoundError(
    _Jv_NewStringUtf8Const((_Jv_Utf8Const *) data));
}
#else
// A variant of the NoClassDefFoundError throwing method that can
// be used without libffi.
void
_Jv_ThrowNoClassDefFoundError()
{
  throw new java::lang::NoClassDefFoundError();
}
#endif

// Throw a NoSuchFieldError.  Called by compiler-generated code when
// an otable entry is zero.  OTABLE_INDEX is the index in the caller's
// otable that refers to the missing field.  This index may be used to
// print diagnostic information about the field.
void
_Jv_ThrowNoSuchFieldError (int /* otable_index */)
{
  throw new java::lang::NoSuchFieldError;
}

// This is put in empty vtable slots.
void
_Jv_ThrowAbstractMethodError ()
{
  throw new java::lang::AbstractMethodError();
}

// Each superinterface of a class (i.e. each interface that the class
// directly or indirectly implements) has a corresponding "Partial
// Interface Dispatch Table" whose size is (number of methods + 1) words.
// The first word is a pointer to the interface (i.e. the java.lang.Class
// instance for that interface).  The remaining words are pointers to the
// actual methods that implement the methods declared in the interface,
// in order of declaration.
//
// Append partial interface dispatch table for "iface" to "itable", at
// position itable_pos.
// Returns the offset at which the next partial ITable should be appended.
jshort
_Jv_Linker::append_partial_itable (jclass klass, jclass iface,
				   void **itable, jshort pos)
{
  using namespace java::lang::reflect;

  itable[pos++] = (void *) iface;
  _Jv_Method *meth;
  
  for (int j=0; j < iface->method_count; j++)
    {
      // Skip '<clinit>' here.
      if (iface->methods[j].name->first() == '<')
	continue;

      meth = NULL;
      jclass cl;
      for (cl = klass; cl; cl = cl->getSuperclass())
        {
	  meth = _Jv_GetMethodLocal (cl, iface->methods[j].name,
				     iface->methods[j].signature);
		 
	  if (meth)
	    break;
	}

      if (meth)
        {
	  if ((meth->accflags & Modifier::STATIC) != 0)
	    throw new java::lang::IncompatibleClassChangeError
	      (_Jv_GetMethodString (klass, meth));
	  if ((meth->accflags & Modifier::PUBLIC) == 0)
	    throw new java::lang::IllegalAccessError
	      (_Jv_GetMethodString (klass, meth));

 	  if ((meth->accflags & Modifier::ABSTRACT) != 0)
	    itable[pos] = (void *) &_Jv_ThrowAbstractMethodError;
	  else
	    itable[pos] = meth->ncode;

	  if (cl->loader != iface->loader)
	    check_loading_constraints (meth, cl, iface);
	}
      else
        {
	  // The method doesn't exist in klass. Binary compatibility rules
	  // permit this, so we delay the error until runtime using a pointer
	  // to a method which throws an exception.
	  itable[pos] = (void *) _Jv_ThrowNoSuchMethodError;
	}
      pos++;
    }
    
  return pos;
}

static _Jv_Mutex_t iindex_mutex;
static bool iindex_mutex_initialized = false;

// We need to find the correct offset in the Class Interface Dispatch 
// Table for a given interface. Once we have that, invoking an interface 
// method just requires combining the Method's index in the interface 
// (known at compile time) to get the correct method.  Doing a type test 
// (cast or instanceof) is the same problem: Once we have a possible Partial 
// Interface Dispatch Table, we just compare the first element to see if it 
// matches the desired interface. So how can we find the correct offset?  
// Our solution is to keep a vector of candiate offsets in each interface 
// (ioffsets), and in each class we have an index (idt->iindex) used to
// select the correct offset from ioffsets.
//
// Calculate and return iindex for a new class. 
// ifaces is a vector of num interfaces that the class implements.
// offsets[j] is the offset in the interface dispatch table for the
// interface corresponding to ifaces[j].
// May extend the interface ioffsets if required.
jshort
_Jv_Linker::find_iindex (jclass *ifaces, jshort *offsets, jshort num)
{
  int i;
  int j;
  
  // Acquire a global lock to prevent itable corruption in case of multiple 
  // classes that implement an intersecting set of interfaces being linked
  // simultaneously. We can assume that the mutex will be initialized
  // single-threaded.
  if (! iindex_mutex_initialized)
    {
      _Jv_MutexInit (&iindex_mutex);
      iindex_mutex_initialized = true;
    }
  
  _Jv_MutexLock (&iindex_mutex);
  
  for (i=1;; i++)  /* each potential position in ioffsets */
    {
      for (j=0;; j++)  /* each iface */
        {
	  if (j >= num)
	    goto found;
	  if (i >= ifaces[j]->ioffsets[0])
	    continue;
	  int ioffset = ifaces[j]->ioffsets[i];
	  /* We can potentially share this position with another class. */
	  if (ioffset >= 0 && ioffset != offsets[j])
	    break; /* Nope. Try next i. */	  
	}
    }
  found:
  for (j = 0; j < num; j++)
    {
      int len = ifaces[j]->ioffsets[0];
      if (i >= len) 
	{
	  // Resize ioffsets.
	  int newlen = 2 * len;
	  if (i >= newlen)
	    newlen = i + 3;

	  jshort *old_ioffsets = ifaces[j]->ioffsets;
	  jshort *new_ioffsets = (jshort *) _Jv_AllocBytes (newlen
							    * sizeof(jshort));
	  memcpy (&new_ioffsets[1], &old_ioffsets[1],
		  (len - 1) * sizeof (jshort));
	  new_ioffsets[0] = newlen;

	  while (len < newlen)
	    new_ioffsets[len++] = -1;
	  
	  ifaces[j]->ioffsets = new_ioffsets;
	}
      ifaces[j]->ioffsets[i] = offsets[j];
    }

  _Jv_MutexUnlock (&iindex_mutex);

  return i;
}

#if defined USE_LIBFFI && FFI_CLOSURES && defined(INTERPRETER)
// We use a structure of this type to store the closure that
// represents a missing method.
struct method_closure
{
  // This field must come first, since the address of this field will
  // be the same as the address of the overall structure.  This is due
  // to disabling interior pointers in the GC.
  ffi_closure closure;
  _Jv_ClosureList list;
  ffi_cif cif;
  ffi_type *arg_types[1];
};

void *
_Jv_Linker::create_error_method (_Jv_Utf8Const *class_name, jclass klass)
{
  void *code;
  method_closure *closure
    = (method_closure *)ffi_closure_alloc (sizeof (method_closure), &code);

  closure->arg_types[0] = &ffi_type_void;

  // Initializes the cif and the closure.  If that worked the closure
  // is returned and can be used as a function pointer in a class'
  // atable.
  if (   ffi_prep_cif (&closure->cif,
                       FFI_DEFAULT_ABI,
                       1,
                       &ffi_type_void,
		       closure->arg_types) == FFI_OK
      && ffi_prep_closure_loc (&closure->closure,
			       &closure->cif,
			       _Jv_ThrowNoClassDefFoundErrorTrampoline,
			       class_name,
			       code) == FFI_OK)
    {
      closure->list.registerClosure (klass, closure);
      return code;
    }
  else
    {
      ffi_closure_free (closure);
      java::lang::StringBuffer *buffer = new java::lang::StringBuffer();
      buffer->append(JvNewStringLatin1("Error setting up FFI closure"
				       " for static method of"
				       " missing class: "));
      buffer->append (_Jv_NewStringUtf8Const(class_name));
      throw new java::lang::InternalError(buffer->toString());
    }
}
#else
void *
_Jv_Linker::create_error_method (_Jv_Utf8Const *, jclass)
{
  // Codepath for platforms which do not support (or want) libffi.
  // You have to accept that it is impossible to provide the name
  // of the missing class then.
  return (void *) _Jv_ThrowNoClassDefFoundError;
}
#endif // USE_LIBFFI && FFI_CLOSURES

// Functions for indirect dispatch (symbolic virtual binding) support.

// There are three tables, atable otable and itable.  atable is an
// array of addresses, and otable is an array of offsets, and these
// are used for static and virtual members respectively.  itable is an
// array of pairs {address, index} where each address is a pointer to
// an interface.

// {a,o,i}table_syms is an array of _Jv_MethodSymbols.  Each such
// symbol is a tuple of {classname, member name, signature}.

// Set this to true to enable debugging of indirect dispatch tables/linking.
static bool debug_link = false;

// link_symbol_table() scans these two arrays and fills in the
// corresponding atable and otable with the addresses of static
// members and the offsets of virtual members.

// The offset (in bytes) for each resolved method or field is placed
// at the corresponding position in the virtual method offset table
// (klass->otable). 

// This must be called while holding the class lock.

void
_Jv_Linker::link_symbol_table (jclass klass)
{
  int index = 0;
  _Jv_MethodSymbol sym;
  if (klass->otable == NULL
      || klass->otable->state != 0)
    goto atable;
   
  klass->otable->state = 1;

  if (debug_link)
    fprintf (stderr, "Fixing up otable in %s:\n", klass->name->chars());
  for (index = 0;
       (sym = klass->otable_syms[index]).class_name != NULL;
       ++index)
    {
      jclass target_class = _Jv_FindClass (sym.class_name, klass->loader);
      _Jv_Method *meth = NULL;            

      _Jv_Utf8Const *signature = sym.signature;
      uaddr special;
      maybe_adjust_signature (signature, special);

      if (target_class == NULL)
	throw new java::lang::NoClassDefFoundError 
	  (_Jv_NewStringUTF (sym.class_name->chars()));

      // We're looking for a field or a method, and we can tell
      // which is needed by looking at the signature.
      if (signature->first() == '(' && signature->len() >= 2)
	{
	  // Looks like someone is trying to invoke an interface method
	  if (target_class->isInterface())
	    {
	      using namespace java::lang;
	      StringBuffer *sb = new StringBuffer();
	      sb->append(JvNewStringLatin1("found interface "));
	      sb->append(target_class->getName());
	      sb->append(JvNewStringLatin1(" when searching for a class"));
	      throw new VerifyError(sb->toString());
	    }

 	  // If the target class does not have a vtable_method_count yet, 
	  // then we can't tell the offsets for its methods, so we must lay 
	  // it out now.
	  wait_for_state(target_class, JV_STATE_PREPARED);

	  try
	    {
	      meth = (search_method_in_superclasses 
		      (target_class, klass, sym.name, signature, 
		       NULL, special == 0));
	    }
	  catch (::java::lang::IllegalAccessError *e)
	    {
	    }

	  // Every class has a throwNoSuchMethodErrorIndex method that
	  // it inherits from java.lang.Object.  Find its vtable
	  // offset.
	  static int throwNoSuchMethodErrorIndex;
	  if (throwNoSuchMethodErrorIndex == 0)
	    {
	      Utf8Const* name 
		= _Jv_makeUtf8Const ("throwNoSuchMethodError", 
				     strlen ("throwNoSuchMethodError"));
	      _Jv_Method* meth
		= _Jv_LookupDeclaredMethod (&java::lang::Object::class$, 
					    name, gcj::void_signature);
	      throwNoSuchMethodErrorIndex 
		= _Jv_VTable::idx_to_offset (meth->index);
	    }
	  
	  // If we don't find a nonstatic method, insert the
	  // vtable index of Object.throwNoSuchMethodError().
	  // This defers the missing method error until an attempt
	  // is made to execute it.	  
	  {
	    int offset;
	    
	    if (meth != NULL)
	      offset = _Jv_VTable::idx_to_offset (meth->index);
	    else
	      offset = throwNoSuchMethodErrorIndex;		    
	    
	    if (offset == -1)
	      JvFail ("Bad method index");
	    JvAssert (meth->index < target_class->vtable_method_count);
	    
	    klass->otable->offsets[index] = offset;
	  }

	  if (debug_link)
	    fprintf (stderr, "  offsets[%d] = %d (class %s@%p : %s(%s))\n",
		     (int)index,
		     (int)klass->otable->offsets[index],
		     (const char*)target_class->name->chars(),
		     target_class,
		     (const char*)sym.name->chars(),
		     (const char*)signature->chars());
	  continue;
	}

      // Try fields.
      {
	wait_for_state(target_class, JV_STATE_PREPARED);
	jclass found_class;
	_Jv_Field *the_field = NULL;
	try
	  {
	    the_field = find_field (klass, target_class, &found_class,
				    sym.name, signature);
	    if ((the_field->flags & java::lang::reflect::Modifier::STATIC))
	      throw new java::lang::IncompatibleClassChangeError;
	    else
	      klass->otable->offsets[index] = the_field->u.boffset;
	  }
	catch (java::lang::NoSuchFieldError *err)
	  {
	    klass->otable->offsets[index] = 0;
	  }
      }
    }

 atable:
  if (klass->atable == NULL || klass->atable->state != 0)
    goto itable;

  klass->atable->state = 1;

  for (index = 0;
       (sym = klass->atable_syms[index]).class_name != NULL;
       ++index)
    {
      jclass target_class =
        _Jv_FindClassNoException (sym.class_name, klass->loader);

      _Jv_Method *meth = NULL;            

      _Jv_Utf8Const *signature = sym.signature;
      uaddr special;
      maybe_adjust_signature (signature, special);

      // ??? Setting this pointer to null will at least get us a
      // NullPointerException
      klass->atable->addresses[index] = NULL;

      bool use_error_method = false;

      // If the target class is missing we prepare a function call
      // that throws a NoClassDefFoundError and store the address of
      // that newly prepared method in the atable. The user can run
      // code in classes where the missing class is part of the
      // execution environment as long as it is never referenced.
      if (target_class == NULL)
	use_error_method = true;
      // We're looking for a static field or a static method, and we
      // can tell which is needed by looking at the signature.
      else if (signature->first() == '(' && signature->len() >= 2)
	{
 	  // If the target class does not have a vtable_method_count yet, 
	  // then we can't tell the offsets for its methods, so we must lay 
	  // it out now.
	  wait_for_state (target_class, JV_STATE_PREPARED);

	  // Interface methods cannot have bodies.
	  if (target_class->isInterface())
	    {
	      using namespace java::lang;
	      StringBuffer *sb = new StringBuffer();
	      sb->append(JvNewStringLatin1("class "));
	      sb->append(target_class->getName());
	      sb->append(JvNewStringLatin1(" is an interface: "
					   "class expected"));
	      throw new VerifyError(sb->toString());
	    }

	  try
	    {
	      meth = (search_method_in_superclasses 
		      (target_class, klass, sym.name, signature, 
		       NULL, special == 0));
	    }
	  catch (::java::lang::IllegalAccessError *e)
	    {
	    }

	  if (meth != NULL)
	    {
	      if (meth->ncode) // Maybe abstract?
		{
		  klass->atable->addresses[index] = meth->ncode;
		  if (debug_link)
		    fprintf (stderr, "  addresses[%d] = %p (class %s@%p : %s(%s))\n",
			     index,
			     &klass->atable->addresses[index],
			     (const char*)target_class->name->chars(),
			     klass,
			     (const char*)sym.name->chars(),
			     (const char*)signature->chars());
		}
	    }
	  else
	    use_error_method = true;

	  if (use_error_method)
	    klass->atable->addresses[index]
	      = create_error_method(sym.class_name, klass);

	  continue;
	}


      // Try fields only if the target class exists.
      if (target_class != NULL)
      {
	wait_for_state(target_class, JV_STATE_PREPARED);
	jclass found_class;
	_Jv_Field *the_field = find_field (klass, target_class, &found_class,
					   sym.name, signature);
	if ((the_field->flags & java::lang::reflect::Modifier::STATIC))
	  klass->atable->addresses[index] = the_field->u.addr;
	else
	  throw new java::lang::IncompatibleClassChangeError;
      }
    }

 itable:
  if (klass->itable == NULL
      || klass->itable->state != 0)
    return;

  klass->itable->state = 1;

  for (index = 0;
       (sym = klass->itable_syms[index]).class_name != NULL; 
       ++index)
    {
      jclass target_class = _Jv_FindClass (sym.class_name, klass->loader);

      _Jv_Utf8Const *signature = sym.signature;
      uaddr special;
      maybe_adjust_signature (signature, special);

      jclass cls;
      int i;

      wait_for_state(target_class, JV_STATE_LOADED);
      bool found = _Jv_getInterfaceMethod (target_class, cls, i,
					   sym.name, signature);

      if (found)
	{
	  klass->itable->addresses[index * 2] = cls;
	  klass->itable->addresses[index * 2 + 1] = (void *)(unsigned long) i;
	  if (debug_link)
	    {
	      fprintf (stderr, "  interfaces[%d] = %p (interface %s@%p : %s(%s))\n",
		       index,
		       klass->itable->addresses[index * 2],
		       (const char*)cls->name->chars(),
		       cls,
		       (const char*)sym.name->chars(),
		       (const char*)signature->chars());
	      fprintf (stderr, "            [%d] = offset %d\n",
		       index + 1,
		       (int)(unsigned long)klass->itable->addresses[index * 2 + 1]);
	    }

	}
      else
	throw new java::lang::IncompatibleClassChangeError;
    }

}

// For each catch_record in the list of caught classes, fill in the
// address field.
void 
_Jv_Linker::link_exception_table (jclass self)
{
  struct _Jv_CatchClass *catch_record = self->catch_classes;
  if (!catch_record || catch_record->classname)
    return;  
  catch_record++;
  while (catch_record->classname)
    {
      try
	{
	  jclass target_class
	    = _Jv_FindClass (catch_record->classname,  
			     self->getClassLoaderInternal ());
	  *catch_record->address = target_class;
	}
      catch (::java::lang::Throwable *t)
	{
	  // FIXME: We need to do something better here.
	  *catch_record->address = 0;
	}
      catch_record++;
    }
  self->catch_classes->classname = (_Jv_Utf8Const *)-1;
}
  
// Set itable method indexes for members of interface IFACE.
void
_Jv_Linker::layout_interface_methods (jclass iface)
{
  if (! iface->isInterface())
    return;

  // itable indexes start at 1. 
  // FIXME: Static initalizers currently get a NULL placeholder entry in the
  // itable so they are also assigned an index here.
  for (int i = 0; i < iface->method_count; i++)
    iface->methods[i].index = i + 1;
}

// Prepare virtual method declarations in KLASS, and any superclasses
// as required, by determining their vtable index, setting
// method->index, and finally setting the class's vtable_method_count.
// Must be called with the lock for KLASS held.
void
_Jv_Linker::layout_vtable_methods (jclass klass)
{
  if (klass->vtable != NULL || klass->isInterface() 
      || klass->vtable_method_count != -1)
    return;

  jclass superclass = klass->getSuperclass();

  if (superclass != NULL && superclass->vtable_method_count == -1)
    {
      JvSynchronize sync (superclass);
      layout_vtable_methods (superclass);
    }

  int index = (superclass == NULL ? 0 : superclass->vtable_method_count);

  for (int i = 0; i < klass->method_count; ++i)
    {
      _Jv_Method *meth = &klass->methods[i];
      _Jv_Method *super_meth = NULL;

      if (! _Jv_isVirtualMethod (meth))
	continue;

      if (superclass != NULL)
	{
	  jclass declarer;
	  super_meth = _Jv_LookupDeclaredMethod (superclass, meth->name,
						 meth->signature, &declarer);
	  // See if this method actually overrides the other method
	  // we've found.
	  if (super_meth)
	    {
	      if (! _Jv_isVirtualMethod (super_meth)
		  || ! _Jv_CheckAccess (klass, declarer,
					super_meth->accflags))
		super_meth = NULL;
	      else if ((super_meth->accflags
			& java::lang::reflect::Modifier::FINAL) != 0)
		{
		  using namespace java::lang;
		  StringBuffer *sb = new StringBuffer();
		  sb->append(JvNewStringLatin1("method "));
		  sb->append(_Jv_GetMethodString(klass, meth));
		  sb->append(JvNewStringLatin1(" overrides final method "));
		  sb->append(_Jv_GetMethodString(declarer, super_meth));
		  throw new VerifyError(sb->toString());
		}
	      else if (declarer->loader != klass->loader)
		{
		  // JVMS 5.4.2.
		  check_loading_constraints (meth, klass, declarer);
		}
	    }
	}

      if (super_meth)
        meth->index = super_meth->index;
      else
	meth->index = index++;
    }

  klass->vtable_method_count = index;
}

// Set entries in VTABLE for virtual methods declared in KLASS.
void
_Jv_Linker::set_vtable_entries (jclass klass, _Jv_VTable *vtable)
{
  for (int i = klass->method_count - 1; i >= 0; i--)
    {
      using namespace java::lang::reflect;

      _Jv_Method *meth = &klass->methods[i];
      if (meth->index == (_Jv_ushort) -1)
	continue;
      if ((meth->accflags & Modifier::ABSTRACT))
	// FIXME: it might be nice to have a libffi trampoline here,
	// so we could pass in the method name and other information.
	vtable->set_method(meth->index,
			   (void *) &_Jv_ThrowAbstractMethodError);
      else
	vtable->set_method(meth->index, meth->ncode);
    }
}

// Allocate and lay out the virtual method table for KLASS.  This will
// also cause vtables to be generated for any non-abstract
// superclasses, and virtual method layout to occur for any abstract
// superclasses.  Must be called with monitor lock for KLASS held.
void
_Jv_Linker::make_vtable (jclass klass)
{
  using namespace java::lang::reflect;  

  // If the vtable exists, or for interface classes, do nothing.  All
  // other classes, including abstract classes, need a vtable.
  if (klass->vtable != NULL || klass->isInterface())
    return;

  // Ensure all the `ncode' entries are set.
  klass->engine->create_ncode(klass);

  // Class must be laid out before we can create a vtable. 
  if (klass->vtable_method_count == -1)
    layout_vtable_methods (klass);

  // Allocate the new vtable.
  _Jv_VTable *vtable = _Jv_VTable::new_vtable (klass->vtable_method_count);
  klass->vtable = vtable;

  // Copy the vtable of the closest superclass.
  jclass superclass = klass->superclass;
  {
    JvSynchronize sync (superclass);
    make_vtable (superclass);
  }
  for (int i = 0; i < superclass->vtable_method_count; ++i)
    vtable->set_method (i, superclass->vtable->get_method (i));

  // Set the class pointer and GC descriptor.
  vtable->clas = klass;
  vtable->gc_descr = _Jv_BuildGCDescr (klass);

  // For each virtual declared in klass, set new vtable entry or
  // override an old one.
  set_vtable_entries (klass, vtable);

  // Note that we don't check for abstract methods here.  We used to,
  // but there is a JVMS clarification that indicates that a check
  // here would be too eager.  And, a simple test case confirms this.
}

// Lay out the class, allocating space for static fields and computing
// offsets of instance fields.  The class lock must be held by the
// caller.
void
_Jv_Linker::ensure_fields_laid_out (jclass klass)
{  
  if (klass->size_in_bytes != -1)
    return;

  // Compute the alignment for this type by searching through the
  // superclasses and finding the maximum required alignment.  We
  // could consider caching this in the Class.
  int max_align = __alignof__ (java::lang::Object);
  jclass super = klass->getSuperclass();
  while (super != NULL)
    {
      // Ensure that our super has its super installed before
      // recursing.
      wait_for_state(super, JV_STATE_LOADING);
      ensure_fields_laid_out(super);
      int num = JvNumInstanceFields (super);
      _Jv_Field *field = JvGetFirstInstanceField (super);
      while (num > 0)
	{
	  int field_align = get_alignment_from_class (field->type);
	  if (field_align > max_align)
	    max_align = field_align;
	  ++field;
	  --num;
	}
      super = super->getSuperclass();
    }

  int instance_size;
  // This is the size of the 'static' non-reference fields.
  int non_reference_size = 0;
  // This is the size of the 'static' reference fields.  We count
  // these separately to make it simpler for the GC to scan them.
  int reference_size = 0;

  // Although java.lang.Object is never interpreted, an interface can
  // have a null superclass.  Note that we have to lay out an
  // interface because it might have static fields.
  if (klass->superclass)
    instance_size = klass->superclass->size();
  else
    instance_size = java::lang::Object::class$.size();

  klass->engine->allocate_field_initializers (klass); 

  for (int i = 0; i < klass->field_count; i++)
    {
      int field_size;
      int field_align;

      _Jv_Field *field = &klass->fields[i];

      if (! field->isRef ())
	{
	  // It is safe to resolve the field here, since it's a
	  // primitive class, which does not cause loading to happen.
	  resolve_field (field, klass->loader);
	  field_size = field->type->size ();
	  field_align = get_alignment_from_class (field->type);
	}
      else 
	{
	  field_size = sizeof (jobject);
	  field_align = __alignof__ (jobject);
	}

      field->bsize = field_size;

      if ((field->flags & java::lang::reflect::Modifier::STATIC))
	{
	  if (field->u.addr == NULL)
	    {
	      // This computes an offset into a region we'll allocate
	      // shortly, and then adds this offset to the start
	      // address.
	      if (field->isRef())
		{
		  reference_size = ROUND (reference_size, field_align);
		  field->u.boffset = reference_size;
		  reference_size += field_size;
		}
	      else
		{
		  non_reference_size = ROUND (non_reference_size, field_align);
		  field->u.boffset = non_reference_size;
		  non_reference_size += field_size;
		}
	    }
	}
      else
	{
	  instance_size      = ROUND (instance_size, field_align);
	  field->u.boffset   = instance_size;
	  instance_size     += field_size;
	  if (field_align > max_align)
	    max_align = field_align;
	}
    }

  if (reference_size != 0 || non_reference_size != 0)
    klass->engine->allocate_static_fields (klass, reference_size,
					   non_reference_size);

  // Set the instance size for the class.  Note that first we round it
  // to the alignment required for this object; this keeps us in sync
  // with our current ABI.
  instance_size = ROUND (instance_size, max_align);
  klass->size_in_bytes = instance_size;
}

// This takes the class to state JV_STATE_LINKED.  The class lock must
// be held when calling this.
void
_Jv_Linker::ensure_class_linked (jclass klass)
{
  if (klass->state >= JV_STATE_LINKED)
    return;

  int state = klass->state;
  try
    {
      // Short-circuit, so that mutually dependent classes are ok.
      klass->state = JV_STATE_LINKED;

      _Jv_Constants *pool = &klass->constants;

      // Compiled classes require that their class constants be
      // resolved here.  However, interpreted classes need their
      // constants to be resolved lazily.  If we resolve an
      // interpreted class' constants eagerly, we can end up with
      // spurious IllegalAccessErrors when the constant pool contains
      // a reference to a class we can't access.  This can validly
      // occur in an obscure case involving the InnerClasses
      // attribute.
      if (! _Jv_IsInterpretedClass (klass))
	{
	  // Resolve class constants first, since other constant pool
	  // entries may rely on these.
	  for (int index = 1; index < pool->size; ++index)
	    {
	      if (pool->tags[index] == JV_CONSTANT_Class)
                // Lazily resolve the entries.
		resolve_pool_entry (klass, index, true);
	    }
	}

      // Resolve the remaining constant pool entries.
      for (int index = 1; index < pool->size; ++index)
	{
	  jbyte tags;
	  _Jv_word data;

	  tags = read_cpool_entry (&data, pool, index);
	  if (tags == JV_CONSTANT_String)
	    {
	      data.o = _Jv_NewStringUtf8Const (data.utf8);
	      tags |= JV_CONSTANT_ResolvedFlag;
	      write_cpool_entry (data, tags, pool, index);
	    }
	}

      if (klass->engine->need_resolve_string_fields())
	{
	  jfieldID f = JvGetFirstStaticField (klass);
	  for (int n = JvNumStaticFields (klass); n > 0; --n)
	    {
	      int mod = f->getModifiers ();
	      // If we have a static String field with a non-null initial
	      // value, we know it points to a Utf8Const.

              // Finds out whether we have to initialize a String without the
              // need to resolve the field.
              if ((f->isResolved()
                   ? (f->type == &java::lang::String::class$)
                   : _Jv_equalUtf8Classnames((_Jv_Utf8Const *) f->type,
                                             java::lang::String::class$.name))
		  && (mod & java::lang::reflect::Modifier::STATIC) != 0)
		{
		  jstring *strp = (jstring *) f->u.addr;
		  if (*strp)
		    *strp = _Jv_NewStringUtf8Const ((_Jv_Utf8Const *) *strp);
		}
	      f = f->getNextField ();
	    }
	}

      klass->notifyAll ();

      _Jv_PushClass (klass);
    }
  catch (java::lang::Throwable *t)
    {
      klass->state = state;
      throw t;
    }
}

// This ensures that symbolic superclass and superinterface references
// are resolved for the indicated class.  This must be called with the
// class lock held.
void
_Jv_Linker::ensure_supers_installed (jclass klass)
{
  resolve_class_ref (klass, &klass->superclass);
  // An interface won't have a superclass.
  if (klass->superclass)
    wait_for_state (klass->superclass, JV_STATE_LOADING);

  for (int i = 0; i < klass->interface_count; ++i)
    {
      resolve_class_ref (klass, &klass->interfaces[i]);
      wait_for_state (klass->interfaces[i], JV_STATE_LOADING);
    }
}

// This adds missing `Miranda methods' to a class.
void
_Jv_Linker::add_miranda_methods (jclass base, jclass iface_class)
{
  // Note that at this point, all our supers, and the supers of all
  // our superclasses and superinterfaces, will have been installed.

  for (int i = 0; i < iface_class->interface_count; ++i)
    {
      jclass interface = iface_class->interfaces[i];

      for (int j = 0; j < interface->method_count; ++j)
	{
 	  _Jv_Method *meth = &interface->methods[j];
	  // Don't bother with <clinit>.
	  if (meth->name->first() == '<')
	    continue;
	  _Jv_Method *new_meth = _Jv_LookupDeclaredMethod (base, meth->name,
							   meth->signature);
	  if (! new_meth)
	    {
	      // We assume that such methods are very unlikely, so we
	      // just reallocate the method array each time one is
	      // found.  This greatly simplifies the searching --
	      // otherwise we have to make sure that each such method
	      // found is really unique among all superinterfaces.
	      int new_count = base->method_count + 1;
	      _Jv_Method *new_m
		= (_Jv_Method *) _Jv_AllocRawObj (sizeof (_Jv_Method)
						  * new_count);
	      memcpy (new_m, base->methods,
		      sizeof (_Jv_Method) * base->method_count);

	      // Add new method.
	      new_m[base->method_count] = *meth;
	      new_m[base->method_count].index = (_Jv_ushort) -1;
	      new_m[base->method_count].accflags
		|= java::lang::reflect::Modifier::INVISIBLE;

	      base->methods = new_m;
	      base->method_count = new_count;
	    }
	}

      wait_for_state (interface, JV_STATE_LOADED);
      add_miranda_methods (base, interface);
    }
}

// This ensures that the class' method table is "complete".  This must
// be called with the class lock held.
void
_Jv_Linker::ensure_method_table_complete (jclass klass)
{
  if (klass->vtable != NULL)
    return;

  // We need our superclass to have its own Miranda methods installed.
  if (! klass->isInterface())
    wait_for_state (klass->getSuperclass (), JV_STATE_LOADED);

  // A class might have so-called "Miranda methods".  This is a method
  // that is declared in an interface and not re-declared in an
  // abstract class.  Some compilers don't emit declarations for such
  // methods in the class; this will give us problems since we expect
  // a declaration for any method requiring a vtable entry.  We handle
  // this here by searching for such methods and constructing new
  // internal declarations for them.  Note that we do this
  // unconditionally, and not just for abstract classes, to correctly
  // account for cases where a class is modified to be concrete and
  // still incorrectly inherits an abstract method.
  int pre_count = klass->method_count;
  add_miranda_methods (klass, klass);

  // Let the execution engine know that we've added methods.
  if (klass->method_count != pre_count)
    klass->engine->post_miranda_hook(klass);
}

// Verify a class.  Must be called with class lock held.
void
_Jv_Linker::verify_class (jclass klass)
{
  klass->engine->verify(klass);
}

// Check the assertions contained in the type assertion table for KLASS.
// This is the equivilent of bytecode verification for native, BC-ABI code.
void
_Jv_Linker::verify_type_assertions (jclass klass)
{
  if (debug_link)
    fprintf (stderr, "Evaluating type assertions for %s:\n",
	     klass->name->chars());

  if (klass->assertion_table == NULL)
    return;

  for (int i = 0;; i++)
    {
      int assertion_code = klass->assertion_table[i].assertion_code;
      _Jv_Utf8Const *op1 = klass->assertion_table[i].op1;
      _Jv_Utf8Const *op2 = klass->assertion_table[i].op2;
      
      if (assertion_code == JV_ASSERT_END_OF_TABLE)
        return;
      else if (assertion_code == JV_ASSERT_TYPES_COMPATIBLE)
        {
	  if (debug_link)
	    {
	      fprintf (stderr, "  code=%i, operand A=%s B=%s\n",
		       assertion_code, op1->chars(), op2->chars());
	    }
	
	  // The operands are class signatures. op1 is the source, 
	  // op2 is the target.
	  jclass cl1 = _Jv_FindClassFromSignature (op1->chars(), 
	    klass->getClassLoaderInternal());
	  jclass cl2 = _Jv_FindClassFromSignature (op2->chars(),
	    klass->getClassLoaderInternal());
	    
	  // If the class doesn't exist, ignore the assertion. An exception
	  // will be thrown later if an attempt is made to actually 
	  // instantiate the class.
	  if (cl1 == NULL || cl2 == NULL)
	    continue;

          if (! _Jv_IsAssignableFromSlow (cl1, cl2))
	    {
	      jstring s = JvNewStringUTF ("Incompatible types: In class ");
	      s = s->concat (klass->getName());
	      s = s->concat (JvNewStringUTF (": "));
	      s = s->concat (cl1->getName());
	      s = s->concat (JvNewStringUTF (" is not assignable to "));
	      s = s->concat (cl2->getName());
	      throw new java::lang::VerifyError (s);
	    }
	}
      else if (assertion_code == JV_ASSERT_IS_INSTANTIABLE)
        {
	  // TODO: Implement this.
	}
      // Unknown assertion codes are ignored, for forwards-compatibility.
    }
}
   
void
_Jv_Linker::print_class_loaded (jclass klass)
{
  char *codesource = NULL;
  if (klass->protectionDomain != NULL)
    {
      java::security::CodeSource *cs
	= klass->protectionDomain->getCodeSource();
      if (cs != NULL)
	{
	  jstring css = cs->toString();
	  int len = JvGetStringUTFLength(css);
	  codesource = (char *) _Jv_AllocBytes(len + 1);
	  JvGetStringUTFRegion(css, 0, css->length(), codesource);
	  codesource[len] = '\0';
	}
    }
  if (codesource == NULL)
    codesource = (char *) "<no code source>";

  const char *abi;
  if (_Jv_IsInterpretedClass (klass))
    abi = "bytecode";
  else if (_Jv_IsBinaryCompatibilityABI (klass))
    abi = "BC-compiled";
  else
    abi = "pre-compiled";

  fprintf (stderr, "[Loaded (%s) %s from %s]\n", abi, klass->name->chars(),
	   codesource);
}

// FIXME: mention invariants and stuff.
void
_Jv_Linker::wait_for_state (jclass klass, int state)
{
  if (klass->state >= state)
    return;

  java::lang::Thread *self = java::lang::Thread::currentThread();

  {
    JvSynchronize sync (klass);

    // This is similar to the strategy for class initialization.  If we
    // already hold the lock, just leave.
    while (klass->state <= state
	   && klass->thread 
	   && klass->thread != self)
      klass->wait ();

    java::lang::Thread *save = klass->thread;
    klass->thread = self;

    // Allocate memory for static fields and constants.
    if (GC_base (klass) && klass->fields && ! GC_base (klass->fields))
      {
	jsize count = klass->field_count;
	if (count)
	  {
	    _Jv_Field* fields 
	      = (_Jv_Field*) _Jv_AllocRawObj (count * sizeof (_Jv_Field));
	    memcpy ((void*)fields,
		    (void*)klass->fields,
		    count * sizeof (_Jv_Field));
	    klass->fields = fields;
	  }
      }
      
  // Print some debugging info if requested.  Interpreted classes are
  // handled in defineclass, so we only need to handle the two
  // pre-compiled cases here.
  if ((klass->state == JV_STATE_COMPILED
	  || klass->state == JV_STATE_PRELOADING)
      && ! _Jv_IsInterpretedClass (klass))
    {
      if (gcj::verbose_class_flag)
	print_class_loaded (klass);
      ++gcj::loadedClasses;
    }

    try
      {
	if (state >= JV_STATE_LOADING && klass->state < JV_STATE_LOADING)
	  {
	    ensure_supers_installed (klass);
	    klass->set_state(JV_STATE_LOADING);
	  }

	if (state >= JV_STATE_LOADED && klass->state < JV_STATE_LOADED)
	  {
	    ensure_method_table_complete (klass);
	    klass->set_state(JV_STATE_LOADED);
	  }

	if (state >= JV_STATE_PREPARED && klass->state < JV_STATE_PREPARED)
	  {
	    ensure_fields_laid_out (klass);
	    make_vtable (klass);
	    layout_interface_methods (klass);
	    prepare_constant_time_tables (klass);
	    klass->set_state(JV_STATE_PREPARED);
	  }

	if (state >= JV_STATE_LINKED && klass->state < JV_STATE_LINKED)
	  {
	    if (gcj::verifyClasses)
	      verify_class (klass);

	    ensure_class_linked (klass);
	    link_exception_table (klass);
	    link_symbol_table (klass);
	    klass->set_state(JV_STATE_LINKED);
	  }
      }
    catch (java::lang::Throwable *exc)
      {
	klass->thread = save;
	klass->set_state(JV_STATE_ERROR);
	throw exc;
      }

    klass->thread = save;

    if (klass->state == JV_STATE_ERROR)
      throw new java::lang::LinkageError;
  }

#ifdef INTERPRETER
  if (__builtin_expect (klass->state == JV_STATE_LINKED, false)
      && state >= JV_STATE_LINKED
      && JVMTI_REQUESTED_EVENT (ClassPrepare))
    {
      JNIEnv *jni_env = _Jv_GetCurrentJNIEnv ();
      _Jv_JVMTI_PostEvent (JVMTI_EVENT_CLASS_PREPARE, self, jni_env,
			   klass);
    }
#endif
}
