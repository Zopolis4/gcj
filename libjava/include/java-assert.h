// java-assert.h - Header file holding assertion definitions.  -*- c++ -*-

/* Copyright (C) 1998, 1999, 2010  Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

#ifndef __JAVA_ASSERT_H__
#define __JAVA_ASSERT_H__

// This is a libgcj implementation header.

void _Jv_Abort (const char *, const char *, int, const char *)
  __attribute__ ((__noreturn__));

#ifdef __GCJ_DEBUG
#define _Jv_AssertDoCall(Message) _Jv_Abort (__FUNCTION__, __FILE__, __LINE__, Message)

#define JvAssertMessage(Expr, Message) \
   do { if (! (Expr)) _Jv_AssertDoCall (Message); } while (0)
#define JvAssert(Expr) \
   do { if (! (Expr)) _Jv_AssertDoCall (# Expr); } while (0)

#define JvFail(Message) _Jv_AssertDoCall (Message)

#else /* __GCJ_DEBUG */

#define _Jv_AssertDoCall(Message)
#define JvAssertMessage(Expr, Message) (void) sizeof (Expr)
#define JvAssert(Expr) (void) sizeof (Expr)
#define JvFail(Message) _Jv_Abort (0, 0, 0, Message)

#endif /* not __GCJ_DEBUG */

#endif /* __JAVA_ASSERT_H__ */
