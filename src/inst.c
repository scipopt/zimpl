#ident "@(#) $Id: inst.c,v 1.12 2002/05/26 12:44:57 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: inst.c                                                        */
/*   Name....: Instructions                                                  */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001 by Thorsten Koch <koch@zib.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"
#include "code.h"
#include "inst.h"

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
   const char*  name;
   Term*        term;
   ConType      type;
   double       rhs;
   Con*         con;
   unsigned int flags;
   
   Trace("i_once");
   
   assert(code_is_valid(self));

   name  = Code_eval_child_name(self, 0);
   term  = Code_eval_child_term(self, 1);
   type  = Code_eval_child_contype(self, 2);
   rhs   = Code_eval_child_numb(self, 3);
   flags = Code_eval_child_bits(self, 4);
   con   = lps_addcon(name, type, rhs, flags);

   term_to_nzo(term, con);
   term_free(term);   
   
   code_value_void(self);
}

void i_forall(CodeNode* self)
{
   const char*  name;
   char*        conname;
   Con*         con;
   IdxSet*      idxset;
   Term*        term;
   ConType      type;
   double       rhs;
   unsigned int flags;
   Set*         set;
   Tuple*       pattern;
   Tuple*       tuple;
   CodeNode*    lexpr;
   int          idx   = 0;
   int          count = 0;
   
   Trace("i_forall");
   
   assert(code_is_valid(self));

   name    = Code_eval_child_name(self, 0);
   idxset  = Code_eval_child_idxset(self, 1);
   type    = Code_eval_child_contype(self, 3);
   flags   = Code_eval_child_bits(self, 5);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   
   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         conname = malloc(strlen(name) + 13);
         assert(conname != NULL);
         sprintf(conname, "%s_%d", name, ++count);
         
         term = Code_eval_child_term(self, 2);
         rhs  = Code_eval_child_numb(self, 4);         
         con  = lps_addcon(conname, type, rhs, flags);

         term_to_nzo(term, con);

         term_free(term);   
         free(conname);
      }
      local_drop_frame();

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

   code_value_numb(self,
      Code_eval_child_numb(self, 0) + Code_eval_child_numb(self, 1));
}

void i_expr_sub(CodeNode* self)
{
   Trace("i_sub");

   assert(code_is_valid(self));

   code_value_numb(self,
      Code_eval_child_numb(self, 0) - Code_eval_child_numb(self, 1));
}

void i_expr_mul(CodeNode* self)
{
   Trace("i_mul");

   assert(code_is_valid(self));

   code_value_numb(self, 
      Code_eval_child_numb(self, 0) * Code_eval_child_numb(self, 1));
}

void i_expr_div(CodeNode* self)
{
   Trace("i_div");

   assert(code_is_valid(self));

   code_value_numb(self,
      Code_eval_child_numb(self, 0) / Code_eval_child_numb(self, 1));
}

void i_expr_mod(CodeNode* self)
{
   Trace("i_mod");

   assert(code_is_valid(self));

   code_value_numb(self,
      fmod(Code_eval_child_numb(self, 0), Code_eval_child_numb(self, 1)));
}

void i_expr_intdiv(CodeNode* self)
{
   Trace("i_intdiv");

   assert(code_is_valid(self));

   code_value_numb(self,
      floor(Code_eval_child_numb(self, 0) / Code_eval_child_numb(self, 1)));
}

void i_expr_pow(CodeNode* self)
{
   Trace("i_pow");

   assert(code_is_valid(self));

   code_value_numb(self,
      pow(Code_eval_child_numb(self, 0), Code_eval_child_numb(self, 1)));
}

void i_expr_neg(CodeNode* self)
{
   Trace("i_neg");

   assert(code_is_valid(self));

   code_value_numb(self, -Code_eval_child_numb(self, 0));
}

void i_expr_if(CodeNode* self)
{
   Trace("i_if");

   assert(code_is_valid(self));

   if (Code_eval_child_bool(self, 0))
      code_value_numb(self, Code_eval_child_numb(self, 1));
   else
      code_value_numb(self, Code_eval_child_numb(self, 2));
}

void i_expr_min(CodeNode* self)
{
   IdxSet*   idxset;
   Set*      set;
   Tuple*    pattern;
   Tuple*    tuple;
   CodeNode* lexpr;
   int       idx = 0;
   double    value;
   double    min = DBL_MAX;

   Trace("i_expr_min");
   
   assert(code_is_valid(self));

   idxset  = Code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         value = Code_eval_child_numb(self, 1);      

         if (value < min)
            min = value;
      }
      local_drop_frame();

      tuple_free(tuple);
   }
   tuple_free(pattern);

   code_value_numb(self, min);

   set_free(set);   
   idxset_free(idxset);
}

void i_expr_max(CodeNode* self)
{
   IdxSet*   idxset;
   Set*      set;
   Tuple*    pattern;
   Tuple*    tuple;
   CodeNode* lexpr;
   int       idx = 0;
   double    value;
   double    max = -DBL_MAX;

   Trace("i_expr_max");
   
   assert(code_is_valid(self));

   idxset  = Code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         value = Code_eval_child_numb(self, 1);      

         if (value > max)
            max = value;
      }
      local_drop_frame();

      tuple_free(tuple);
   }
   tuple_free(pattern);

   code_value_numb(self, max);

   set_free(set);   
   idxset_free(idxset);
}

void i_expr_sum(CodeNode* self)
{
   IdxSet*   idxset;
   Set*      set;
   Tuple*    pattern;
   Tuple*    tuple;
   CodeNode* lexpr;
   int       idx = 0;
   double    sum = 0;

   Trace("i_expr_sum");
   
   assert(code_is_valid(self));

   idxset  = Code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
         sum += Code_eval_child_numb(self, 1);      

      local_drop_frame();

      tuple_free(tuple);
   }
   tuple_free(pattern);

   code_value_numb(self, sum);

   set_free(set);   
   idxset_free(idxset);
}

/* ----------------------------------------------------------------------------
 * Logische Funktionen
 * ----------------------------------------------------------------------------
 */
void i_bool_true(CodeNode* self)
{
   Trace("i_bool_true");

   assert(code_is_valid(self));
   
   code_value_bool(self, TRUE);
}

void i_bool_false(CodeNode* self)
{
   Trace("i_bool_false");

   assert(code_is_valid(self));
   
   code_value_bool(self, FALSE);
}

void i_bool_not(CodeNode* self)
{
   Trace("i_bool_not");

   assert(code_is_valid(self));
   
   code_value_bool(self, !Code_eval_child_bool(self, 0));
}

void i_bool_and(CodeNode* self)
{
   Trace("i_bool_and");

   assert(code_is_valid(self));
   
   code_value_bool(self,
      Code_eval_child_bool(self, 0) && Code_eval_child_bool(self, 1));
}

void i_bool_or(CodeNode* self)
{
   Trace("i_bool_or");

   assert(code_is_valid(self));
   
   code_value_bool(self,
      Code_eval_child_bool(self, 0) || Code_eval_child_bool(self, 1));
}

void i_bool_eq(CodeNode* self)
{
   CodeNode* op1;
   CodeNode* op2;
   CodeType  tp1;
   CodeType  tp2;
   Bool      result;
   
   Trace("i_bool_eq");

   assert(code_is_valid(self));

   op1 = Code_eval_child(self, 0);
   op2 = Code_eval_child(self, 1);
   tp1 = code_get_type(op1);
   tp2 = code_get_type(op2);

   if (tp1 != tp2)
   {
      fprintf(stderr, "*** Error: Comparison of different types\n");
      code_errmsg(self);
      abort();
   }
   assert(tp1 == tp2);

   switch(tp1)
   {
   case CODE_NUMB :
      result = EQ(code_get_numb(op1), code_get_numb(op2));
      break;
   case CODE_STRG :
      result = strcmp(code_get_strg(op1), code_get_strg(op2)) == 0;
      break;
   default :
      abort();
   }
   code_value_bool(self, result);
}

void i_bool_ne(CodeNode* self)
{
   Trace("i_bool_ne");

   assert(code_is_valid(self));

   i_bool_eq(self);
   
   code_value_bool(self, !code_get_bool(self));
}

void i_bool_ge(CodeNode* self)
{
   Trace("i_bool_ge");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      GE(Code_eval_child_numb(self, 0), Code_eval_child_numb(self, 1)));
}

void i_bool_gt(CodeNode* self)
{
   Trace("i_bool_gt");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      GT(Code_eval_child_numb(self, 0), Code_eval_child_numb(self, 1)));
}

void i_bool_le(CodeNode* self)
{
   Trace("i_bool_le");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      LE(Code_eval_child_numb(self, 0), Code_eval_child_numb(self, 1)));
}

void i_bool_lt(CodeNode* self)
{
   Trace("i_bool_lt");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      LT(Code_eval_child_numb(self, 0), Code_eval_child_numb(self, 1)));
}

void i_bool_seq(CodeNode* self)
{
   Set* set_a;
   Set* set_b;
      
   Trace("i_bool_seq");

   assert(code_is_valid(self));

   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);
   
   code_value_bool(self, set_is_equal(set_a, set_b));

   set_free(set_a);
   set_free(set_b);
}

void i_bool_sneq(CodeNode* self)
{
   Set* set_a;
   Set* set_b;

   Trace("i_bool_sneq");

   assert(code_is_valid(self));

   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);
   
   code_value_bool(self, !set_is_equal(set_a, set_b));

   set_free(set_a);
   set_free(set_b);
}

void i_bool_subs(CodeNode* self)
{
   Set* set_a;
   Set* set_b;

   Trace("i_bool_subs");

   assert(code_is_valid(self));
   
   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);

   code_value_bool(self, set_is_subset(set_a, set_b));

   set_free(set_a);
   set_free(set_b);
}

void i_bool_sseq(CodeNode* self)
{
   Set* set_a;
   Set* set_b;

   Trace("i_bool_sseq");

   assert(code_is_valid(self));
   
   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);

   code_value_bool(self, set_is_subseteq(set_a, set_b));

   set_free(set_a);
   set_free(set_b);
}

void i_bool_is_elem(CodeNode* self)
{
   Tuple* tuple;
   Set*   set;
   
   Trace("i_bool_is_elem");

   assert(code_is_valid(self));

   tuple = Code_eval_child_tuple(self, 0);
   set   = Code_eval_child_set(self, 1);

   code_value_bool(self, set_lookup(set, tuple));

   tuple_free(tuple);
   set_free(set);
}

void i_bool_exists(CodeNode* self)
{
   IdxSet*   idxset;
   Set*      set;
   Tuple*    pattern;
   Tuple*    tuple;
   CodeNode* lexpr;
   int       idx   = 0;
   Bool      exists = FALSE;

   Trace("i_bool_exists");
   
   assert(code_is_valid(self));

   idxset  = Code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);

   while(!exists && (tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      exists = code_get_bool(code_eval(lexpr));

      local_drop_frame();

      tuple_free(tuple);
   }
   tuple_free(pattern);

   code_value_bool(self, exists);

   set_free(set);   
   idxset_free(idxset);
}

/* ----------------------------------------------------------------------------
 * Set Funktionen
 * ----------------------------------------------------------------------------
 */
void i_set_new_tuple(CodeNode* self)
{
   List*     list  = Code_eval_child_list(self, 0);
   ListElem* le    = NULL;
   Tuple*    tuple = list_get_tuple(list, &le);
   int       dim   = tuple_get_dim(tuple);
   Set*      set   = set_new(dim);
   int       n     = list_get_elems(list);
   int       i;
   
   Trace("i_set_new_tuple");

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

void i_set_new_elem(CodeNode* self)
{
   List*     list  = Code_eval_child_list(self, 0);
   ListElem* le    = NULL;
   Set*      set   = set_new(1);
   int       n     = list_get_elems(list);
   Tuple*    tuple;
   const Elem* elem;
   int       i;
   
   Trace("i_set_new_elem");

   assert(code_is_valid(self));

   /* Und jetzt noch mal alle.
    */
   le  = NULL;

   for(i = 0; i < n; i++)
   {
      elem  = list_get_elem(list, &le);
      tuple = tuple_new(1);

      tuple_set_elem(tuple, 0, elem);      
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

   dim = Code_eval_child_size(self, 0);
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

void i_set_union(CodeNode* self)
{
   Set* set_a;
   Set* set_b;
   Set* set_r;
   
   Trace("i_set_union");

   assert(code_is_valid(self));

   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error: Union of incompatible sets\n");
      code_errmsg(self);
      abort();
   }
   set_r = set_union(set_a, set_b);
   
   code_value_set(self, set_r);

   set_free(set_a);
   set_free(set_b);
   set_free(set_r);
}

void i_set_minus(CodeNode* self)
{
   Set* set_a;
   Set* set_b;
   Set* set_r;
   
   Trace("i_set_minus");

   assert(code_is_valid(self));

   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error: Minus of incompatible sets\n");
      code_errmsg(self);
      abort();
   }
   set_r = set_minus(set_a, set_b);
   
   code_value_set(self, set_r);

   set_free(set_a);
   set_free(set_b);
   set_free(set_r);
}

void i_set_inter(CodeNode* self)
{
   Set* set_a;
   Set* set_b;
   Set* set_r;
   
   Trace("i_set_inter");

   assert(code_is_valid(self));

   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error: Intersection of incompatible sets\n");
      code_errmsg(self);
      abort();
   }
   set_r = set_inter(set_a, set_b);
   
   code_value_set(self, set_r);

   set_free(set_a);
   set_free(set_b);
   set_free(set_r);
}

void i_set_sdiff(CodeNode* self)
{
   Set* set_a;
   Set* set_b;
   Set* set_r;
   
   Trace("i_set_sdiff");

   assert(code_is_valid(self));

   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error: Symetric Difference of incompatible sets\n");
      code_errmsg(self);
      abort();
   }
   set_r = set_sdiff(set_a, set_b);
   
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

   set_a = Code_eval_child_set(self, 0);
   set_b = Code_eval_child_set(self, 1);
   set_r = set_cross(set_a, set_b);

   code_value_set(self, set_r);

   set_free(set_a);
   set_free(set_b);
   set_free(set_r);
}

void i_set_range(CodeNode* self)
{
   Set*   set;
   double from;
   double upto;
   double step;
   
   Trace("i_set_range");

   assert(code_is_valid(self));

   from = Code_eval_child_numb(self, 0);
   upto = Code_eval_child_numb(self, 1);
   step = Code_eval_child_numb(self, 2);
   set  = set_range(from, upto, step);

   code_value_set(self, set);

   set_free(set);
}

/* ----------------------------------------------------------------------------
 * Tupel Funktionen
 * ----------------------------------------------------------------------------
 */
void i_tuple_new(CodeNode* self)
{
   List*      list  = Code_eval_child_list(self, 0);
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
   const char*  name  = Code_eval_child_name(self, 0);
   Set*         iset  = Code_eval_child_set(self, 1);
   Symbol*      sym   = symbol_new(name, SYM_SET, iset);
   Tuple*       tuple = tuple_new(0);   
   Set*         set   = Code_eval_child_set(self, 2);
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

void i_newsym_para(CodeNode* self)
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
   
   Trace("i_newsym_para");

   assert(code_is_valid(self));

   name = Code_eval_child_name(self, 0);
   set  = Code_eval_child_set(self, 1);
   list = Code_eval_child_list(self, 2);

   if (!list_is_entrylist(list))
   {
      /* This errors occurs, if the parameter is mssing in the template
       * for a "read" statement.
       */
      assert(list_is_tuplelist(list));
      
      fprintf(stderr, "*** Error: Values in parameter list missing,\n");
      fprintf(stderr, "           probably wrong read template\n");      
      code_errmsg(self);
      abort();
   }
   
   /* First element will determine the type
    */
   sym  = symbol_new(name, SYM_ERR, set);
   
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
         fprintf(stderr, "*** Error: Illegal element ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " for symbol\n");
         code_errmsg(self);
         abort();
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
   const char* name;
   IdxSet*     idxset;
   Set*        set;
   Symbol*     sym;
   Tuple*      tuple;
   Tuple*      pattern;
   VarType     vartype;
   Var*        var;
   Entry*      entry;
   int         idx = 0;
   double      lower;
   double      upper;
   char*       tuplestr;
   char*       varname;
   
   Trace("i_newsym_var");

   assert(code_is_valid(self));

   name    = Code_eval_child_name(self, 0);
   idxset  = Code_eval_child_idxset(self, 1);
   vartype = Code_eval_child_vartype(self, 2);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   sym     = symbol_new(name, SYM_VAR, set);

   if ((!code_get_bool(code_eval(idxset_get_lexpr(idxset)))))
   {
      fprintf(stderr, "*** Error: WITH not allowed here\n");
      code_errmsg(self);
      abort();
   }

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);
      
      lower = Code_eval_child_numb(self, 3);
      upper = Code_eval_child_numb(self, 4);

      /* Hier geben wir der Variable einen eindeutigen Namen
       */
      tuplestr = tuple_tostr(tuple);
      varname  = malloc(strlen(name) + strlen(tuplestr) + 2);

      assert(varname != NULL);
      
      sprintf(varname, "%s_%s", name, tuplestr);

      /* Und nun legen wir sie an.
       */
      var = lps_addvar(varname, vartype, lower, upper);

      entry = entry_new_var(tuple, var);

      symbol_add_entry(sym, entry);

      free(varname);
      free(tuplestr);
      
      local_drop_frame();

      entry_free(entry);
      tuple_free(tuple);
   }
   set_free(set);
   tuple_free(pattern);
   idxset_free(idxset);

   code_value_void(self);
}

void i_symbol_deref(CodeNode* self)
{
   const char* name;
   Tuple*      tuple;
   const Entry* entry;
   Symbol*     sym;
   Set*        set;   
   Term*       term;
   
   Trace("i_symbol_deref");
   
   assert(code_is_valid(self));

   name  = Code_eval_child_name(self, 0);
   tuple = Code_eval_child_tuple(self, 1);   
   sym   = symbol_lookup(name);

   if (sym == NULL)
   {
      fprintf(stderr, "*** Error: Unknown symbol \"%s\"\n", name);
      code_errmsg(self);
      abort();
   }

   if (!symbol_has_entry(sym, tuple))
   {
      fprintf(stderr, "*** Error: Unknown index ");
      tuple_print(stderr, tuple);
      fprintf(stderr, " for symbol \"%s\"\n", name);
      code_errmsg(self);
      abort();
   }
   entry = symbol_lookup_entry(sym, tuple);

   tuple_free(tuple);

   switch(symbol_get_type(sym))
   {
   case SYM_NUMB :
      code_value_numb(self, entry_get_numb(entry));
      break;
   case SYM_STRG :
      code_value_strg(self, entry_get_strg(entry));
      break;
   case SYM_SET :
      set = entry_get_set(entry);
      code_value_set(self, set);
      set_free(set);
      break;
   case SYM_VAR :
      term = term_new(1);
      term_add_elem(term, entry, 1.0);
      code_value_term(self, term);
      term_free(term);
      break;
   default :
      abort();
   }
   /* entry_free(entry); */
}

/* ----------------------------------------------------------------------------
 * Index Set Funktionen
 * ----------------------------------------------------------------------------
 */
void i_idxset_new(CodeNode* self)
{
   IdxSet*   idxset;
   Tuple*    tuple;
   Tuple*    t0;
   Set*      set;
   char      name[13]; /* "@-2000000000" */
   int       dim;
   int       i;
         
   Trace("i_idxset_new");
   
   assert(code_is_valid(self));

   t0     = tuple_new(0);
   tuple  = Code_eval_child_tuple(self, 0);
   set    = Code_eval_child_set(self, 1);

   /* Wenn kein Index tuple angegeben wurde, konstruieren wir eins.
    */
   if (!tuple_cmp(tuple, t0))
   {
      tuple_free(tuple);

      dim   = set_get_dim(set);
      tuple = tuple_new(dim);
      
      for(i = 0; i < dim; i++)
      {
         sprintf(name, "@%d", i + 1); 
         tuple_set_elem(tuple, i, elem_new_name(str_new(name)));
      }
   }
   idxset = idxset_new(tuple, set, code_get_child(self, 2));

   code_value_idxset(self, idxset);

   tuple_free(t0);
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

   name = Code_eval_child_name(self, 0);   
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
         abort();
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

   term  = Code_eval_child_term(self, 0);
   coeff = Code_eval_child_numb(self, 1);

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

   term_a = Code_eval_child_term(self, 0);
   term_b = Code_eval_child_term(self, 1);
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

   term_a = Code_eval_child_term(self, 0);
   term_b = Code_eval_child_term(self, 1);
   term_negate(term_b);
   term_r = term_add_term(term_a, term_b);
   
   code_value_term(self, term_r);

   term_free(term_a);
   term_free(term_b);
   term_free(term_r);
}

void i_term_sum(CodeNode* self)
{
   IdxSet*   idxset;
   Term*     term_a;
   Term*     term_b;
   Term*     term_r = NULL;
   Set*      set;
   Tuple*    pattern;
   Tuple*    tuple;
   CodeNode* lexpr;
   int       idx = 0;

   Trace("i_term_sum");
   
   assert(code_is_valid(self));

   idxset  = Code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   term_a  = term_new(1);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         term_b = Code_eval_child_term(self, 1);      
         term_r = term_add_term(term_a, term_b);

         term_free(term_a);
         term_free(term_b);
         term_a = term_r;
      }
      local_drop_frame();

      tuple_free(tuple);
   }
   tuple_free(pattern);

   if (term_r == NULL)
      term_r = term_new(1);
   
   code_value_term(self, term_r);

   set_free(set);   
   term_free(term_r);
   idxset_free(idxset);
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
   
   Trace("i_entry");
   
   assert(code_is_valid(self));

   tuple = Code_eval_child_tuple(self, 0);
   child = Code_eval_child(self, 1);

   switch(code_get_type(child))
   {
   case CODE_NUMB :
      entry = entry_new_numb(tuple, code_get_numb(child));
      break;
   case CODE_STRG :
      entry = entry_new_strg(tuple, code_get_strg(child));
      break;
   default :
      abort();
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
   CodeNode*   child = Code_eval_child(self, 0);
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
      abort();
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

   list  = Code_eval_child_list(self, 0);
   child = Code_eval_child(self, 1);

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
      abort();
   }
   list_add_elem(list, elem);   
   code_value_list(self, list);

   list_free(list);
}

void i_tuple_list_new(CodeNode* self)
{
   Tuple* tuple = Code_eval_child_tuple(self, 0);
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

   list  = Code_eval_child_list(self, 0);
   tuple = Code_eval_child_tuple(self, 1);
   
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

   entry = Code_eval_child_entry(self, 0);
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

   list  = Code_eval_child_list(self, 0);
   entry = Code_eval_child_entry(self, 1);

   list_add_entry(list, entry);   
   code_value_list(self, list);
   
   entry_free(entry);
   list_free(list);
}

static void objective(CodeNode* self, Bool minimize)
{
   const char* name;
   Term*       term;
   
   assert(code_is_valid(self));

   name = Code_eval_child_name(self, 0);
   term = Code_eval_child_term(self, 1);

   lps_objname(name);
   lps_setdir(minimize ? LP_MIN : LP_MAX);
   term_to_objective(term);

   term_free(term);
   
   code_value_void(self);
}

void i_object_min(CodeNode* self)
{
   Trace("i_object_min");

   assert(code_is_valid(self));

   objective(self, TRUE);
}

void i_object_max(CodeNode* self)
{
   Trace("i_object_max");

   assert(code_is_valid(self));

   objective(self, FALSE);
}






