#ident "@(#) $Id: inst.c,v 1.27 2002/10/29 10:42:47 bzfkocht Exp $"
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
CodeNode* i_nop(CodeNode* self)
{
   Trace("i_nop");

   assert(code_is_valid(self));

   return self;
}

CodeNode* i_subto(CodeNode* self)
{
   const char* name;
   
   Trace("i_subto");
   
   assert(code_is_valid(self));

   name = code_eval_child_name(self, 0);

   if (!conname_set(name))
   {
      fprintf(stderr, "*** Error: Duplicate constraint name \"%s\"\n", name);
      code_errmsg(self);
      abort();
   }
   (void)code_eval_child(self, 1); /* constraint */

   conname_free();
   
   code_value_void(self);

   return self;
}

CodeNode* i_constraint(CodeNode* self)
{
   Term*        term;
   const Term*  term_lhs;
   const Term*  term_rhs;
   ConType      type;
   Con*         con;
   double       rhs;
   unsigned int flags;
   
   Trace("i_constraint");
   
   assert(code_is_valid(self));
   
   term_lhs   = code_eval_child_term(self, 0);
   type       = code_eval_child_contype(self, 1);
   term_rhs   = code_eval_child_term(self, 2);
   flags      = code_eval_child_bits(self, 3);

   rhs        = term_get_constant(term_rhs) - term_get_constant(term_lhs);
   term       = term_sub_term(term_lhs, term_rhs);
   con        = lps_addcon(conname_get(), type, rhs, flags);

   term_add_constant(term, rhs);
   term_to_nzo(term, con);
   
   code_value_void(self);

   term_free(term);
   
   return self;
}

CodeNode* i_forall(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   const Tuple*  tuple;
   CodeNode*     lexpr;
   int           idx   = 0;
   
   Trace("i_forall");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);  
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   
   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
         (void)code_eval_child(self, 1); /* constraint */

      local_drop_frame();
   }
   code_value_void(self);

   return self;
}

/* ----------------------------------------------------------------------------
 * Arithmetische Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_expr_add(CodeNode* self)
{
   Trace("i_add");

   assert(code_is_valid(self));

   code_value_numb(self,
      code_eval_child_numb(self, 0) + code_eval_child_numb(self, 1));

   return self;
}

CodeNode* i_expr_sub(CodeNode* self)
{
   Trace("i_sub");

   assert(code_is_valid(self));

   code_value_numb(self,
      code_eval_child_numb(self, 0) - code_eval_child_numb(self, 1));

   return self;
}

CodeNode* i_expr_mul(CodeNode* self)
{
   Trace("i_mul");

   assert(code_is_valid(self));

   code_value_numb(self, 
      code_eval_child_numb(self, 0) * code_eval_child_numb(self, 1));

   return self;
}

CodeNode* i_expr_div(CodeNode* self)
{
   double divisor;
   
   Trace("i_div");

   assert(code_is_valid(self));

   divisor = code_eval_child_numb(self, 1);

   if (EQ(divisor, 0.0))
   {
      fprintf(stderr, "*** Error: Division by zero\n");
      code_errmsg(self);
      abort();
   }      
   code_value_numb(self,
      code_eval_child_numb(self, 0) / divisor);

   return self;
}

CodeNode* i_expr_mod(CodeNode* self)
{
   Trace("i_mod");

   assert(code_is_valid(self));

   code_value_numb(self,
      fmod(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_expr_intdiv(CodeNode* self)
{
   Trace("i_intdiv");

   assert(code_is_valid(self));

   code_value_numb(self,
      floor(code_eval_child_numb(self, 0) / code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_expr_pow(CodeNode* self)
{
   Trace("i_pow");

   assert(code_is_valid(self));

   code_value_numb(self,
      pow(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_expr_neg(CodeNode* self)
{
   Trace("i_neg");

   assert(code_is_valid(self));

   code_value_numb(self, -code_eval_child_numb(self, 0));

   return self;
}

CodeNode* i_expr_abs(CodeNode* self)
{
   Trace("i_abs");

   assert(code_is_valid(self));

   code_value_numb(self, fabs(code_eval_child_numb(self, 0)));

   return self;
}

CodeNode* i_expr_floor(CodeNode* self)
{
   Trace("i_floor");

   assert(code_is_valid(self));

   code_value_numb(self, floor(code_eval_child_numb(self, 0)));

   return self;
}

CodeNode* i_expr_ceil(CodeNode* self)
{
   Trace("i_ceil");

   assert(code_is_valid(self));

   code_value_numb(self, ceil(code_eval_child_numb(self, 0)));

   return self;
}

CodeNode* i_expr_log(CodeNode* self)
{
   double exponent;
   
   Trace("i_log");

   assert(code_is_valid(self));

   exponent = code_eval_child_numb(self, 0);
   
   if (EQ(exponent, 0.0))
   {
      fprintf(stderr, "*** Error: log of zero\n");
      code_errmsg(self);
      abort();
   }      
   code_value_numb(self, log10(exponent));

   return self;
}

CodeNode* i_expr_ln(CodeNode* self)
{
   double exponent;
   
   Trace("i_ln");

   assert(code_is_valid(self));

   exponent = code_eval_child_numb(self, 0);
   
   if (EQ(exponent, 0.0))
   {
      fprintf(stderr, "*** Error: ln of zero\n");
      code_errmsg(self);
      abort();
   }      
   code_value_numb(self, log(exponent));

   return self;
}

CodeNode* i_expr_exp(CodeNode* self)
{
   Trace("i_exp");

   assert(code_is_valid(self));

   code_value_numb(self, exp(code_eval_child_numb(self, 0)));

   return self;
}

CodeNode* i_expr_fac(CodeNode* self)
{
   double x;
   double y = 1.0;
   double z;
   
   Trace("i_exp");

   assert(code_is_valid(self));

   x = code_eval_child_numb(self, 0);

   for(z = 2.0; z <= x; z += 1.0)
      y *= z;

   code_value_numb(self, y);

   return self;
}


CodeNode* i_expr_card(CodeNode* self)
{
   const Set* set;
   
   Trace("i_abs");

   assert(code_is_valid(self));

   set = code_eval_child_set(self, 0);
   
   code_value_numb(self, (double)set_get_used(set));

   return self;
}

CodeNode* i_expr_if(CodeNode* self)
{
   Trace("i_if");

   assert(code_is_valid(self));

   if (code_eval_child_bool(self, 0))
      code_value_numb(self, code_eval_child_numb(self, 1));
   else
      code_value_numb(self, code_eval_child_numb(self, 2));

   return self;
}

CodeNode* i_expr_min(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   const Tuple*  tuple;
   CodeNode*     lexpr;
   int           idx = 0;
   double        value;
   double        min = DBL_MAX;

   Trace("i_expr_min");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         value = code_eval_child_numb(self, 1);      

         if (value < min)
            min = value;
      }
      local_drop_frame();
   }
   code_value_numb(self, min);

   return self;
}

CodeNode* i_expr_max(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   const Tuple*  tuple;
   CodeNode*     lexpr;
   int           idx = 0;
   double        value;
   double        max = -DBL_MAX;

   Trace("i_expr_max");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         value = code_eval_child_numb(self, 1);      

         if (value > max)
            max = value;
      }
      local_drop_frame();
   }
   code_value_numb(self, max);

   return self;
}

CodeNode* i_expr_sum(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   const Tuple*  tuple;
   CodeNode*     lexpr;
   int           idx = 0;
   double        sum = 0;

   Trace("i_expr_sum");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
         sum += code_eval_child_numb(self, 1);      

      local_drop_frame();
   }
   code_value_numb(self, sum);

   return self;
}

/* ----------------------------------------------------------------------------
 * Logische Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_bool_true(CodeNode* self)
{
   Trace("i_bool_true");

   assert(code_is_valid(self));
   
   code_value_bool(self, TRUE);

   return self;
}

CodeNode* i_bool_false(CodeNode* self)
{
   Trace("i_bool_false");

   assert(code_is_valid(self));
   
   code_value_bool(self, FALSE);

   return self;
}

CodeNode* i_bool_not(CodeNode* self)
{
   Trace("i_bool_not");

   assert(code_is_valid(self));
   
   code_value_bool(self, !code_eval_child_bool(self, 0));

   return self;
}

CodeNode* i_bool_and(CodeNode* self)
{
   Trace("i_bool_and");

   assert(code_is_valid(self));
   
   code_value_bool(self,
      code_eval_child_bool(self, 0) && code_eval_child_bool(self, 1));

   return self;
}

CodeNode* i_bool_or(CodeNode* self)
{
   Trace("i_bool_or");

   assert(code_is_valid(self));
   
   code_value_bool(self,
      code_eval_child_bool(self, 0) || code_eval_child_bool(self, 1));

   return self;
}

CodeNode* i_bool_eq(CodeNode* self)
{
   CodeNode* op1;
   CodeNode* op2;
   CodeType  tp1;
   CodeType  tp2;
   Bool      result;
   
   Trace("i_bool_eq");

   assert(code_is_valid(self));

   op1 = code_eval_child(self, 0);
   op2 = code_eval_child(self, 1);
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

   return self;
}

CodeNode* i_bool_ne(CodeNode* self)
{
   Trace("i_bool_ne");

   assert(code_is_valid(self));

   code_value_bool(self, !code_get_bool(i_bool_eq(self)));

   return self;
}

CodeNode* i_bool_ge(CodeNode* self)
{
   Trace("i_bool_ge");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      GE(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_bool_gt(CodeNode* self)
{
   Trace("i_bool_gt");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      GT(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_bool_le(CodeNode* self)
{
   Trace("i_bool_le");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      LE(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_bool_lt(CodeNode* self)
{
   Trace("i_bool_lt");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      LT(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_bool_seq(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
      
   Trace("i_bool_seq");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);
   
   code_value_bool(self, set_is_equal(set_a, set_b));

   return self;
}

CodeNode* i_bool_sneq(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;

   Trace("i_bool_sneq");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);
   
   code_value_bool(self, !set_is_equal(set_a, set_b));

   return self;
}

CodeNode* i_bool_subs(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;

   Trace("i_bool_subs");

   assert(code_is_valid(self));
   
   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   code_value_bool(self, set_is_subset(set_a, set_b));

   return self;
}

CodeNode* i_bool_sseq(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;

   Trace("i_bool_sseq");

   assert(code_is_valid(self));
   
   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   code_value_bool(self, set_is_subseteq(set_a, set_b));

   return self;
}

CodeNode* i_bool_is_elem(CodeNode* self)
{
   const Tuple* tuple;
   const Set*   set;
   
   Trace("i_bool_is_elem");

   assert(code_is_valid(self));

   tuple = code_eval_child_tuple(self, 0);
   set   = code_eval_child_set(self, 1);

   code_value_bool(self, set_lookup(set, tuple));

   return self;
}

CodeNode* i_bool_exists(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   const Tuple*  tuple;
   CodeNode*     lexpr;
   int           idx   = 0;
   Bool          exists = FALSE;

   Trace("i_bool_exists");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);

   while(!exists && (tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      exists = code_get_bool(code_eval(lexpr));

      local_drop_frame();
   }
   code_value_bool(self, exists);

   return self;
}

/* ----------------------------------------------------------------------------
 * Set Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_set_new_tuple(CodeNode* self)
{
   const List*  list  = code_eval_child_list(self, 0);
   ListElem*    le    = NULL;
   const Tuple* tuple = list_get_tuple(list, &le);
   int          dim   = tuple_get_dim(tuple);
   Set*         set   = set_new(dim);
   int          n     = list_get_elems(list);
   int          i;
   
   Trace("i_set_new_tuple");

   assert(code_is_valid(self));

   /* Und jetzt noch mal alle.
    */
   le  = NULL;

   for(i = 0; i < n; i++)
   {
      tuple = list_get_tuple(list, &le);
      set_add_member(set, tuple, SET_ADD_END, SET_CHECK_WARN);
   }
   code_value_set(self, set);

   set_free(set);

   return self;
}

CodeNode* i_set_new_elem(CodeNode* self)
{
   const List* list  = code_eval_child_list(self, 0);
   ListElem*   le    = NULL;
   Set*        set   = set_new(1);
   int         n     = list_get_elems(list);
   Tuple*      tuple;
   const Elem* elem;
   int         i;
   
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
      set_add_member(set, tuple, SET_ADD_END, SET_CHECK_WARN);

      tuple_free(tuple);
   }
   code_value_set(self, set);

   set_free(set);

   return self;
}

CodeNode* i_set_empty(CodeNode* self)
{
   Set*      set;
   Tuple*    tuple;
   int       dim;
   
   Trace("i_set_empty");

   assert(code_is_valid(self));

   dim = code_eval_child_size(self, 0);
   set = set_new(dim);

   if (dim == 0)
   {
      tuple = tuple_new(0);
      set_add_member(set, tuple, SET_ADD_END, SET_CHECK_NONE);
      tuple_free(tuple);
   }
   code_value_set(self, set);

   set_free(set);

   return self;
}

CodeNode* i_set_union(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   Set*       set_r;
   
   Trace("i_set_union");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error: Union of incompatible sets\n");
      code_errmsg(self);
      abort();
   }
   set_r = set_union(set_a, set_b);
   
   code_value_set(self, set_r);

   set_free(set_r);

   return self;
}

CodeNode* i_set_minus(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   Set*       set_r;
   
   Trace("i_set_minus");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error: Minus of incompatible sets\n");
      code_errmsg(self);
      abort();
   }
   set_r = set_minus(set_a, set_b);
   
   code_value_set(self, set_r);

   set_free(set_r);

   return self;
}

CodeNode* i_set_inter(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   Set*       set_r;
   
   Trace("i_set_inter");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error: Intersection of incompatible sets\n");
      code_errmsg(self);
      abort();
   }
   set_r = set_inter(set_a, set_b);
   
   code_value_set(self, set_r);

   set_free(set_r);

   return self;
}

CodeNode* i_set_sdiff(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   Set*       set_r;
   
   Trace("i_set_sdiff");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error: Symetric Difference of incompatible sets\n");
      code_errmsg(self);
      abort();
   }
   set_r = set_sdiff(set_a, set_b);
   
   code_value_set(self, set_r);

   set_free(set_r);

   return self;
}

CodeNode* i_set_cross(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   Set*       set_r;
   
   Trace("i_set_cross");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);
   set_r = set_cross(set_a, set_b);

   code_value_set(self, set_r);

   set_free(set_r);

   return self;
}

CodeNode* i_set_range(CodeNode* self)
{
   Set*   set;
   double from;
   double upto;
   double step;
   
   Trace("i_set_range");

   assert(code_is_valid(self));

   from = code_eval_child_numb(self, 0);
   upto = code_eval_child_numb(self, 1);
   step = code_eval_child_numb(self, 2);
   set  = set_range(from, upto, step);

   code_value_set(self, set);

   set_free(set);

   return self;
}

CodeNode* i_set_proj(CodeNode* self)
{
   const Set*   set_a;
   const Tuple* tuple;
   const Elem*  elem;
   int          dim;
   int          idx;
   Set*         set_r;
   int          i;
   
   Trace("i_set_proj");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   tuple = code_eval_child_tuple(self, 1);
   dim   = set_get_dim(set_a);
   
   for(i = 0; i < tuple_get_dim(tuple); i++)
   {
      elem = tuple_get_elem(tuple, i);

      /* Are there only number in the selection tuple ?
       */
      if (ELEM_NUMB != elem_get_type(elem))
      {
         fprintf(stderr, "*** Error: Illegal value type in tuple: ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " only numbers are possible\n");
         code_errmsg(self);
         abort();
      }
      idx = (int)elem_get_numb(elem);
      
      /* Are all the number between 1 and dim(set) ?
       */
      if (idx < 1 || idx > dim)
      {
         fprintf(stderr, "*** Error: Illegal index %d, ", idx);
         fprintf(stderr, "           set has only dimension %d\n", dim);
         code_errmsg(self);
         abort();
      }
   }
   set_r = set_proj(set_a, tuple);

   code_value_set(self, set_r);

   set_free(set_r);

   return self;
}

/* ----------------------------------------------------------------------------
 * Tupel Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_tuple_new(CodeNode* self)
{
   const List* list  = code_eval_child_list(self, 0);
   int         n     = list_get_elems(list);
   Tuple*      tuple = tuple_new(n);
   ListElem*   le    = NULL;
   int         i;

   Trace("i_tuple_new");
   
   assert(code_is_valid(self));
   
   for(i = 0; i < n; i++)
      tuple_set_elem(tuple, i, list_get_elem(list, &le));
   
   code_value_tuple(self, tuple);

   tuple_free(tuple);

   return self;
}

CodeNode* i_tuple_empty(CodeNode* self)
{
   Tuple* tuple;
   
   assert(code_is_valid(self));

   tuple = tuple_new(0);
   code_value_tuple(self, tuple);
   tuple_free(tuple);

   return self;
}

/* ----------------------------------------------------------------------------
 * Symbol Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_newsym_set(CodeNode* self)
{
   const char*   name    = code_eval_child_name(self, 0);
   const Set*    iset    = code_eval_child_set(self, 1);
   const IdxSet* idxset  = code_eval_child_idxset(self, 2);

   const Set*    set     = idxset_get_set(idxset);
   CodeNode*     lexpr   = idxset_get_lexpr(idxset);
   Symbol*       sym     = symbol_new(name, SYM_SET, iset);
   Tuple*        empty   = tuple_new(0);   

   const Tuple*  pattern;
   const Tuple*  tuple;
   Set*          newset;
   Entry*        entry;
   int           idx;
   
   Trace("i_newsym_set");

   assert(code_is_valid(self));

   /* Is it a simple set ?
    */
   if (code_get_inst(lexpr) == (Inst)i_bool_true)
   {
      entry = entry_new_set(empty, set);
   }
   else
   {
      pattern = idxset_get_tuple(idxset);
      newset  = set_new(tuple_get_dim(pattern));
      idx     = 0;
      
      while((tuple = set_match_next(set, pattern, &idx)) != NULL)
      {
         local_install_tuple(pattern, tuple);

         if (code_get_bool(code_eval(lexpr)))
            set_add_member(newset, tuple, SET_ADD_END, SET_CHECK_WARN);

         local_drop_frame();
      }
      entry = entry_new_set(empty, newset);

      set_free(newset);
   }
   symbol_add_entry(sym, entry);

   tuple_free(empty);
   entry_free(entry);
   
   code_value_void(self);

   return self;
}

/* initialisation per list
 */
CodeNode* i_newsym_para1(CodeNode* self)
{
   const char*   name;
   const Set*    set;
   const IdxSet* idxset;
   Symbol*       sym;
   const List*   list;
   ListElem*     lelem;
   const Entry*  entry;
   const Tuple*  tuple;
   int           count;
   int           i;
   
   Trace("i_newsym_para1");

   assert(code_is_valid(self));

   name   = code_eval_child_name(self, 0);
   idxset = code_eval_child_idxset(self, 1);
   set    = idxset_get_set(idxset);
   list   = code_eval_child_list(self, 2);

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
   }
   code_value_void(self);

   return self;
}

/* initialisation per element
 */
CodeNode* i_newsym_para2(CodeNode* self)
{
   const char*   name;
   const Set*    set;
   const IdxSet* idxset;
   Symbol*       sym;
   Entry*        entry;
   const Tuple*  tuple;
   const Tuple*  pattern;
   int           idx = 0;
   
   Trace("i_newsym_para2");

   assert(code_is_valid(self));

   name    = code_eval_child_name(self, 0);
   idxset  = code_eval_child_idxset(self, 1);
   set     = idxset_get_set(idxset);
   sym     = symbol_new(name, SYM_NUMB, set);
   pattern = idxset_get_tuple(idxset);

   /* Why did I this for the variables ??? Is it neccessary ?
    */
   if ((!code_get_bool(code_eval(idxset_get_lexpr(idxset)))))
   {
      fprintf(stderr, "*** Error: WITH not allowed here\n");
      code_errmsg(self);
      abort();
   }

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);
      
      entry = entry_new_numb(tuple, code_eval_child_numb(self, 2));
      
      symbol_add_entry(sym, entry);
      
      local_drop_frame();
      
      entry_free(entry);
   }
   code_value_void(self);

   return self;
}

CodeNode* i_newsym_var(CodeNode* self)
{
   const char*   name;
   const IdxSet* idxset;
   const Set*    set;
   Symbol*       sym;
   const Tuple*  tuple;
   const Tuple*  pattern;
   VarType       vartype;
   Var*          var;
   Entry*        entry;
   int           idx = 0;
   double        lower;
   double        upper;
   double        priority;
   double        startval;
   char*         tuplestr;
   char*         varname;
   
   Trace("i_newsym_var");

   assert(code_is_valid(self));

   name    = code_eval_child_name(self, 0);
   idxset  = code_eval_child_idxset(self, 1);
   vartype = code_eval_child_vartype(self, 2);
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
      
      lower     = code_eval_child_numb(self, 3);
      upper     = code_eval_child_numb(self, 4);
      priority  = code_eval_child_numb(self, 5);
      startval  = code_eval_child_numb(self, 6);

      if ((vartype == VAR_BIN) && NE(lower, 0.0) && NE(upper, 1.0))
         fprintf(stderr,
            "*** Warning: Bounds for binary variable %s ignored\n",
            name);

      if ((vartype == VAR_CON)
         && (NE(priority, 0.0) || LT(startval, INFINITY)))
         fprintf(stderr,
            "*** Warning: Priority/Startval for continous var %s ignored\n",
            name);

      if ((vartype == VAR_INT) && EQ(lower, 0.0) && EQ(upper, 1.0))
         vartype = VAR_BIN;
      
      /* Hier geben wir der Variable einen eindeutigen Namen
       */
      tuplestr = tuple_tostr(tuple);
      varname  = malloc(strlen(name) + strlen(tuplestr) + 2);

      assert(varname != NULL);
      
      sprintf(varname, "%s_%s", name, tuplestr);

      /* Und nun legen wir sie an.
       */
      var = lps_addvar(varname, vartype, lower, upper,
         (int)priority, startval);

      entry = entry_new_var(tuple, var);

      symbol_add_entry(sym, entry);

      free(varname);
      free(tuplestr);
      
      local_drop_frame();

      entry_free(entry);
   }
   code_value_void(self);

   return self;
}

CodeNode* i_symbol_deref(CodeNode* self)
{
   const char*  name;
   const Tuple* tuple;
   const Entry* entry;
   Symbol*      sym;
   const Set*   set;   
   Term*        term;
   
   Trace("i_symbol_deref");
   
   assert(code_is_valid(self));

   name  = code_eval_child_name(self, 0);
   tuple = code_eval_child_tuple(self, 1);   
   sym   = symbol_lookup(name);

   if (sym == NULL)
   {
      fprintf(stderr, "*** Error: Unknown symbol \"%s\"\n", name);
      code_errmsg(self);
      abort();
   }

   entry = symbol_lookup_entry(sym, tuple);

   if (NULL == entry)
   {
      fprintf(stderr, "*** Error: Unknown index ");
      tuple_print(stderr, tuple);
      fprintf(stderr, " for symbol \"%s\"\n", name);
      code_errmsg(self);
      abort();
   }

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

   return self;
}

/* ----------------------------------------------------------------------------
 * Index Set Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_idxset_new(CodeNode* self)
{
   IdxSet*      idxset;
   Tuple*       tuple;
   Tuple*       t0;
   Elem*        elem;
   const Set*   set;
   char         name[13]; /* "@-2000000000" */
   int          dim;
   int          i;
         
   Trace("i_idxset_new");

   assert(code_is_valid(self));

   t0     = tuple_new(0);
   tuple  = tuple_copy(code_eval_child_tuple(self, 0));
   set    = code_eval_child_set(self, 1);

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
         elem = elem_new_name(str_new(name));
         tuple_set_elem(tuple, i, elem);
         elem_free(elem);
      }
   }
   idxset = idxset_new(tuple, set, code_get_child(self, 2));

   code_value_idxset(self, idxset);

   tuple_free(t0);
   tuple_free(tuple);
   idxset_free(idxset);

   return self;
}

/* ----------------------------------------------------------------------------
 * Local Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_local_deref(CodeNode* self)
{
   const char* name;
   const Elem* elem;
   
   Trace("i_local_deref");
   
   assert(code_is_valid(self));

   name = code_eval_child_name(self, 0);   
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
   return self;
}   

/* ----------------------------------------------------------------------------
 * Term Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_term_coeff(CodeNode* self)
{
   Term*  term;
   double coeff;
   
   Trace("i_term_coeff");
   
   assert(code_is_valid(self));

   term  = term_copy(code_eval_child_term(self, 0));
   coeff = code_eval_child_numb(self, 1);

   term_mul_coeff(term, coeff);
   
   code_value_term(self, term);

   term_free(term);

   return self;
}

CodeNode* i_term_const(CodeNode* self)
{
   Term*  term;
   double numb;
   
   Trace("i_term_const");
   
   assert(code_is_valid(self));

   term = term_copy(code_eval_child_term(self, 0));
   numb = code_eval_child_numb(self, 1);

   term_add_constant(term, numb);
   
   code_value_term(self, term);

   term_free(term);

   return self;
}

CodeNode* i_term_add(CodeNode* self)
{
   const Term* term_a;
   const Term* term_b;
   Term*       term_r;

   Trace("i_term_add");
   
   assert(code_is_valid(self));

   term_a = code_eval_child_term(self, 0);
   term_b = code_eval_child_term(self, 1);
   term_r = term_add_term(term_a, term_b);
   
   code_value_term(self, term_r);

   term_free(term_r);

   return self;
}

CodeNode* i_term_sub(CodeNode* self)
{
   const Term* term_a;
   Term*       term_b;
   Term*       term_r;
   
   Trace("i_term_sub");
   
   assert(code_is_valid(self));

   term_a = code_eval_child_term(self, 0);
   term_b = term_copy(code_eval_child_term(self, 1));
   term_r = term_sub_term(term_a, term_b);
   
   code_value_term(self, term_r);

   term_free(term_b);
   term_free(term_r);

   return self;
}

CodeNode* i_term_sum(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   const Tuple*  tuple;
   CodeNode*     lexpr;
   int           idx = 0;
   Term*         term_a;
   Term*         term_b;
   Term*         term_r = NULL;

   Trace("i_term_sum");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   term_r  = term_new(1);

   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         term_b = term_copy(code_eval_child_term(self, 1));      
         term_a = term_add_term(term_r, term_b);

         term_free(term_r);
         term_free(term_b);
         term_r = term_a;
      }
      local_drop_frame();
   }
   code_value_term(self, term_r);

   term_free(term_r);

   return self;
}

CodeNode* i_term_expr(CodeNode* self)
{
   Term*  term;
   double numb;
   
   Trace("i_term_expr");
   
   assert(code_is_valid(self));

   term  = term_new(1);
   numb  = code_eval_child_numb(self, 0);

   term_add_constant(term, numb);
   
   code_value_term(self, term);

   term_free(term);

   return self;
}

/* ----------------------------------------------------------------------------
 * Entry Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_entry(CodeNode* self)
{
   const Tuple* tuple;
   Entry*       entry;
   CodeNode*    child;
   
   Trace("i_entry");
   
   assert(code_is_valid(self));

   tuple = code_eval_child_tuple(self, 0);
   child = code_eval_child(self, 1);

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

   entry_free(entry);

   return self;
}

/* ----------------------------------------------------------------------------
 * List Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_elem_list_new(CodeNode* self)
{
   CodeNode* child = code_eval_child(self, 0);
   Elem*     elem;
   List*     list;
   
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

   elem_free(elem);
   list_free(list);

   return self;
}

CodeNode* i_elem_list_add(CodeNode* self)
{
   CodeNode* child;
   Elem*     elem;
   List*     list;
   
   Trace("i_elem_list_add");

   assert(code_is_valid(self));

   list  = list_copy(code_eval_child_list(self, 0));
   child = code_eval_child(self, 1);

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

   elem_free(elem);
   list_free(list);

   return self;
}

CodeNode* i_tuple_list_new(CodeNode* self)
{
   const Tuple* tuple = code_eval_child_tuple(self, 0);
   List*        list  = list_new_tuple(tuple);
   
   Trace("i_tuple_list_new");

   assert(code_is_valid(self));

   code_value_list(self, list);

   list_free(list);

   return self;
}

CodeNode* i_tuple_list_add(CodeNode* self)
{
   const Tuple* tuple;
   List*        list;
   
   Trace("i_tuple_list_add");

   assert(code_is_valid(self));

   list  = list_copy(code_eval_child_list(self, 0));
   tuple = code_eval_child_tuple(self, 1);
   
   list_add_tuple(list, tuple);   

   code_value_list(self, list);

   list_free(list);

   return self;
}

CodeNode* i_entry_list_new(CodeNode* self)
{
   const Entry* entry;
   List*        list;
   
   Trace("i_entry_list_new");

   assert(code_is_valid(self));

   entry = code_eval_child_entry(self, 0);
   list  = list_new_entry(entry);
   
   code_value_list(self, list);

   list_free(list);

   return self;
}

CodeNode* i_entry_list_add(CodeNode* self)
{
   const Entry* entry;
   List*        list;
   
   Trace("i_entry_list_add");

   assert(code_is_valid(self));

   list  = list_copy(code_eval_child_list(self, 0));
   entry = code_eval_child_entry(self, 1);

   list_add_entry(list, entry);   
   code_value_list(self, list);
   
   list_free(list);

   return self;
}

static void objective(CodeNode* self, Bool minimize)
{
   const char* name;
   const Term* term;
   
   assert(code_is_valid(self));

   name = code_eval_child_name(self, 0);
   term = code_eval_child_term(self, 1);

   lps_objname(name);
   lps_setdir(minimize ? LP_MIN : LP_MAX);
   term_to_objective(term);

   code_value_void(self);
}

CodeNode* i_object_min(CodeNode* self)
{
   Trace("i_object_min");

   assert(code_is_valid(self));

   objective(self, TRUE);

   return self;
}

CodeNode* i_object_max(CodeNode* self)
{
   Trace("i_object_max");

   assert(code_is_valid(self));

   objective(self, FALSE);

   return self;
}






