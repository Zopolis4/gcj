/* Handle exceptions for GNU compiler for the Java(TM) language.
   Copyright (C) 1997-2016 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.

Java and all Java-based marks are trademarks or registered trademarks
of Sun Microsystems, Inc. in the United States and other countries.
The Free Software Foundation is independent of Sun Microsystems, Inc.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "stringpool.h"
#include "diagnostic-core.h"
#include "fold-const.h"
#include "stor-layout.h"
#include "java-tree.h"
#include "java-except.h"
#include "toplev.h"
#include "tree-iterator.h"


static void expand_start_java_handler (struct eh_range *);
static struct eh_range *find_handler_in_range (int, struct eh_range *,
					       struct eh_range *);
static void check_start_handlers (struct eh_range *, int);
static void free_eh_ranges (struct eh_range *range);

struct eh_range *current_method_handlers;

struct eh_range *current_try_block = NULL;

/* These variables are used to speed up find_handler. */

static int cache_range_start, cache_range_end;
static struct eh_range *cache_range;
static struct eh_range *cache_next_child;

/* A dummy range that represents the entire method. */

struct eh_range whole_range;

/* Check the invariants of the structure we're using to contain
   exception regions.  Either returns true or fails an assertion
   check.  */

bool
sanity_check_exception_range (struct eh_range *range)
{
  struct eh_range *ptr = range->first_child;
  for (; ptr; ptr = ptr->next_sibling)
    {
      gcc_assert (ptr->outer == range
		  && ptr->end_pc > ptr->start_pc);
      if (ptr->next_sibling)
	gcc_assert (ptr->next_sibling->start_pc >= ptr->end_pc);
      gcc_assert (ptr->start_pc >= ptr->outer->start_pc
		  && ptr->end_pc <=  ptr->outer->end_pc);
      (void) sanity_check_exception_range (ptr);
    }
  return true;
}

#if defined(DEBUG_JAVA_BINDING_LEVELS)
extern int is_class_level;
extern int current_pc;
extern int binding_depth;
extern void indent (void);
static void
print_ranges (struct eh_range *range)
{
  if (! range)
    return;

  struct eh_range *child = range->first_child;
  
  indent ();
  fprintf (stderr, "handler pc %d --> %d ", range->start_pc, range->end_pc);
  
  tree handler = range->handlers;
  for ( ; handler != NULL_TREE; handler = TREE_CHAIN (handler))
    {
      tree type = TREE_PURPOSE (handler);
      if (type == NULL)
	type = throwable_type_node;
      fprintf (stderr, " type=%s ", IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (type))));
    }
  fprintf (stderr, "\n");

  int saved = binding_depth;
  binding_depth++;
  print_ranges (child);
  binding_depth = saved;

  print_ranges (range->next_sibling);
}
#endif

/* Search for the most specific eh_range containing PC.
   Assume PC is within RANGE.
   CHILD is a list of children of RANGE such that any
   previous children have end_pc values that are too low. */

static struct eh_range *
find_handler_in_range (int pc, struct eh_range *range, struct eh_range *child)
{
  for (; child != NULL;  child = child->next_sibling)
    {
      if (pc < child->start_pc)
	break;
      if (pc < child->end_pc)
	return find_handler_in_range (pc, child, child->first_child);
    }
  cache_range = range;
  cache_range_start = pc;
  cache_next_child = child;
  cache_range_end = child == NULL ? range->end_pc : child->start_pc;
  return range;
}

/* Find the inner-most handler that contains PC. */

struct eh_range *
find_handler (int pc)
{
  struct eh_range *h;
  if (pc >= cache_range_start)
    {
      h = cache_range;
      if (pc < cache_range_end)
	return h;
      while (pc >= h->end_pc)
	{
	  cache_next_child = h->next_sibling;
	  h = h->outer;
	}
    }
  else
    {
      h = &whole_range;
      cache_next_child = h->first_child;
    }
  return find_handler_in_range (pc, h, cache_next_child);
}

static void
free_eh_ranges (struct eh_range *range)
{
  while (range) 
    {
      struct eh_range *next = range->next_sibling;
      free_eh_ranges (range->first_child);
      if (range != &whole_range)
	free (range);
      range = next;
    }
}

/* Called to re-initialize the exception machinery for a new method. */

void
method_init_exceptions (void)
{
  free_eh_ranges (&whole_range);
  whole_range.start_pc = 0;
  whole_range.end_pc = DECL_CODE_LENGTH (current_function_decl) + 1;
  whole_range.outer = NULL;
  whole_range.first_child = NULL;
  whole_range.next_sibling = NULL;
  cache_range_start = 0xFFFFFF;
}

/* Split an exception range into two at PC.  The sub-ranges that
   belong to the range are split and distributed between the two new
   ranges.  */

static void
split_range (struct eh_range *range, int pc)
{
  struct eh_range *ptr;
  struct eh_range **first_child, **second_child;
  struct eh_range *h;

  /* First, split all the sub-ranges.  */
  for (ptr = range->first_child; ptr; ptr = ptr->next_sibling)
    {
      if (pc > ptr->start_pc
	  && pc < ptr->end_pc)
	{
	  split_range (ptr, pc);
	}
    }

  /* Create a new range.  */
  h = XNEW (struct eh_range);

  h->start_pc = pc;
  h->end_pc = range->end_pc;
  h->next_sibling = range->next_sibling;
  range->next_sibling = h;
  range->end_pc = pc;
  h->handlers = build_tree_list (TREE_PURPOSE (range->handlers),
				 TREE_VALUE (range->handlers));
  h->next_sibling = NULL;
  h->expanded = 0;
  h->stmt = NULL;
  h->outer = range->outer;
  h->first_child = NULL;

  ptr = range->first_child;
  first_child = &range->first_child;
  second_child = &h->first_child;

  /* Distribute the sub-ranges between the two new ranges.  */
  for (ptr = range->first_child; ptr; ptr = ptr->next_sibling)
    {
      if (ptr->start_pc < pc)
	{
	  *first_child = ptr;
	  ptr->outer = range;
	  first_child = &ptr->next_sibling;
	}
      else
	{
	  *second_child = ptr;
	  ptr->outer = h;
	  second_child = &ptr->next_sibling;
	}
    }
  *first_child = NULL;
  *second_child = NULL;
}  


/* Add an exception range. 

   There are some missed optimization opportunities here.  For
   example, some bytecode obfuscators generate seemingly
   nonoverlapping exception ranges which, when coalesced, do in fact
   nest correctly.  We could merge these, but we'd have to fix up all
   the enclosed regions first and perhaps create a new range anyway if
   it overlapped existing ranges.
   
   Also, we don't attempt to detect the case where two previously
   added disjoint ranges could be coalesced by a new range.  */

void 
add_handler (int start_pc, int end_pc, tree handler, tree type)
{
  struct eh_range *ptr, *h;
  struct eh_range **first_child, **prev;

  /* First, split all the existing ranges that we need to enclose.  */
  for (ptr = whole_range.first_child; ptr; ptr = ptr->next_sibling)
    {
      if (start_pc > ptr->start_pc
	  && start_pc < ptr->end_pc)
	{
	  split_range (ptr, start_pc);
	}

      if (end_pc > ptr->start_pc
	  && end_pc < ptr->end_pc)
	{
	  split_range (ptr, end_pc);
	}

      if (ptr->start_pc >= end_pc)
	break;
    }

  /* Create the new range.  */
  h = XNEW (struct eh_range);
  first_child = &h->first_child;

  h->start_pc = start_pc;
  h->end_pc = end_pc;
  h->first_child = NULL;
  h->outer = NULL_EH_RANGE;
  h->handlers = build_tree_list (type, handler);
  h->next_sibling = NULL;
  h->expanded = 0;
  h->stmt = NULL;

  /* Find every range at the top level that will be a sub-range of the
     range we're inserting and make it so.  */
  {
    struct eh_range **prev = &whole_range.first_child;
    for (ptr = *prev; ptr;)
      {
	struct eh_range *next = ptr->next_sibling;

	if (ptr->start_pc >= end_pc)
	  break;

	if (ptr->start_pc < start_pc)
	  {
	    prev = &ptr->next_sibling;
	  }
	else if (ptr->start_pc >= start_pc
		 && ptr->start_pc < end_pc)
	  {
	    *prev = next;
	    *first_child = ptr;
	    first_child = &ptr->next_sibling;
	    ptr->outer = h;
	    ptr->next_sibling = NULL;	  
	  }

	ptr = next;
      }
  }

  /* Find the right place to insert the new range.  */
  prev = &whole_range.first_child;
  for (ptr = *prev; ptr; prev = &ptr->next_sibling, ptr = ptr->next_sibling)
    {
      gcc_assert (ptr->outer == NULL_EH_RANGE);
      if (ptr->start_pc >= start_pc)
	break;
    }

  /* And insert it there.  */
  *prev = h;
  if (ptr)
    {
      h->next_sibling = ptr;
      h->outer = ptr->outer;
    }
}
      
  
/* if there are any handlers for this range, issue start of region */
static void
expand_start_java_handler (struct eh_range *range)
{
#if defined(DEBUG_JAVA_BINDING_LEVELS)
  indent ();
  fprintf (stderr, "expand start handler pc %d --> %d\n",
	   current_pc, range->end_pc);
#endif /* defined(DEBUG_JAVA_BINDING_LEVELS) */
  pushlevel (0);
  register_exception_range (range,  range->start_pc, range->end_pc);
  range->expanded = 1;
}

tree
prepare_eh_table_type (tree type)
{
  tree exp;
  tree *slot;
  const char *name;
  char *buf;
  tree decl;
  tree utf8_ref;

  /* The "type" (match_info) in a (Java) exception table is a pointer to:
   * a) NULL - meaning match any type in a try-finally.
   * b) a pointer to a pointer to a class.
   * c) a pointer to a pointer to a utf8_ref.  The pointer is
   * rewritten to point to the appropriate class.  */

  if (type == NULL_TREE)
    return NULL_TREE;

  if (TYPE_TO_RUNTIME_MAP (output_class) == NULL)
    TYPE_TO_RUNTIME_MAP (output_class) = java_treetreehash_create (10);
  
  slot = java_treetreehash_new (TYPE_TO_RUNTIME_MAP (output_class), type);
  if (*slot != NULL)
    return TREE_VALUE (*slot);

  if (is_compiled_class (type) && !flag_indirect_dispatch)
    {
      name = IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (type)));
      buf = (char *) alloca (strlen (name) + 5);
      sprintf (buf, "%s_ref", name);
      decl = build_decl (input_location,
			 VAR_DECL, get_identifier (buf), ptr_type_node);
      TREE_STATIC (decl) = 1;
      DECL_ARTIFICIAL (decl) = 1;
      DECL_IGNORED_P (decl) = 1;
      TREE_READONLY (decl) = 1;
      TREE_THIS_VOLATILE (decl) = 0;
      DECL_INITIAL (decl) = build_class_ref (type);
      layout_decl (decl, 0);
      pushdecl (decl);
      exp = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (decl)), decl);
    }
  else
    {
      utf8_ref = build_utf8_ref (DECL_NAME (TYPE_NAME (type)));
      name = IDENTIFIER_POINTER (DECL_NAME (TREE_OPERAND (utf8_ref, 0)));
      buf = (char *) alloca (strlen (name) + 5);
      sprintf (buf, "%s_ref", name);
      decl = build_decl (input_location,
			 VAR_DECL, get_identifier (buf), utf8const_ptr_type);
      TREE_STATIC (decl) = 1;
      DECL_ARTIFICIAL (decl) = 1;
      DECL_IGNORED_P (decl) = 1;
      TREE_READONLY (decl) = 1;
      TREE_THIS_VOLATILE (decl) = 0;
      layout_decl (decl, 0);
      pushdecl (decl);
      exp = build1 (ADDR_EXPR, build_pointer_type (utf8const_ptr_type), decl);
      CONSTRUCTOR_APPEND_ELT (TYPE_CATCH_CLASSES (output_class),
			      NULL_TREE,
			      make_catch_class_record (exp, utf8_ref));
    }

  exp = convert (ptr_type_node, exp);

  *slot = tree_cons (type, exp, NULL_TREE);

  return exp;
}

int
expand_catch_class (treetreehash_entry **entry, int)
{
  struct treetreehash_entry *ite = *entry;
  tree addr = TREE_VALUE ((tree)ite->value);
  tree decl;
  STRIP_NOPS (addr);
  decl = TREE_OPERAND (addr, 0);
  rest_of_decl_compilation (decl, global_bindings_p (), 0);
  return true;
}
  
/* For every class in the TYPE_TO_RUNTIME_MAP, expand the
   corresponding object that is used by the runtime type matcher.  */

void
java_expand_catch_classes (tree this_class)
{
  if (TYPE_TO_RUNTIME_MAP (this_class))
    TYPE_TO_RUNTIME_MAP (this_class)->traverse<int, expand_catch_class> (0);
}

/* Build and push the variable that will hold the exception object
   within this function.  */

static tree
build_exception_object_var (void)
{
  tree decl = DECL_FUNCTION_EXC_OBJ (current_function_decl);
  if (decl == NULL)
    {
      decl = build_decl (DECL_SOURCE_LOCATION (current_function_decl),
			 VAR_DECL, get_identifier ("#exc_obj"), ptr_type_node);
      DECL_IGNORED_P (decl) = 1;
      DECL_ARTIFICIAL (decl) = 1;

      DECL_FUNCTION_EXC_OBJ (current_function_decl) = decl;
      pushdecl_function_level (decl);
    }
  return decl;
}

/* Build a reference to the jthrowable object being carried in the
   exception header.  */

tree
build_exception_object_ref (tree type)
{
  tree obj;

  /* Java only passes object via pointer and doesn't require adjusting.
     The java object is immediately before the generic exception header.  */
  obj = build_exception_object_var ();
  obj = fold_convert (build_pointer_type (type), obj);
  obj = fold_build_pointer_plus (obj,
		fold_build1 (NEGATE_EXPR, sizetype,
			     TYPE_SIZE_UNIT (TREE_TYPE (obj))));
  obj = build1 (INDIRECT_REF, type, obj);

  return obj;
}

/* If there are any handlers for this range, issue end of range,
   and then all handler blocks */
void
expand_end_java_handler (struct eh_range *range)
{  
  tree handler = range->handlers;
  if (handler)
    {
      tree exc_obj = build_exception_object_var ();
      tree catches = make_node (STATEMENT_LIST);
      tree_stmt_iterator catches_i = tsi_last (catches);
      tree *body;

      for (; handler; handler = TREE_CHAIN (handler))
	{
	  tree type, eh_type, x;
	  tree stmts = make_node (STATEMENT_LIST);
	  tree_stmt_iterator stmts_i = tsi_last (stmts);

	  type = TREE_PURPOSE (handler);
	  if (type == NULL)
	    type = throwable_type_node;
	  eh_type = prepare_eh_table_type (type);

	  x = build_call_expr (builtin_decl_explicit (BUILT_IN_EH_POINTER),
			       1, integer_zero_node);
	  x = build2 (MODIFY_EXPR, void_type_node, exc_obj, x);
	  tsi_link_after (&stmts_i, x, TSI_CONTINUE_LINKING);

	  x = build1 (GOTO_EXPR, void_type_node, TREE_VALUE (handler));
	  tsi_link_after (&stmts_i, x, TSI_CONTINUE_LINKING);

	  x = build2 (CATCH_EXPR, void_type_node, eh_type, stmts);
	  tsi_link_after (&catches_i, x, TSI_CONTINUE_LINKING);

	  /* Throwable can match anything in Java, and therefore
	     any subsequent handlers are unreachable.  */
	  /* ??? If we're assured of no foreign language exceptions,
	     we'd be better off using NULL as the exception type
	     for the catch.  */
	  if (type == throwable_type_node)
	    break;
	}

      body = get_stmts ();
      *body = build2 (TRY_CATCH_EXPR, void_type_node, *body, catches);
    }

#if defined(DEBUG_JAVA_BINDING_LEVELS)
  indent ();
  fprintf (stderr, "expand end handler pc %d <-- %d\n",
	   current_pc, range->start_pc);
#endif /* defined(DEBUG_JAVA_BINDING_LEVELS) */
}

/* Recursive helper routine for maybe_start_handlers. */

static void
check_start_handlers (struct eh_range *range, int pc)
{
  if (range != NULL_EH_RANGE && range->start_pc == pc)
    {
      check_start_handlers (range->outer, pc);
      if (!range->expanded)
	expand_start_java_handler (range);
    }
}


/* Routine to see if exception handling is turned on.
   DO_WARN is nonzero if we want to inform the user that exception
   handling is turned off.

   This is used to ensure that -fexceptions has been specified if the
   compiler tries to use any exception-specific functions.  */

static inline int
doing_eh (void)
{
  if (! flag_exceptions)
    {
      static int warned = 0;
      if (! warned)
	{
	  error ("exception handling disabled, use -fexceptions to enable");
	  warned = 1;
	}
      return 0;
    }
  return 1;
}

static struct eh_range *current_range;

/* Emit any start-of-try-range starting at start_pc and ending after
   end_pc. */

void
maybe_start_try (int start_pc, int end_pc)
{
  struct eh_range *range;
  if (! doing_eh ())
    return;

  range = find_handler (start_pc);
  while (range != NULL_EH_RANGE && range->start_pc == start_pc
	 && range->end_pc < end_pc)
    range = range->outer;
	 
  current_range = range;
  check_start_handlers (range, start_pc);
}

