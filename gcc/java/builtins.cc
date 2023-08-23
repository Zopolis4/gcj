/* Built-in and inline functions for gcj
   Copyright (C) 2001-2016 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

.

.  

Java and all Java-based marks are trademarks or registered trademarks
of Sun Microsystems, Inc. in the United States and other countries.
The Free Software Foundation is independent of Sun Microsystems, Inc.  */

/* Written by Tom Tromey <tromey@redhat.com>.  */

/* FIXME: Still need to include rtl.h here (see below).  */
#undef IN_GCC_FRONTEND

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "tree.h"
#include "stringpool.h"
#include "expmed.h"
#include "optabs.h"
#include "fold-const.h"
#include "stor-layout.h"
#include "java-tree.h"

/* FIXME: All these headers are necessary for sync_compare_and_swap.
   Front ends should never have to look at that.  */

static tree max_builtin (tree, tree);
static tree min_builtin (tree, tree);
static tree abs_builtin (tree, tree);
static tree convert_real (tree, tree);

static tree java_build_function_call_expr (tree, tree);

static tree putObject_builtin (tree, tree);
static tree compareAndSwapInt_builtin (tree, tree);
static tree compareAndSwapLong_builtin (tree, tree);
static tree compareAndSwapObject_builtin (tree, tree);
static tree putVolatile_builtin (tree, tree);
static tree getVolatile_builtin (tree, tree);
static tree VMSupportsCS8_builtin (tree, tree);


/* Functions of this type are used to inline a given call.  Such a
   function should either return an expression, if the call is to be
   inlined, or NULL_TREE if a real call should be emitted.  Arguments
   are method return type and the original CALL_EXPR containing the
   arguments to the call.  */
typedef tree builtin_creator_function (tree, tree);

/* Hold a char*, before initialization, or a tree, after
   initialization.  */
union GTY(()) string_or_tree {
  const char * GTY ((tag ("0"))) s;
  tree GTY ((tag ("1"))) t;
};

/* Used to hold a single builtin record.  */
struct GTY(()) builtin_record {
  union string_or_tree GTY ((desc ("1"))) class_name;
  union string_or_tree GTY ((desc ("1"))) method_name;
  builtin_creator_function * GTY((skip)) creator;
  enum built_in_function builtin_code;
};

static GTY(()) struct builtin_record java_builtins[] =
{
  { { "java.lang.Math" }, { "min" }, min_builtin, (enum built_in_function) 0 },
  { { "java.lang.Math" }, { "max" }, max_builtin, (enum built_in_function) 0 },
  { { "java.lang.Math" }, { "abs" }, abs_builtin, (enum built_in_function) 0 },
  { { "java.lang.Math" }, { "acos" }, NULL, BUILT_IN_ACOS },
  { { "java.lang.Math" }, { "asin" }, NULL, BUILT_IN_ASIN },
  { { "java.lang.Math" }, { "atan" }, NULL, BUILT_IN_ATAN },
  { { "java.lang.Math" }, { "atan2" }, NULL, BUILT_IN_ATAN2 },
  { { "java.lang.Math" }, { "ceil" }, NULL, BUILT_IN_CEIL },
  { { "java.lang.Math" }, { "cos" }, NULL, BUILT_IN_COS },
  { { "java.lang.Math" }, { "exp" }, NULL, BUILT_IN_EXP },
  { { "java.lang.Math" }, { "floor" }, NULL, BUILT_IN_FLOOR },
  { { "java.lang.Math" }, { "log" }, NULL, BUILT_IN_LOG },
  { { "java.lang.Math" }, { "pow" }, NULL, BUILT_IN_POW },
  { { "java.lang.Math" }, { "sin" }, NULL, BUILT_IN_SIN },
  { { "java.lang.Math" }, { "sqrt" }, NULL, BUILT_IN_SQRT },
  { { "java.lang.Math" }, { "tan" }, NULL, BUILT_IN_TAN },
  { { "java.lang.Integer" }, { "bitCount" }, NULL, BUILT_IN_POPCOUNT },
  { { "java.lang.Integer" }, { "reverseBytes" }, NULL, BUILT_IN_BSWAP32 },
  { { "java.lang.Long" }, { "bitCount" }, NULL, BUILT_IN_POPCOUNTL },
  { { "java.lang.Long" }, { "reverseBytes" }, NULL, BUILT_IN_BSWAP64 },
  { { "java.lang.Short" }, { "reverseBytes" }, NULL, BUILT_IN_BSWAP16 },
  { { "java.lang.Float" }, { "intBitsToFloat" }, convert_real,
    (enum built_in_function) 0 },
  { { "java.lang.Double" }, { "longBitsToDouble" }, convert_real,
    (enum built_in_function) 0 },
  { { "java.lang.Float" }, { "floatToRawIntBits" }, convert_real,
    (enum built_in_function) 0 },
  { { "java.lang.Double" }, { "doubleToRawLongBits" }, convert_real,
    (enum built_in_function) 0 },
  { { "sun.misc.Unsafe" }, { "putInt" }, putObject_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "putLong" }, putObject_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "putObject" }, putObject_builtin,
  (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "compareAndSwapInt" },
    compareAndSwapInt_builtin, (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "compareAndSwapLong" },
    compareAndSwapLong_builtin, (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "compareAndSwapObject" },
    compareAndSwapObject_builtin, (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "putOrderedInt" }, putVolatile_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "putOrderedLong" }, putVolatile_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "putOrderedObject" }, putVolatile_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "putIntVolatile" }, putVolatile_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "putLongVolatile" }, putVolatile_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "putObjectVolatile" }, putVolatile_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "getObjectVolatile" }, getVolatile_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "getIntVolatile" }, getVolatile_builtin,
    (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "getLongVolatile" }, getVolatile_builtin, (enum built_in_function) 0},
  { { "sun.misc.Unsafe" }, { "getLong" }, getVolatile_builtin,
    (enum built_in_function) 0},
  { { "java.util.concurrent.atomic.AtomicLong" }, { "VMSupportsCS8" },
    VMSupportsCS8_builtin, (enum built_in_function) 0},
  { { NULL }, { NULL }, NULL, END_BUILTINS }
};


/* Internal functions which implement various builtin conversions.  */

static tree
max_builtin (tree method_return_type, tree orig_call)
{
  /* MAX_EXPR does not handle -0.0 in the Java style.  */
  if (TREE_CODE (method_return_type) == REAL_TYPE)
    return NULL_TREE;
  return fold_build2 (MAX_EXPR, method_return_type,
		      CALL_EXPR_ARG (orig_call, 0),
		      CALL_EXPR_ARG (orig_call, 1));
}

static tree
min_builtin (tree method_return_type, tree orig_call)
{
  /* MIN_EXPR does not handle -0.0 in the Java style.  */
  if (TREE_CODE (method_return_type) == REAL_TYPE)
    return NULL_TREE;
  return fold_build2 (MIN_EXPR, method_return_type,
		      CALL_EXPR_ARG (orig_call, 0),
		      CALL_EXPR_ARG (orig_call, 1));
}

static tree
abs_builtin (tree method_return_type, tree orig_call)
{
  return fold_build1 (ABS_EXPR, method_return_type,
		      CALL_EXPR_ARG (orig_call, 0));
}

/* Construct a new call to FN using the arguments from ORIG_CALL.  */

static tree
java_build_function_call_expr (tree fn, tree orig_call)
{
  int nargs = call_expr_nargs (orig_call);
  switch (nargs)
    {
      /* Although we could handle the 0-3 argument cases using the general
	 logic in the default case, splitting them out permits folding to
	 be performed without constructing a temporary CALL_EXPR.  */
    case 0:
      return build_call_expr (fn, 0);
    case 1:
      return build_call_expr (fn, 1, CALL_EXPR_ARG (orig_call, 0));
    case 2:
      return build_call_expr (fn, 2,
			      CALL_EXPR_ARG (orig_call, 0),
			      CALL_EXPR_ARG (orig_call, 1));
    case 3:
      return build_call_expr (fn, 3,
			      CALL_EXPR_ARG (orig_call, 0),
			      CALL_EXPR_ARG (orig_call, 1),
			      CALL_EXPR_ARG (orig_call, 2));
    default:
      {
	tree fntype = TREE_TYPE (fn);
	fn = build1 (ADDR_EXPR, build_pointer_type (fntype), fn);
	return fold (build_call_array (TREE_TYPE (fntype),
				       fn, nargs, CALL_EXPR_ARGP (orig_call)));
      }
    }
}

static tree
convert_real (tree method_return_type, tree orig_call)
{
  return build1 (VIEW_CONVERT_EXPR, method_return_type,
		 CALL_EXPR_ARG (orig_call, 0));
}



/* Provide builtin support for atomic operations.  These are
   documented at length in libjava/sun/misc/Unsafe.java.  */

/* FIXME.  There are still a few things wrong with this logic.  In
   particular, atomic writes of multi-word integers are not truly
   atomic: this requires more work.

   In general, double-word compare-and-swap cannot portably be
   implemented, so we need some kind of fallback for 32-bit machines.

*/


/* Macros to unmarshal arguments from a CALL_EXPR into a few
   variables.  We also convert the offset arg from a long to an
   integer that is the same size as a pointer.  */

#define UNMARSHAL3(METHOD_CALL)					\
tree this_arg, obj_arg, offset_arg;					\
do									\
{									\
  tree orig_method_call = METHOD_CALL;					\
  this_arg = CALL_EXPR_ARG (orig_method_call, 0);			\
  obj_arg = CALL_EXPR_ARG (orig_method_call, 1);			\
  offset_arg = fold_convert (java_type_for_size (POINTER_SIZE, 0),	\
			     CALL_EXPR_ARG (orig_method_call, 2));	\
}									\
while (0)

#define UNMARSHAL4(METHOD_CALL)					\
tree value_type, this_arg, obj_arg, offset_arg, value_arg;		\
do									\
{									\
  tree orig_method_call = METHOD_CALL;					\
  this_arg = CALL_EXPR_ARG (orig_method_call, 0);			\
  obj_arg = CALL_EXPR_ARG (orig_method_call, 1);			\
  offset_arg = fold_convert (java_type_for_size (POINTER_SIZE, 0),	\
			     CALL_EXPR_ARG (orig_method_call, 2));	\
  value_arg = CALL_EXPR_ARG (orig_method_call, 3);			\
  value_type = TREE_TYPE (value_arg);					\
}									\
while (0)

#define UNMARSHAL5(METHOD_CALL)					\
tree value_type, this_arg, obj_arg, offset_arg, expected_arg, value_arg; \
do									\
{									\
  tree orig_method_call = METHOD_CALL;					\
  this_arg = CALL_EXPR_ARG (orig_method_call, 0);			\
  obj_arg = CALL_EXPR_ARG (orig_method_call, 1);			\
  offset_arg = fold_convert (java_type_for_size (POINTER_SIZE, 0),	\
			     CALL_EXPR_ARG (orig_method_call, 2));	\
  expected_arg = CALL_EXPR_ARG (orig_method_call, 3);			\
  value_arg = CALL_EXPR_ARG (orig_method_call, 4);			\
  value_type = TREE_TYPE (value_arg);					\
}									\
while (0)

/* Add an address to an offset, forming a sum.  */

static tree
build_addr_sum (tree type, tree addr, tree offset)
{
  tree ptr_type = build_pointer_type (type);
  return fold_build_pointer_plus (fold_convert (ptr_type, addr), offset);
}

/* Make sure that this-arg is non-NULL.  This is a security check.  */

static tree
build_check_this (tree stmt, tree this_arg)
{
  return build2 (COMPOUND_EXPR, TREE_TYPE (stmt), 
		 java_check_reference (this_arg, 1), stmt);
}

/* Now the builtins.  These correspond to the primitive functions in
   libjava/sun/misc/natUnsafe.cc.  */

static tree
putObject_builtin (tree method_return_type ATTRIBUTE_UNUSED, 
		   tree orig_call)
{
  tree addr, stmt;
  UNMARSHAL4 (orig_call);

  addr = build_addr_sum (value_type, obj_arg, offset_arg);
  stmt = fold_build2 (MODIFY_EXPR, value_type,
		      build_java_indirect_ref (value_type, addr,
					       flag_check_references),
		      value_arg);

  return build_check_this (stmt, this_arg);
}

static tree
compareAndSwapInt_builtin (tree method_return_type ATTRIBUTE_UNUSED,
			   tree orig_call)
{
  machine_mode mode = TYPE_MODE (int_type_node);
  if (can_compare_and_swap_p (mode, flag_use_atomic_builtins))
    {
      tree addr, stmt;
      enum built_in_function fncode = BUILT_IN_SYNC_BOOL_COMPARE_AND_SWAP_4;
      UNMARSHAL5 (orig_call);
      (void) value_type; /* Avoid set but not used warning.  */

      addr = build_addr_sum (int_type_node, obj_arg, offset_arg);
      stmt = build_call_expr (builtin_decl_explicit (fncode),
			      3, addr, expected_arg, value_arg);

      return build_check_this (stmt, this_arg);
    }
  return NULL_TREE;
}

static tree
compareAndSwapLong_builtin (tree method_return_type ATTRIBUTE_UNUSED,
			    tree orig_call)
{
  machine_mode mode = TYPE_MODE (long_type_node);
  /* We don't trust flag_use_atomic_builtins for multi-word compareAndSwap.
     Some machines such as ARM have atomic libfuncs but not the multi-word
     versions.  */
  if (can_compare_and_swap_p (mode,
			      (flag_use_atomic_builtins
			       && known_le (GET_MODE_SIZE (mode), UNITS_PER_WORD))))
    {
      tree addr, stmt;
      enum built_in_function fncode = BUILT_IN_SYNC_BOOL_COMPARE_AND_SWAP_8;
      UNMARSHAL5 (orig_call);
      (void) value_type; /* Avoid set but not used warning.  */

      addr = build_addr_sum (long_type_node, obj_arg, offset_arg);
      stmt = build_call_expr (builtin_decl_explicit (fncode),
			      3, addr, expected_arg, value_arg);

      return build_check_this (stmt, this_arg);
    }
  return NULL_TREE;
}
static tree
compareAndSwapObject_builtin (tree method_return_type ATTRIBUTE_UNUSED, 
			      tree orig_call)
{
  machine_mode mode = TYPE_MODE (ptr_type_node);
  if (can_compare_and_swap_p (mode, flag_use_atomic_builtins))
  {
    tree addr, stmt;
    enum built_in_function builtin;

    UNMARSHAL5 (orig_call);
    builtin = (POINTER_SIZE == 32 
	       ? BUILT_IN_SYNC_BOOL_COMPARE_AND_SWAP_4 
	       : BUILT_IN_SYNC_BOOL_COMPARE_AND_SWAP_8);

    addr = build_addr_sum (value_type, obj_arg, offset_arg);
    stmt = build_call_expr (builtin_decl_explicit (builtin),
			    3, addr, expected_arg, value_arg);

    return build_check_this (stmt, this_arg);
  }
  return NULL_TREE;
}

static tree
putVolatile_builtin (tree method_return_type ATTRIBUTE_UNUSED, 
		     tree orig_call)
{
  tree addr, stmt, modify_stmt;
  UNMARSHAL4 (orig_call);
  
  addr = build_addr_sum (value_type, obj_arg, offset_arg);
  addr 
    = fold_convert (build_pointer_type (build_qualified_type
					(value_type, TYPE_QUAL_VOLATILE)),
		    addr);
  
  stmt = build_call_expr (builtin_decl_explicit (BUILT_IN_SYNC_SYNCHRONIZE), 0);
  modify_stmt = fold_build2 (MODIFY_EXPR, value_type,
			     build_java_indirect_ref (value_type, addr,
						      flag_check_references),
			     value_arg);
  stmt = build2 (COMPOUND_EXPR, TREE_TYPE (modify_stmt), 
		 stmt, modify_stmt);

  return build_check_this (stmt, this_arg);
}

static tree
getVolatile_builtin (tree method_return_type ATTRIBUTE_UNUSED, 
		     tree orig_call)
{
  tree addr, stmt, modify_stmt, tmp;
  UNMARSHAL3 (orig_call);
  (void) this_arg; /* Avoid set but not used warning.  */

  addr = build_addr_sum (method_return_type, obj_arg, offset_arg);
  addr 
    = fold_convert (build_pointer_type (build_qualified_type
					(method_return_type,
					 TYPE_QUAL_VOLATILE)), addr);
  
  stmt = build_call_expr (builtin_decl_explicit (BUILT_IN_SYNC_SYNCHRONIZE), 0);
  tmp = build_decl (BUILTINS_LOCATION, VAR_DECL, NULL, method_return_type);
  DECL_IGNORED_P (tmp) = 1;
  DECL_ARTIFICIAL (tmp) = 1;
  pushdecl (tmp);

  modify_stmt = fold_build2 (MODIFY_EXPR, method_return_type,
			     tmp,
			     build_java_indirect_ref (method_return_type, addr,
						      flag_check_references));

  stmt = build2 (COMPOUND_EXPR, void_type_node, modify_stmt, stmt);
  stmt = build2 (COMPOUND_EXPR, method_return_type, stmt, tmp);
  
  return stmt;
}

static tree
VMSupportsCS8_builtin (tree method_return_type, 
		       tree orig_call ATTRIBUTE_UNUSED)
{
  machine_mode mode = TYPE_MODE (long_type_node);
  gcc_assert (method_return_type == boolean_type_node);
  if (can_compare_and_swap_p (mode, false))
    return boolean_true_node;
  else
    return boolean_false_node;
}  



/* Define a single builtin.  */
static void
define_builtin (enum built_in_function val,
		const char *name,
		tree type,
		const char *libname,
		int flags)
{
  tree decl;

  decl = build_decl (BUILTINS_LOCATION,
		     FUNCTION_DECL, get_identifier (name), type);
  DECL_EXTERNAL (decl) = 1;
  TREE_PUBLIC (decl) = 1;
  SET_DECL_ASSEMBLER_NAME (decl, get_identifier (libname));
  pushdecl (decl);
  set_decl_built_in_function (decl, BUILT_IN_NORMAL, val);
  set_call_expr_flags (decl, flags);

  set_builtin_decl (val, decl, true);
}



/* Initialize the builtins.  */
void
initialize_builtins (void)
{
  tree double_ftype_double, double_ftype_double_double;
  tree float_ftype_float_float;
  tree boolean_ftype_boolean_boolean;
  tree int_ftype_int;
  int i;

  for (i = 0; java_builtins[i].builtin_code != END_BUILTINS; ++i)
    {
      tree klass_id = get_identifier (java_builtins[i].class_name.s);
      tree m = get_identifier (java_builtins[i].method_name.s);

      java_builtins[i].class_name.t = klass_id;
      java_builtins[i].method_name.t = m;
    }

  void_list_node = end_params_node;

  float_ftype_float_float
    = build_function_type_list (float_type_node,
				float_type_node, float_type_node, NULL_TREE);

  double_ftype_double
    = build_function_type_list (double_type_node, double_type_node, NULL_TREE);
  double_ftype_double_double
    = build_function_type_list (double_type_node,
				double_type_node, double_type_node, NULL_TREE);

  define_builtin (BUILT_IN_FMOD, "__builtin_fmod",
		  double_ftype_double_double, "fmod", ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_FMODF, "__builtin_fmodf",
		  float_ftype_float_float, "fmodf", ECF_CONST | ECF_LEAF);

  define_builtin (BUILT_IN_ACOS, "__builtin_acos",
		  double_ftype_double, "_ZN4java4lang4Math4acosEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_ASIN, "__builtin_asin",
		  double_ftype_double, "_ZN4java4lang4Math4asinEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_ATAN, "__builtin_atan",
		  double_ftype_double, "_ZN4java4lang4Math4atanEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_ATAN2, "__builtin_atan2",
		  double_ftype_double_double, "_ZN4java4lang4Math5atan2EJddd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_CEIL, "__builtin_ceil",
		  double_ftype_double, "_ZN4java4lang4Math4ceilEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_COS, "__builtin_cos",
		  double_ftype_double, "_ZN4java4lang4Math3cosEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_EXP, "__builtin_exp",
		  double_ftype_double, "_ZN4java4lang4Math3expEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_FLOOR, "__builtin_floor",
		  double_ftype_double, "_ZN4java4lang4Math5floorEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_LOG, "__builtin_log",
		  double_ftype_double, "_ZN4java4lang4Math3logEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_POW, "__builtin_pow",
		  double_ftype_double_double, "_ZN4java4lang4Math3powEJddd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_SIN, "__builtin_sin",
		  double_ftype_double, "_ZN4java4lang4Math3sinEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_SQRT, "__builtin_sqrt",
		  double_ftype_double, "_ZN4java4lang4Math4sqrtEJdd",
		  ECF_CONST | ECF_LEAF);
  define_builtin (BUILT_IN_TAN, "__builtin_tan",
		  double_ftype_double, "_ZN4java4lang4Math3tanEJdd",
		  ECF_CONST | ECF_LEAF);

  int_ftype_int = build_function_type_list (int_type_node,
                                            int_type_node, NULL_TREE);

  define_builtin (BUILT_IN_POPCOUNT, "__builtin_popcount", int_ftype_int,
                  "_ZN4java4lang7Integer8bitCountEJii",
                  ECF_CONST | ECF_LEAF | ECF_NOTHROW);
  define_builtin (BUILT_IN_BSWAP32, "__builtin_bswap32", int_ftype_int,
		  "_ZN4java4lang7Integer12reverseBytesEJii",
                  ECF_CONST | ECF_LEAF | ECF_NOTHROW);

  define_builtin (BUILT_IN_POPCOUNTL, "__builtin_popcountl",
                  build_function_type_list (int_type_node,
					    long_type_node, NULL_TREE),
		  "_ZN4java4lang4Long8bitCountEJix",
                  ECF_CONST | ECF_LEAF | ECF_NOTHROW);
  define_builtin (BUILT_IN_BSWAP64, "__builtin_bswap64",
		  build_function_type_list (long_type_node,
					    long_type_node, NULL_TREE),
		  "_ZN4java4lang4Long12reverseBytesEJxx",
                  ECF_CONST | ECF_LEAF | ECF_NOTHROW);
                  
  define_builtin (BUILT_IN_BSWAP16, "__builtin_bswap16",
		  build_function_type_list (short_type_node,
					    short_type_node, NULL_TREE),
		  "_ZN4java4lang5Short12reverseBytesEJss",
                  ECF_CONST | ECF_LEAF | ECF_NOTHROW);

  boolean_ftype_boolean_boolean
    = build_function_type_list (boolean_type_node,
				boolean_type_node, boolean_type_node,
				NULL_TREE);
  define_builtin (BUILT_IN_EXPECT, "__builtin_expect", 
		  boolean_ftype_boolean_boolean,
		  "__builtin_expect",
		  ECF_CONST | ECF_NOTHROW);
  define_builtin (BUILT_IN_SYNC_BOOL_COMPARE_AND_SWAP_4, 
		  "__sync_bool_compare_and_swap_4",
		  build_function_type_list (boolean_type_node,
					    int_type_node, 
					    build_pointer_type (int_type_node),
					    int_type_node, NULL_TREE), 
		  "__sync_bool_compare_and_swap_4", ECF_NOTHROW | ECF_LEAF);
  define_builtin (BUILT_IN_SYNC_BOOL_COMPARE_AND_SWAP_8, 
		  "__sync_bool_compare_and_swap_8",
		  build_function_type_list (boolean_type_node,
					    long_type_node, 
					    build_pointer_type (long_type_node),
					    int_type_node, NULL_TREE), 
		  "__sync_bool_compare_and_swap_8", ECF_NOTHROW | ECF_LEAF);
  define_builtin (BUILT_IN_SYNC_SYNCHRONIZE, "__sync_synchronize",
		  build_function_type_list (void_type_node, NULL_TREE),
		  "__sync_synchronize", ECF_NOTHROW | ECF_LEAF);
  
  define_builtin (BUILT_IN_RETURN_ADDRESS, "__builtin_return_address",
		  build_function_type_list (ptr_type_node, int_type_node, NULL_TREE),
		  "__builtin_return_address", ECF_NOTHROW | ECF_LEAF);
  define_builtin (BUILT_IN_TRAP, "__builtin_trap",
		  build_function_type_list (void_type_node, NULL_TREE),
		  "__builtin_trap", ECF_NOTHROW | ECF_LEAF | ECF_NORETURN);
  build_common_builtin_nodes ();
}

/* If the call matches a builtin, return the
   appropriate builtin expression instead.  */
tree
check_for_builtin (tree method, tree call)
{
  if (optimize && TREE_CODE (call) == CALL_EXPR)
    {
      int i;
      tree method_class = DECL_NAME (TYPE_NAME (DECL_CONTEXT (method)));
      tree method_name = DECL_NAME (method);
      tree method_return_type = TREE_TYPE (TREE_TYPE (method));

      for (i = 0; java_builtins[i].builtin_code != END_BUILTINS; ++i)
	{
	  if (method_class == java_builtins[i].class_name.t
	      && method_name == java_builtins[i].method_name.t)
	    {
	      tree fn;

	      if (java_builtins[i].creator != NULL)
		{
		  tree result
		    = (*java_builtins[i].creator) (method_return_type, call);
		  return result == NULL_TREE ? call : result;
		}

	      /* Builtin functions emit a direct call which is incompatible
	         with the BC-ABI.  */
	      if (flag_indirect_dispatch)
	        return call;
	      fn = builtin_decl_explicit (java_builtins[i].builtin_code);
	      if (fn == NULL_TREE)
		return call;
	      return java_build_function_call_expr (fn, call);
	    }
	}
    }
  return call;
}

#include "gt-java-builtins.h"
