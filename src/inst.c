#ident "@(#) $Id: inst.c,v 1.1 2001/01/26 07:11:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: inst.c                                                        */
/*   Name....: Instruction Functions                                         */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"

#ifndef NDEBUG
#define Trace(fname) if (verbose) fprintf(stderr, "Eval: %s\n", fname);
#else
#define Trace(fname) /* */
#endif

/* ----------------------------------------------------------------------------
 * Kontrollfluss Funktionen
 * ----------------------------------------------------------------------------
 */
void i_nop(CodeNode* self)
{
   Trace("i_nop");

   assert(code_is_valid(self));
}

void i_once(CodeNode* self)
{
   const char* name;
   Ineq*       ineq;

   Trace("i_once");
   
   assert(code_is_valid(self));

   name = code_get_name(code_get_child(self, 0));
   ineq = code_get_ineq(code_get_child(self, 1));

   printf("%s: ", name);
   ineq_print(stdout, ineq);
   printf("\n");

   ineq_free(ineq);
   
   code_value_void(self);
}

void i_forall(CodeNode* self)
{
   const char* name;
   IdxSet* idxset;
   Ineq*   ineq;
   Set*    set;
   Tuple*  pattern;
   Tuple*  tuple;
   int     idx = 0;

   Trace("i_forall");
   
   assert(code_is_valid(self));

   name    = code_get_name(code_get_child(self, 0));
   idxset  = code_get_idxset(code_get_child(self, 1));
   set     = idxset_get_set(idxset, 0);
   pattern = idxset_get_tuple(idxset, 0);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      if (pattern != NULL)
         local_install_tuple(pattern, tuple);

      ineq = code_get_ineq(code_get_child(self, 2));

      printf("%s_%d: ", name, idx);
      ineq_print(stdout, ineq);
      printf("\n");

      if (pattern != NULL)
         local_drop_frame();

      ineq_free(ineq);
      tuple_free(tuple);
   }
   set_free(set);
   tuple_free(pattern);
   idxset_free(idxset);
   
   code_value_void(self);
}

/* ----------------------------------------------------------------------------
 * Arithmetische Funktionen
 * ----------------------------------------------------------------------------
 */
void i_expr_add(CodeNode* self)
{
   Trace("i_add");

   assert(code_is_valid(self));

   code_value_numb(self, code_get_numb(code_get_child(self, 0))
      + code_get_numb(code_get_child(self, 1)));
}

void i_expr_sub(CodeNode* self)
{
   Trace("i_sub");

   assert(code_is_valid(self));

   code_value_numb(self, code_get_numb(code_get_child(self, 0))
      - code_get_numb(code_get_child(self, 1)));
}

void i_expr_mul(CodeNode* self)
{
   Trace("i_mul");

   assert(code_is_valid(self));

   code_value_numb(self, code_get_numb(code_get_child(self, 0))
      * code_get_numb(code_get_child(self, 1)));
}

void i_expr_div(CodeNode* self)
{
   Trace("i_div");

   assert(code_is_valid(self));

   code_value_numb(self, code_get_numb(code_get_child(self, 0))
      / code_get_numb(code_get_child(self, 1)));
}

/* ----------------------------------------------------------------------------
 * Logische Funktionen
 * ----------------------------------------------------------------------------
 */
void i_true(CodeNode* self)
{
   Trace("i_true");

   assert(code_is_valid(self));
   
   code_value_bool(self, TRUE);
}

void i_false(CodeNode* self)
{
   Trace("i_false");

   assert(code_is_valid(self));
   
   code_value_bool(self, FALSE);
}

void i_not(CodeNode* self)
{
   Trace("i_not");

   assert(code_is_valid(self));
   
   code_value_bool(self, !code_get_bool(code_get_child(self, 0)));
}

void i_and(CodeNode* self)
{
   Trace("i_and");

   assert(code_is_valid(self));
   
   code_value_bool(self, code_get_bool(code_get_child(self, 0))
      && code_get_bool(code_get_child(self, 0)));
}

void i_or(CodeNode* self)
{
   Trace("i_or");

   assert(code_is_valid(self));
   
   code_value_bool(self, code_get_bool(code_get_child(self, 0))
      || code_get_bool(code_get_child(self, 0)));
}

void i_eq(CodeNode* self)
{
   Trace("i_eq");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      EQ(code_get_numb(code_get_child(self, 0)),
         code_get_numb(code_get_child(self, 0))));
}

/* ----------------------------------------------------------------------------
 * Set Funktionen
 * ----------------------------------------------------------------------------
 */
void i_set_new(CodeNode* self)
{
   List*     list  = code_get_list(code_get_child(self, 0));
   ListElem* le    = NULL;
   Tuple*    tuple = list_get_tuple(list, &le);
   int       dim   = tuple_get_dim(tuple);
   Set*      set   = set_new(dim);
   int       n     = list_get_elems(list);
   int       i;
   
   Trace("i_set_new");

   assert(code_is_valid(self));

   tuple_free(tuple);

   /* Und jetzt noch mal alle.
    */
   le  = NULL;

   for(i = 0; i < n; i++)
   {
      tuple = list_get_tuple(list, &le);
      set_add_member(set, tuple, SET_ADD_END);
      tuple_free(tuple);
   }
   code_value_set(self, set);

   set_free(set);
   list_free(list);
}

void i_set_empty(CodeNode* self)
{
   Set*      set;
   Tuple*    tuple;
   int       dim;
   
   Trace("i_set_empty");

   assert(code_is_valid(self));

   dim = code_get_size(code_get_child(self, 0));
   set = set_new(dim);

   if (dim == 0)
   {
      tuple = tuple_new(0);
      set_add_member(set, tuple, SET_ADD_END);
      tuple_free(tuple);
   }
   code_value_set(self, set);

   set_free(set);
}

void i_set_dim(CodeNode* self)
{
   Set* set;

   Trace("i_set_dim");

   assert(code_is_valid(self));
   
   set = code_get_set(code_get_child(self, 0));

   code_value_size(self, set_get_dim(set));

   set_free(set);
}

void i_set_union(CodeNode* self)
{
   Set* set_a;
   Set* set_b;
   Set* set_r;
   
   Trace("i_set_union");

   assert(code_is_valid(self));

   set_a = code_get_set(code_get_child(self, 0));
   set_b = code_get_set(code_get_child(self, 1));

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "Error: unkompatible sets, lineno=%d, column=%d\n",
         code_get_lineno(self), code_get_column(self));      
      assert(0);
   }
   set_r = set_union(set_a, set_b);
   
   code_value_set(self, set_r);

   set_free(set_a);
   set_free(set_b);
   set_free(set_r);
}

void i_set_cross(CodeNode* self)
{
   Set* set_a;
   Set* set_b;
   Set* set_r;
   
   Trace("i_set_cross");

   assert(code_is_valid(self));

   set_a = code_get_set(code_get_child(self, 0));
   set_b = code_get_set(code_get_child(self, 1));
   set_r = set_cross(set_a, set_b);

   code_value_set(self, set_r);

   set_free(set_a);
   set_free(set_b);
   set_free(set_r);
}

/* ----------------------------------------------------------------------------
 * Tupel Funktionen
 * ----------------------------------------------------------------------------
 */
void i_tuple_new(CodeNode* self)
{
   List*      list  = code_get_list(code_get_child(self, 0));
   int        n     = list_get_elems(list);
   Tuple*     tuple = tuple_new(n);
   ListElem*  le    = NULL;
   int        i;

   Trace("i_tuple_new");
   
   assert(code_is_valid(self));
   
   for(i = 0; i < n; i++)
      tuple_set_elem(tuple, i, list_get_elem(list, &le));
   
   code_value_tuple(self, tuple);

   list_free(list);
   tuple_free(tuple);
}

void i_tuple_empty(CodeNode* self)
{
   Tuple* tuple;
   
   assert(code_is_valid(self));

   tuple = tuple_new(0);
   code_value_tuple(self, tuple);
   tuple_free(tuple);
}

/* ----------------------------------------------------------------------------
 * Symbol Funktionen
 * ----------------------------------------------------------------------------
 */
void i_newsym_set(CodeNode* self)
{
   const char*  name  = code_get_name(code_get_child(self, 0));
   Set*         iset  = code_get_set(code_get_child(self, 1));
   Symbol*      sym   = symbol_new(name, SYM_SET, iset);
   Tuple*       tuple = tuple_new(0);   
   Set*         set   = code_get_set(code_get_child(self, 2));
   Entry*       entry = entry_new_set(tuple, set);

   Trace("i_newsym_set");
   
   assert(code_is_valid(self));

   symbol_add_entry(sym, entry);

   set_free(iset);
   tuple_free(tuple);
   set_free(set);
   entry_free(entry);
   
   code_value_void(self);
}

void i_newsym_numb(CodeNode* self)
{
   const char*  name;
   Set*         set;
   Symbol*      sym;
   List*        list;
   ListElem*    lelem;
   Entry*       entry;
   Tuple*       tuple;
   int          count;
   int          i;
   
   Trace("i_newsym_numb");

   assert(code_is_valid(self));

   name = code_get_name(code_get_child(self, 0));
   set  = code_get_set(code_get_child(self, 1));
   list = code_get_list(code_get_child(self, 2));
   sym  = symbol_new(name, SYM_NUMB, set);
   
   lelem = NULL;
   count = list_get_elems(list);
   
   for(i = 0; i < count; i++)
   {
      entry  = list_get_entry(list, &lelem);
      tuple  = entry_get_tuple(entry);
      
      if (set_lookup(set, tuple))
         symbol_add_entry(sym, entry);
      else
      {
         fprintf(stderr, "Error: Illegal element ");
         tuple_print(stderr, tuple);
         fprintf(stderr, "\nline=%d col=%d",
            code_get_lineno(self), code_get_column(self));
      }
      tuple_free(tuple);
      entry_free(entry);
   }
   set_free(set);
   list_free(list);

   code_value_void(self);
}

void i_newsym_var(CodeNode* self)
{
   const char*  name;
   Set*         set;
   Symbol*      sym;
   Tuple*       tuple;
   VarType      vartype;
   Var*         var;
   Entry*       entry;
   int          idx = 0;
   double       lower = 0.0;
   double       upper = 1000.0;
   
   Trace("i_newsym_var");

   assert(code_is_valid(self));

   name    = code_get_name(code_get_child(self, 0));
   set     = code_get_set(code_get_child(self, 1));
   vartype = code_get_vartype(code_get_child(self, 2)); 
   sym     = symbol_new(name, SYM_VAR, set);

   if (vartype == VAR_BIN)
      upper = 1.0;
      
   while((tuple = set_match_next(set, NULL, &idx)) != NULL)
   {
      var   = var_new(vartype, lower, upper);
      entry = entry_new_var(tuple, var);

      symbol_add_entry(sym, entry);

      entry_free(entry);
      var_free(var);
      tuple_free(tuple);
   }
   set_free(set);
   
   code_value_void(self);
}

void i_symbol_deref(CodeNode* self)
{
   const char* name;
   Tuple*      tuple;
   Symbol*     sym;
   Set*        set;   
   int         idx;
   Term*       term;
   
   Trace("i_symbol_deref");
   
   assert(code_is_valid(self));

   name  = code_get_name(code_get_child(self, 0));
   tuple = code_get_tuple(code_get_child(self, 1));   
   sym   = symbol_lookup(name);

   if (sym == NULL)
   {
      fprintf(stderr, "Error: unknown symbol \"%s\", lineno=%d, column=%d\n",
         name, code_get_lineno(self), code_get_column(self));      
      assert(0);
   }
   idx = symbol_lookup_tuple(sym, tuple);

   if (idx < 0)
   {
      fprintf(stderr, "Error: Unknown index ");
      tuple_print(stderr, tuple);
      fprintf(stderr, " for symbol \"%s\" at lineno=%d, column=%d\n",
         name, code_get_lineno(self), code_get_column(self));
      assert(0);
   }
   tuple_free(tuple);

   switch(symbol_get_type(sym))
   {
   case SYM_NUMB :
      code_value_numb(self, symbol_get_numb(sym, idx));
      break;
   case SYM_STRG :
      code_value_strg(self, symbol_get_strg(sym, idx));
      break;
   case SYM_SET :
      set = symbol_get_set(sym, idx);
      code_value_set(self, set);
      set_free(set);
      break;
   case SYM_VAR :
      term = term_new();
      term_add_elem(term, sym, idx, 1.0);
      code_value_term(self, term);
      term_free(term);
      break;
   default :
      assert(0);
   }
}

/* ----------------------------------------------------------------------------
 * Index Set Funktionen
 * ----------------------------------------------------------------------------
 */
void i_idxset_new(CodeNode* self)
{
   IdxSet*   idxset;
   Tuple*    tuple;
   Set*      set;
   CodeNode* child;
   
   Trace("i_idxset_new");
   
   assert(code_is_valid(self));

   child  = code_get_child(self, 0);
   tuple  = (child == NULL) ? NULL : code_get_tuple(child);
   set    = code_get_set(code_get_child(self, 1));
   idxset = idxset_new();

   idxset_add_set(idxset, tuple, set);

   code_value_idxset(self, idxset);

   if (tuple != NULL)
      tuple_free(tuple);
   
   set_free(set);
   idxset_free(idxset);
}

/* ----------------------------------------------------------------------------
 * Local Funktionen
 * ----------------------------------------------------------------------------
 */
void i_local_deref(CodeNode* self)
{
   const char* name;
   const Elem* elem;
   
   Trace("i_local");
   
   assert(code_is_valid(self));

   name = code_get_name(code_get_child(self, 0));   
   elem = local_lookup(name);

   if (elem == NULL)
      code_value_name(self, name);
   else
   {
      switch(elem_get_type(elem))
      {
      case ELEM_NUMB :
         code_value_numb(self, elem_get_numb(elem));
         break;
      case ELEM_STRG :
         code_value_strg(self, elem_get_strg(elem));
         break;
      default :
         assert(0);
      }
   }
}   

/* ----------------------------------------------------------------------------
 * Term Funktionen
 * ----------------------------------------------------------------------------
 */
void i_term_coeff(CodeNode* self)
{
   Term*  term;
   double coeff;
   
   Trace("i_term_coeff");
   
   assert(code_is_valid(self));

   term  = code_get_term(code_get_child(self, 0));
   coeff = code_get_numb(code_get_child(self, 1));

   term_mul_coeff(term, coeff);
   
   code_value_term(self, term);

   term_free(term);
}

void i_term_add(CodeNode* self)
{
   Term* term_a;
   Term* term_b;
   Term* term_r;

   Trace("i_term_add");
   
   assert(code_is_valid(self));

   term_a = code_get_term(code_get_child(self, 0));
   term_b = code_get_term(code_get_child(self, 1));
   term_r = term_add_term(term_a, term_b);
   
   code_value_term(self, term_r);

   term_free(term_a);
   term_free(term_b);
   term_free(term_r);
}

void i_term_sub(CodeNode* self)
{
   Term* term_a;
   Term* term_b;
   Term* term_r;
   
   Trace("i_term_sub");
   
   assert(code_is_valid(self));

   term_a = code_get_term(code_get_child(self, 0));
   term_b = code_get_term(code_get_child(self, 1));
   term_negate(term_b);
   term_r = term_add_term(term_a, term_b);
   
   code_value_term(self, term_r);

   term_free(term_a);
   term_free(term_b);
   term_free(term_r);
}

/* Eventuell kann man unter einsatz der i_term Funktion
 * diese Funktion wesendlich vereinfachen.
 */
void i_term_sum(CodeNode* self)
{
   IdxSet*  idxset;
   Term*    term_a;
   Term*    term_b;
   Term*    term_r = NULL;
   Set*     set;
   Tuple*   pattern;
   Tuple*   tuple;
   int      idx = 0;

   Trace("i_term_sum");
   
   assert(code_is_valid(self));

   idxset  = code_get_idxset(code_get_child(self, 0));
   set     = idxset_get_set(idxset, 0);
   pattern = idxset_get_tuple(idxset, 0);
   term_a  = term_new();

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      if (pattern != NULL)
         local_install_tuple(pattern, tuple);

      term_b = code_get_term(code_get_child(self, 1));      
      term_r = term_add_term(term_a, term_b);

      term_free(term_a);
      term_free(term_b);
      term_a = term_r;
      
      if (pattern != NULL)
         local_drop_frame();

      tuple_free(tuple);
   }
   if (pattern != NULL)
      tuple_free(pattern);

   if (term_r == NULL)
   {
      fprintf(stderr, "Error: Empty Term generated, lineno=%d, column=%d\n",
         code_get_lineno(self), code_get_column(self));      
      assert(0);
   }
   code_value_term(self, term_r);

   set_free(set);   
   term_free(term_r);
   idxset_free(idxset);
}

/* ----------------------------------------------------------------------------
 * Inequality Funktionen
 * ----------------------------------------------------------------------------
 */
void i_ineq_new(CodeNode* self)
{
   Term*    term;
   Ineq*    ineq;
   IneqType type;
   double   rhs;
   
   Trace("i_ineq_new");
   
   assert(code_is_valid(self));

   term = code_get_term(code_get_child(self, 0));
   type = code_get_ineqtype(code_get_child(self, 1));
   rhs  = code_get_numb(code_get_child(self, 2));
   rhs -= term_get_constant(term);
   
   if (term == NULL)
   {
      fprintf(stderr, "Error: Empty term\n");
      assert(0);
   }
   ineq = ineq_new(type, term, rhs);

   code_value_ineq(self, ineq);

   ineq_free(ineq);
   term_free(term);
}

/* ----------------------------------------------------------------------------
 * Size Funktionen
 * ----------------------------------------------------------------------------
 */
void i_numb_size(CodeNode* self)
{
   int    size;
   
   Trace("i_entry");
   
   assert(code_is_valid(self));

   size = (int)code_get_numb(code_get_child(self, 0));

   assert(size >= 0);

   code_value_size(self, size);
}

/* ----------------------------------------------------------------------------
 * Entry Funktionen
 * ----------------------------------------------------------------------------
 */
void i_entry(CodeNode* self)
{
   Tuple*     tuple;
   Entry*     entry;
   CodeNode*  child;
   CodeType   type;
   
   Trace("i_entry");
   
   assert(code_is_valid(self));

   tuple = code_get_tuple(code_get_child(self, 0));
   child = code_get_child(self, 1);
   type  = code_get_type(child);

   switch(type)
   {
   case CODE_NUMB :
      entry = entry_new_numb(tuple, code_get_numb(child));
      break;
   case CODE_STRG :
      entry = entry_new_strg(tuple, code_get_strg(child));
      break;
   default :
      assert(0);
   }
   code_value_entry(self, entry);

   tuple_free(tuple);
   entry_free(entry);
}

/* ----------------------------------------------------------------------------
 * List Funktionen
 * ----------------------------------------------------------------------------
 */
void i_elem_list_new(CodeNode* self)
{
   CodeNode*   child = code_get_child(self, 0);
   const Elem* elem;
   List*       list;
   
   Trace("i_elem_list_new");

   assert(code_is_valid(self));

   switch(code_get_type(child))
   {
   case CODE_NUMB :
      elem = elem_new_numb(code_get_numb(child));
      break;
   case CODE_STRG :
      elem = elem_new_strg(code_get_strg(child));
      break;
   case CODE_NAME :
      elem = elem_new_name(code_get_name(child));
      break;
   default :
      assert(0);
   }
   list = list_new_elem(elem);
   
   code_value_list(self, list);

   list_free(list);
}

void i_elem_list_add(CodeNode* self)
{
   CodeNode*   child;
   const Elem* elem;
   List*       list;
   
   Trace("i_elem_list_add");

   assert(code_is_valid(self));

   list  = code_get_list(code_get_child(self, 0));
   child = code_get_child(self, 1);

   switch(code_get_type(child))
   {
   case CODE_NUMB :
      elem = elem_new_numb(code_get_numb(child));
      break;
   case CODE_STRG :
      elem = elem_new_strg(code_get_strg(child));
      break;
   case CODE_NAME :
      elem = elem_new_name(code_get_name(child));
      break;
   default :
      assert(0);
   }
   list_add_elem(list, elem);   
   code_value_list(self, list);

   list_free(list);
}

void i_tuple_list_new(CodeNode* self)
{
   Tuple* tuple = code_get_tuple(code_get_child(self, 0));
   List*  list  = list_new_tuple(tuple);
   
   Trace("i_tuple_list_new");

   assert(code_is_valid(self));

   code_value_list(self, list);

   tuple_free(tuple);
   list_free(list);
}

void i_tuple_list_add(CodeNode* self)
{
   Tuple* tuple;
   List*  list;
   
   Trace("i_tuple_list_add");

   assert(code_is_valid(self));

   list  = code_get_list(code_get_child(self, 0));
   tuple = code_get_tuple(code_get_child(self, 1));
   
   list_add_tuple(list, tuple);   

   code_value_list(self, list);

   tuple_free(tuple);
   list_free(list);
}

void i_entry_list_new(CodeNode* self)
{
   Entry* entry;
   List*  list;
   
   Trace("i_entry_list_new");

   assert(code_is_valid(self));

   entry = code_get_entry(code_get_child(self, 0));
   list  = list_new_entry(entry);
   
   code_value_list(self, list);

   entry_free(entry);
   list_free(list);
}

void i_entry_list_add(CodeNode* self)
{
   Entry* entry;
   List*  list;
   
   Trace("i_entry_list_add");

   assert(code_is_valid(self));

   list  = code_get_list(code_get_child(self, 0));
   entry = code_get_entry(code_get_child(self, 1));

   list_add_entry(list, entry);   
   code_value_list(self, list);
   
   entry_free(entry);
   list_free(list);
}

void i_object_min(CodeNode* self)
{
   const char* name;
   Term*       term;
   
   Trace("i_object_min");

   assert(code_is_valid(self));

   name = code_get_name(code_get_child(self, 0));
   term = code_get_term(code_get_child(self, 1));

   printf("Minimize\n");
   printf("%s: ", name);
   term_print(stdout, term);
   printf("\nSubject To\n");

   term_free(term);
   
   code_value_void(self);
}







