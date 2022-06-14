// natReference.cc - Native code for References

/* Copyright (C) 2001, 2002, 2003, 2005, 2006  Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

// Written by Tom Tromey <tromey@redhat.com>

#include <config.h>

#include <gcj/cni.h>
#include <jvm.h>
#include <java/lang/Throwable.h>
#include <java/lang/ref/Reference.h>
#include <java/lang/ref/SoftReference.h>
#include <java/lang/ref/WeakReference.h>
#include <java/lang/ref/PhantomReference.h>
#include <java/lang/ref/ReferenceQueue.h>

static void finalize_reference (jobject ref);
static void finalize_referred_to_object (jobject obj);



enum weight
{
  SOFT = 0,
  WEAK = 1,
  FINALIZE = 2,
  PHANTOM = 3,

  // This is used to mark the head of a list.
  HEAD = 4,

  // This is used to mark a deleted item.
  DELETED = 5
};

// Objects of this type are used in the hash table to keep track of
// the mapping between a finalizable object and the various References
// which refer to it.
struct object_list
{
  // The reference object.  This is NULL for FINALIZE weight.
  jobject reference;

  // The weight of this object.
  enum weight weight;

  // Next in list.
  object_list *next;
};

// Hash table used to hold mapping from object to References.  The
// object_list item in the hash holds the object itself in the
// reference field; chained to it are all the references sorted in
// order of weight (lowest first).
static object_list *hash = NULL;

// Number of slots used in HASH.
static int hash_count = 0;

// Number of slots total in HASH.  Must be power of 2.
static int hash_size = 0;

#define DELETED_REFERENCE  ((jobject) -1)

static object_list *
find_slot (jobject key)
{
  jint hcode = _Jv_HashCode (key);
  /* step must be non-zero, and relatively prime with hash_size. */
  jint step = (hcode ^ (hcode >> 16)) | 1;
  int start_index = hcode & (hash_size - 1);
  int index = start_index;
  int deleted_index = -1;
  do
    {
      object_list *ptr = &hash[index];
      if (ptr->reference == key)
	return ptr;
      else if (ptr->reference == NULL)
	{
	  if (deleted_index == -1)
	    return ptr;
	  else
	    return &hash[deleted_index];
	}
      else if (ptr->weight == DELETED)
	{
	  deleted_index = index;
	  JvAssert (ptr->reference == DELETED_REFERENCE);
	}
      index = (index + step) & (hash_size - 1);
    }
  while (index != start_index);
  // Note that we can have INDEX == START_INDEX if the table has no
  // NULL entries but does have DELETED entries.
  JvAssert (deleted_index >= 0);
  return &hash[deleted_index];
}

static void
rehash ()
{
  if (hash == NULL)
    {
      hash_size = 1024;
      hash = (object_list *) _Jv_Malloc (hash_size * sizeof (object_list));
      memset (hash, 0, hash_size * sizeof (object_list));
    }
  else
    {
      object_list *old = hash;
      int i = hash_size;

      hash_size *= 2;
      hash = (object_list *) _Jv_Malloc (hash_size * sizeof (object_list));
      memset (hash, 0, hash_size * sizeof (object_list));

      while (--i >= 0)
	{
	  if (old[i].reference == NULL || old[i].weight == DELETED)
	    continue;
	  object_list *newslot = find_slot (old[i].reference);
	  *newslot = old[i];
	}

      _Jv_Free (old);
    }
}

// Remove a Reference.
static void
remove_from_hash (jobject obj)
{
  java::lang::ref::Reference *ref
    = reinterpret_cast<java::lang::ref::Reference *> (obj);
  object_list *head = find_slot (ref->copy);

  // We might have found a new slot.  We can just ignore that here.
  if (head->reference != ref->copy)
    return;

  object_list **link = &head->next;
  head = head->next;

  while (head && head->reference != ref)
    {
      link = &head->next;
      head = head->next;
    }

  // Remove the slot.
  if (head)
    {
      *link = head->next;
      _Jv_Free (head);
    }
}

// Return list head if object is in hash, NULL otherwise.
object_list *
in_hash (jobject obj)
{
  // The hash table might not yet be initialized.
  if (hash == NULL)
    return NULL;
  object_list *head = find_slot (obj);
  if (head->reference != obj)
    return NULL;
  return head;
}

// FIXME what happens if an object's finalizer creates a Reference to
// the object, and the object has never before been added to the hash?
// Madness!

// Add an item to the hash table.  If the item is new, we also add a
// finalizer item.  We keep items in the hash table until they are
// completely collected; this lets us know when an item is new, even
// if it has been resurrected after its finalizer has been run.
static void
add_to_hash (java::lang::ref::Reference *the_reference)
{
  JvSynchronize sync (java::lang::ref::Reference::lock);

  if (3 * hash_count >= 2 * hash_size)
    rehash ();

  // Use `copy' here because the `referent' field has been cleared.
  jobject referent = the_reference->copy;
  object_list *item = find_slot (referent);
  if (item->reference == NULL || item->reference == DELETED_REFERENCE)
    {
      // New item, so make an entry for the finalizer.
      item->reference = referent;
      item->weight = HEAD;

      item->next = (object_list *) _Jv_Malloc (sizeof (object_list));
      item->next->reference = NULL;
      item->next->weight = FINALIZE;
      item->next->next = NULL;
      ++hash_count;
    }

  object_list *n = (object_list *) _Jv_Malloc (sizeof (object_list));
  n->reference = the_reference;

  enum weight w = PHANTOM;
  if (java::lang::ref::SoftReference::class$.isInstance (the_reference))
    w = SOFT;
  else if (java::lang::ref::WeakReference::class$.isInstance (the_reference))
    w = WEAK;
  n->weight = w;

  object_list **link = &item->next;
  object_list *iter = *link;
  while (iter && iter->weight < n->weight)
    {
      link = &iter->next;
      iter = *link;
    }
  n->next = *link;
  *link = n;
}

// Add a FINALIZE entry if one doesn't exist.
static void
maybe_add_finalize (object_list *entry, jobject obj)
{
  object_list **link = &entry->next;
  object_list *iter = *link;
  while (iter && iter->weight < FINALIZE)
    {
      link = &iter->next;
      iter = *link;
    }

  // We want at most one FINALIZE entry in the queue.
  if (iter && iter->weight == FINALIZE)
    return;

  object_list *n = (object_list *) _Jv_Malloc (sizeof (object_list));
  n->reference = obj;
  n->weight = FINALIZE;
  n->next = *link;
  *link = n;
}

// This is called when an object is ready to be finalized.  This
// actually implements the appropriate Reference semantics.
static void
finalize_referred_to_object (jobject obj)
{
  JvSynchronize sync (java::lang::ref::Reference::lock);

  object_list *list = find_slot (obj);
  object_list *head = list->next;
  if (head == NULL)
    {
      // We have a truly dead object: the object's finalizer has been
      // run, all the object's references have been processed, and the
      // object is unreachable.  There is, at long last, no way to
      // resurrect it.
      list->reference = DELETED_REFERENCE;
      list->weight = DELETED;
      --hash_count;
      return;
    }

  enum weight w = head->weight;
  if (w == FINALIZE)
    {
      // Update the list first, as _Jv_FinalizeString might end up
      // looking at this data structure.
      list->next = head->next;
      _Jv_Free (head);

      // If we have a Reference A to a Reference B, and B is
      // finalized, then we have to take special care to make sure
      // that B is properly deregistered.  This is super gross.  FIXME
      // will it fail if B's finalizer resurrects B?
      if (java::lang::ref::Reference::class$.isInstance (obj))
	finalize_reference (obj);
      else if (obj->getClass() == &java::lang::String::class$)
	_Jv_FinalizeString (obj);
      else
	_Jv_FinalizeObject (obj);
    }
  else if (w != SOFT || _Jv_GCCanReclaimSoftReference (obj))
    {
      // If we just decided to reclaim a soft reference, we might as
      // well do all the weak references at the same time.
      if (w == SOFT)
	w = WEAK;

      while (head && head->weight <= w)
	{
	  java::lang::ref::Reference *ref
	    = reinterpret_cast<java::lang::ref::Reference *> (head->reference);
	  if (! ref->cleared)
	    ref->enqueue ();

	  object_list *next = head->next;
	  _Jv_Free (head);
	  head = next;
	}
      list->next = head;
    }

  // Re-register this finalizer.  We always re-register because we
  // can't know until the next collection cycle whether or not the
  // object is truly unreachable.
  _Jv_RegisterFinalizer (obj, finalize_referred_to_object);
}

// This is called when a Reference object is finalized.  If there is a
// Reference pointing to this Reference then that case is handled by
// finalize_referred_to_object.
static void
finalize_reference (jobject ref)
{
  JvSynchronize sync (java::lang::ref::Reference::lock);
  remove_from_hash (ref);
  // The user might have a subclass of Reference with a finalizer.
  _Jv_FinalizeObject (ref);
}

void
_Jv_RegisterStringFinalizer (jobject str)
{
  // This function might be called before any other Reference method,
  // so we must ensure the class is initialized.
  _Jv_InitClass (&java::lang::ref::Reference::class$);
  JvSynchronize sync (java::lang::ref::Reference::lock);
  // If the object is in our hash table, then we might need to add a
  // new FINALIZE entry.  Otherwise, we just register an ordinary
  // finalizer.
  object_list *entry = in_hash (str);
  if (entry)
    maybe_add_finalize (entry, str);
  else
    _Jv_RegisterFinalizer ((void *) str, _Jv_FinalizeString);
}

void
::java::lang::ref::Reference::create (jobject ref)
{
  // Nothing says you can't make a Reference with a NULL referent.
  // But there's nothing to do in such a case.
  referent = reinterpret_cast<gnu::gcj::RawData *> (ref);
  copy = referent;
  if (referent != NULL)
    {
      JvSynchronize sync (java::lang::ref::Reference::lock);
      // `this' is a new Reference object.  We register a new
      // finalizer for pointed-to object and we arrange a special
      // finalizer for ourselves as well.
      _Jv_RegisterFinalizer (this, finalize_reference);
      _Jv_RegisterFinalizer (referent, finalize_referred_to_object);
      gnu::gcj::RawData **p = &referent;
     _Jv_GCRegisterDisappearingLink ((jobject *) p);
      add_to_hash (this);
    }
}

::java::lang::Object *
::java::lang::ref::Reference::get()
{
  JvSynchronize sync (lock);
  return referent;
}
