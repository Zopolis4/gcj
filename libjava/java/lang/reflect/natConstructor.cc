// natConstructor.cc - Native code for Constructor class.

/* Copyright (C) 1999, 2000, 2001, 2002, 2003, 2006  Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

#include <config.h>

#include <gcj/cni.h>
#include <jvm.h>
#include <java-stack.h>

#include <java/lang/ArrayIndexOutOfBoundsException.h>
#include <java/lang/IllegalAccessException.h>
#include <java/lang/reflect/Constructor.h>
#include <java/lang/reflect/Method.h>
#include <java/lang/reflect/InvocationTargetException.h>
#include <java/lang/reflect/Modifier.h>
#include <java/lang/InstantiationException.h>
#include <gcj/method.h>

typedef JArray< ::java::lang::annotation::Annotation * > * anno_a_t;
typedef JArray< JArray< ::java::lang::annotation::Annotation * > *> * anno_aa_t;

jint
java::lang::reflect::Constructor::getModifiersInternal ()
{
  return _Jv_FromReflectedConstructor (this)->accflags;
}

jstring
java::lang::reflect::Constructor::getSignature()
{
  return declaringClass->getReflectionSignature (this);
}

anno_a_t
java::lang::reflect::Constructor::getDeclaredAnnotationsInternal()
{
  return (anno_a_t) declaringClass->getDeclaredAnnotations(this, false);
}

anno_aa_t
java::lang::reflect::Constructor::getParameterAnnotationsInternal()
{
  return (anno_aa_t) declaringClass->getDeclaredAnnotations(this, true);
}

void
java::lang::reflect::Constructor::getType ()
{
  _Jv_GetTypesFromSignature (_Jv_FromReflectedConstructor (this),
			     declaringClass,
			     &parameter_types,
			     NULL);

  // FIXME: for now we have no way to get exception information.
  exception_types = 
    (JArray<jclass> *) JvNewObjectArray (0, &java::lang::Class::class$, NULL);
}

jobject
java::lang::reflect::Constructor::newInstance (jobjectArray args)
{
  using namespace java::lang::reflect;

  if (parameter_types == NULL)
    getType ();

  jmethodID meth = _Jv_FromReflectedConstructor (this);

  // Check accessibility, if required.
  if (! (Modifier::isPublic (meth->accflags) || this->isAccessible()))
    {
      Class *caller = _Jv_StackTrace::GetCallingClass (&Constructor::class$);
      if (! _Jv_CheckAccess(caller, declaringClass, meth->accflags))
	throw new IllegalAccessException;
    }

  if (Modifier::isAbstract (declaringClass->getModifiers()))
    throw new InstantiationException;

  _Jv_InitClass (declaringClass);

  // In the constructor case the return type is the type of the
  // constructor.
  return _Jv_CallAnyMethodA (NULL, declaringClass, meth, true,
			     parameter_types, args);
}
