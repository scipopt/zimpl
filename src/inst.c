#pragma ident "@(#) $Id: inst.c,v 1.58 2003/09/09 11:13:30 bzfkocht Exp $"
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

/* #define TRACE  1 */

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "code.h"
#include "inst.h"
#include "xlpglue.h"

/* ----------------------------------------------------------------------------
 * Kontrollfluss Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_nop(CodeNode* self)
{
   Trace("i_nop");

   assert(code_is_valid(self));

   if (code_get_type(self) == CODE_ERR)
      code_value_void(self);

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
      fprintf(stderr, "*** Error 105: Dublicate constraint name \"%s\"\n", name);
      code_errmsg(self);
      exit(EXIT_FAILURE);
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
   Numb*        rhs;
   unsigned int flags;
   int          res;
   
   Trace("i_constraint");
   
   assert(code_is_valid(self));

   term_lhs   = code_eval_child_term(self, 0);
   type       = code_eval_child_contype(self, 1);
   term_rhs   = code_eval_child_term(self, 2);
   flags      = code_eval_child_bits(self, 3);

   rhs        = numb_new_sub(term_get_constant(term_rhs), term_get_constant(term_lhs));
   term       = term_sub_term(term_lhs, term_rhs);

   /* Check if trival infeasible
    */
   if (term_get_elements(term) == 0)
   {
      /* If zero, trival ok, otherwise ...
       */
      res = numb_cmp(rhs, numb_zero());

      assert(type != CON_RANGE);
      assert(type != CON_FREE);
      
      if (  (type == CON_EQUAL && res != 0)
         || (type == CON_LHS   && res >  0)
         || (type == CON_RHS   && res <  0))
      {
         fprintf(stderr, "*** Error 106: Empty LHS, contraint trivally violated\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
   }
   else
   {
      con = xlp_addcon(conname_get(), type, rhs, rhs, flags);

      term_add_constant(term, rhs);
      term_to_nzo(term, con);
   }
   code_value_void(self);

   numb_free(rhs);
   term_free(term);
   
   return self;
}

CodeNode* i_rangeconst(CodeNode* self)
{
   Term*        term;
   Numb*        lhs;
   Numb*        rhs;
   Con*         con;
   unsigned int flags;
   
   Trace("i_rangeconst");
   
   assert(code_is_valid(self));
   
   /* It has to be either l <= x <= u, or u >= x >= l
    */
   if (code_eval_child_contype(self, 3) != code_eval_child_contype(self, 4))
   {
      fprintf(stderr, "*** Error 107: Range must be l <= x <= u, or u >= x >= l\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   
   lhs        = numb_copy(code_eval_child_numb(self, 0));
   term       = term_copy(code_eval_child_term(self, 1));
   rhs        = numb_copy(code_eval_child_numb(self, 2));
   flags      = code_eval_child_bits(self, 5);

   numb_sub(lhs, term_get_constant(term));
   numb_sub(rhs, term_get_constant(term));
      
   /* Check if trival infeasible
    */
   if (term_get_elements(term) == 0)
   {
      /* If zero, trival ok, otherwise ...
       */
      if (numb_cmp(lhs, numb_zero()) > 0 || numb_cmp(rhs, numb_zero()) < 0)
      {
         fprintf(stderr, "*** Error 108: Empty Term with nonempty LHS/RHS, contraint trivally violated\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
   }
   else
   {
      if (numb_cmp(lhs, rhs) > 0)
      {
         fprintf(stderr, "*** Error 109: LHS/RHS contradiction, contraint trivally violated\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
      con = xlp_addcon(conname_get(), CON_RANGE, lhs, rhs, flags);
         
      term_sub_constant(term, term_get_constant(term));
      term_to_nzo(term, con);
   }
   code_value_void(self);

   numb_free(rhs);
   numb_free(lhs);
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
   Trace("i_expr_add");

   assert(code_is_valid(self));

   code_value_numb(self,
      numb_new_add(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_expr_sub(CodeNode* self)
{
   Trace("i_expr_sub");

   assert(code_is_valid(self));

   code_value_numb(self,
      numb_new_sub(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_expr_mul(CodeNode* self)
{
   Trace("i_expr_mul");

   assert(code_is_valid(self));

   code_value_numb(self,
      numb_new_mul(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)));

   return self;
}

CodeNode* i_expr_div(CodeNode* self)
{
   const Numb* divisor;
   
   Trace("i_expr_div");

   assert(code_is_valid(self));

   divisor = code_eval_child_numb(self, 1);

   if (numb_equal(divisor, numb_zero()))
   {
      fprintf(stderr, "*** Error: 110 Division by zero\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }      
   code_value_numb(self,
      numb_new_div(code_eval_child_numb(self, 0), divisor));

   return self;
}

CodeNode* i_expr_mod(CodeNode* self)
{
   const Numb* divisor;
   
   Trace("i_expr_mod");

   assert(code_is_valid(self));

   divisor = code_eval_child_numb(self, 1);

   if (numb_equal(divisor, numb_zero()))
   {
      fprintf(stderr, "*** Error 111: Modulo by zero\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }      
   code_value_numb(self,
      numb_new_mod(code_eval_child_numb(self, 0), divisor));

   return self;
}

CodeNode* i_expr_intdiv(CodeNode* self)
{
   const Numb* divisor;
   
   Trace("i_expr_intdiv");

   assert(code_is_valid(self));

   divisor = code_eval_child_numb(self, 1);

   if (numb_equal(divisor, numb_zero()))
   {
      fprintf(stderr, "*** Error 110: Division by zero\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }      
   code_value_numb(self,
      numb_new_intdiv(
         code_eval_child_numb(self, 0), divisor));

   return self;
}

CodeNode* i_expr_pow(CodeNode* self)
{
   const Numb* expo;
   int         ex;
   
   Trace("i_expr_pow");

   assert(code_is_valid(self));

   expo = code_eval_child_numb(self, 1);

   if (!numb_is_int(expo))
   {
      fprintf(stderr, "*** Error 112: Exponent value ");
      numb_print(stderr, expo);
      fprintf(stderr, " is too big or not an integer\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   ex = numb_toint(expo);

   code_value_numb(self,
      numb_new_pow(code_eval_child_numb(self, 0), ex));

   return self;
}

CodeNode* i_expr_neg(CodeNode* self)
{
   Numb* numb;
   
   Trace("i_expr_neg");

   assert(code_is_valid(self));

   numb = numb_copy(code_eval_child_numb(self, 0));
   numb_neg(numb);
   
   code_value_numb(self, numb);

   return self;
}

CodeNode* i_expr_abs(CodeNode* self)
{
   Numb* numb;
   
   Trace("i_expr_abs");

   assert(code_is_valid(self));

   numb = numb_copy(code_eval_child_numb(self, 0));
   numb_abs(numb);
   
   code_value_numb(self, numb);

   return self;
}

CodeNode* i_expr_floor(CodeNode* self)
{
   Numb* numb;
   
   Trace("i_expr_floor");

   assert(code_is_valid(self));

   numb = numb_copy(code_eval_child_numb(self, 0));
   numb_floor(numb);
   
   code_value_numb(self, numb);

   return self;
}

CodeNode* i_expr_ceil(CodeNode* self)
{
   Numb* numb;
   
   Trace("i_expr_ceil");

   assert(code_is_valid(self));

   numb = numb_copy(code_eval_child_numb(self, 0));
   numb_ceil(numb);
   
   code_value_numb(self, numb);

   return self;
}

CodeNode* i_expr_log(CodeNode* self)
{
   double exponent;
   
   Trace("i_expr_log");

   assert(code_is_valid(self));

#if 1
   fprintf(stderr, "Not yet implemented\n");
#else
   exponent = code_eval_child_numb(self, 0);
   
   if (EQ(exponent, 0.0))
   {
      fprintf(stderr, "*** Error: log of zero\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }      
   code_value_numb(self, log10(exponent));
#endif
   
   return self;
}

CodeNode* i_expr_ln(CodeNode* self)
{
   double exponent;
   
   Trace("i_expr_ln");

   assert(code_is_valid(self));

#if 1
   fprintf(stderr, "Not yet implemented\n");
#else
   exponent = code_eval_child_numb(self, 0);
   
   if (EQ(exponent, 0.0))
   {
      fprintf(stderr, "*** Error: ln of zero\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }      
   code_value_numb(self, log(exponent));
#endif
   
   return self;
}

CodeNode* i_expr_exp(CodeNode* self)
{
   Trace("i_expr_exp");

   assert(code_is_valid(self));

#if 1
   fprintf(stderr, "Not yet implemented\n");
#else
   code_value_numb(self, exp(code_eval_child_numb(self, 0)));
#endif
   
   return self;
}

CodeNode* i_expr_fac(CodeNode* self)
{
   const Numb* fac;
   int         n;
   
   Trace("i_expr_fac");

   assert(code_is_valid(self));

   fac = code_eval_child_numb(self, 0);

   if (!numb_is_int(fac))
   {
      fprintf(stderr, "*** Error 113: Factorial value ");
      numb_print(stderr, fac);
      fprintf(stderr, " is too big or not an integer\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   n = numb_toint(fac);

   if (n < 0)
   {
      fprintf(stderr, "*** Error 114: Negative factorial value\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   if (n > 1000)
   {
      fprintf(stderr, "*** Error 115: Timeout!\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   code_value_numb(self, numb_new_fac(n));

   return self;
}

CodeNode* i_expr_card(CodeNode* self)
{
   const Set* set;
   
   Trace("i_card");

   assert(code_is_valid(self));

   set = code_eval_child_set(self, 0);

   code_value_numb(self, numb_new_integer(set_get_used(set)));

   return self;
}

CodeNode* i_expr_rand(CodeNode* self)
{
   double mini;
   double maxi;
   double val;
   
   Trace("i_rand");

   assert(code_is_valid(self));

#if 1
   fprintf(stderr, "Not yet implemented\n");
#else
   mini = code_eval_child_numb(self, 0);
   maxi = code_eval_child_numb(self, 1);

   val = (double)rand() / (double)RAND_MAX;
   val = val * (maxi - mini) + mini;

   code_value_numb(self, val);
#endif
   
   return self;
}

CodeNode* i_expr_if(CodeNode* self)
{
   Trace("i_if");

   assert(code_is_valid(self));

   if (code_eval_child_bool(self, 0))
      code_copy_value(self, code_eval_child(self, 1));
   else
      code_copy_value(self, code_eval_child(self, 2));

   return self;
}

CodeNode* i_expr_min(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   const Tuple*  tuple;
   CodeNode*     lexpr;
   int           idx   = 0;
   const Numb*   value;
   Numb*         min   = numb_new();
   Bool          first = TRUE;
   
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

         if (first || numb_cmp(value, min) < 0)
         {
            numb_set(min, value);
            first = FALSE;
         }
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
   int           idx   = 0;
   const Numb*   value;
   Numb*         max   = numb_new();
   Bool          first = TRUE;

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

         if (first || numb_cmp(value, max) > 0)
         {
            numb_set(max, value);
            first = FALSE;
         }
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
   Numb*         sum = numb_new();

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
         numb_add(sum, code_eval_child_numb(self, 1));      

      local_drop_frame();
   }
   code_value_numb(self, sum);

   return self;
}

CodeNode* i_expr_min2(CodeNode* self)
{
   const List*   list;
   const Numb*   numb;
   const Elem*   elem;
   Numb*         min;
   ListElem*     le    = NULL;
   int           n;
   Bool          first = TRUE;
   
   Trace("i_expr_min2");
   
   assert(code_is_valid(self));

   list  = code_eval_child_list(self, 0);
   n     = list_get_elems(list);
   min   = numb_new();
   
   while(n-- > 0)
   {
      elem = list_get_elem(list, &le);

      /* Are there only number in the selection tuple ?
       */
      if (ELEM_NUMB != elem_get_type(elem))
      {
         fprintf(stderr, "*** Error 116: Illegal value type in min: ");
         elem_print(stderr, elem);
         fprintf(stderr, " only numbers are possible\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
      numb = elem_get_numb(elem);

      if (first || numb_cmp(min, numb) > 0)
      {
         numb_set(min, numb);
         first = FALSE;
      }
   }
   code_value_numb(self, min);

   return self;
}

CodeNode* i_expr_max2(CodeNode* self)
{
   const List*   list;
   const Numb*   numb;
   const Elem*   elem;
   Numb*         max;
   ListElem*     le    = NULL;
   int           n;
   Bool          first = TRUE;
   
   Trace("i_expr_max2");
   
   assert(code_is_valid(self));

   list  = code_eval_child_list(self, 0);
   n     = list_get_elems(list);
   max   = numb_new();
   
   while(n-- > 0)
   {
      elem = list_get_elem(list, &le);

      /* Are there only number in the selection tuple ?
       */
      if (ELEM_NUMB != elem_get_type(elem))
      {
         fprintf(stderr, "*** Error 117: Illegal value type in max: ");
         elem_print(stderr, elem);
         fprintf(stderr, " only numbers are possible\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
      numb = elem_get_numb(elem);

      if (first || numb_cmp(max, numb) < 0)
      {
         numb_set(max, numb);
         first = FALSE;
      }
   }
   code_value_numb(self, max);

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
   Bool val;
   
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
      fprintf(stderr, "*** Error 118: Comparison of different types\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   assert(tp1 == tp2);

   switch(tp1)
   {
   case CODE_NUMB :
      result = numb_equal(code_get_numb(op1), code_get_numb(op2));
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
      numb_cmp(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)) >= 0);

   return self;
}

CodeNode* i_bool_gt(CodeNode* self)
{
   Trace("i_bool_gt");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      numb_cmp(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)) > 0);

   return self;
}

CodeNode* i_bool_le(CodeNode* self)
{
   Trace("i_bool_le");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      numb_cmp(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)) <= 0);

   return self;
}

CodeNode* i_bool_lt(CodeNode* self)
{
   Trace("i_bool_lt");

   assert(code_is_valid(self));
   
   code_value_bool(self, 
      numb_cmp(code_eval_child_numb(self, 0), code_eval_child_numb(self, 1)) < 0);

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
   int          n     = list_get_elems(list);
   Set*         set   = set_new(dim, n);
   int          i;
   
   Trace("i_set_new_tuple");

   assert(code_is_valid(self));

   /* Und jetzt noch mal alle.
    */
   le  = NULL;

   for(i = 0; i < n; i++)
   {
      tuple = list_get_tuple(list, &le);
      set_add_member(set, tuple_copy(tuple), SET_ADD_END, SET_CHECK_WARN);
   }
   code_value_set(self, set);

   return self;
}

CodeNode* i_set_new_elem(CodeNode* self)
{
   const List* list  = code_eval_child_list(self, 0);
   ListElem*   le    = NULL;
   int         n     = list_get_elems(list);
   Set*        set   = set_new(1, n);
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

      tuple_set_elem(tuple, 0, elem_copy(elem));
      set_add_member(set, tuple, SET_ADD_END, SET_CHECK_WARN);
   }
   code_value_set(self, set);

   return self;
}

CodeNode* i_set_pseudo(CodeNode* self)
{
   Set* set;
   
   Trace("i_set_pseudo");

   assert(code_is_valid(self));

   set = set_new(0, 1);

   set_add_member(set, tuple_new(0), SET_ADD_END, SET_CHECK_NONE);

   code_value_set(self, set);

   return self;
}

CodeNode* i_set_empty(CodeNode* self)
{
   Set* set;
   int  dim;
   
   Trace("i_set_empty");

   assert(code_is_valid(self));

   dim = code_eval_child_size(self, 0);
   set = set_new(dim, 1);

   code_value_set(self, set);

   return self;
}

CodeNode* i_set_union(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   
   Trace("i_set_union");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error 119: Union of incompatible sets\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   code_value_set(self, set_union(set_a, set_b));

   return self;
}

CodeNode* i_set_minus(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   
   Trace("i_set_minus");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error 120: Minus of incompatible sets\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   code_value_set(self, set_minus(set_a, set_b));

   return self;
}

CodeNode* i_set_inter(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   
   Trace("i_set_inter");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error 121: Intersection of incompatible sets\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   code_value_set(self, set_inter(set_a, set_b));

   return self;
}

CodeNode* i_set_sdiff(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   
   Trace("i_set_sdiff");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error 122: Symetric Difference of incompatible sets\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   code_value_set(self, set_sdiff(set_a, set_b));

   return self;
}

CodeNode* i_set_cross(CodeNode* self)
{
   const Set* set_a;
   const Set* set_b;
   
   Trace("i_set_cross");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   code_value_set(self, set_cross(set_a, set_b));

   return self;
}

CodeNode* i_set_range(CodeNode* self)
{
   const Numb* from;
   const Numb* upto;
   const Numb* step;
   int         int_from;
   int         int_upto;
   int         int_step;
   
   Trace("i_set_range");

   assert(code_is_valid(self));

   from = code_eval_child_numb(self, 0);
   upto = code_eval_child_numb(self, 1);
   step = code_eval_child_numb(self, 2);

   if (!numb_is_int(from))
   {
      fprintf(stderr, "*** Error 123: \"from\" value ");
      numb_print(stderr, from);
      fprintf(stderr, " in range too big or not an integer\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   if (!numb_is_int(upto))
   {
      fprintf(stderr, "*** Error 124: \"upto\" value ");
      numb_print(stderr, upto);
      fprintf(stderr, " in range too big or not an integer\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   if (!numb_is_int(step))
   {
      fprintf(stderr, "*** Error 125: \"step\" value ");
      numb_print(stderr, step);
      fprintf(stderr, " in range too big or not an integer\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   int_from = numb_toint(from);
   int_upto = numb_toint(upto);
   int_step = numb_toint(step);
   
   if (Sgn(int_upto - int_from) != Sgn(int_step))
   {
      fprintf(stderr,
         "*** Error 126: \"step\" value %d in range not moving from %d to %d\n",
         int_step, int_from, int_upto);
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   code_value_set(self, set_range(int_from, int_upto, int_step));

   return self;
}

CodeNode* i_set_proj(CodeNode* self)
{
   const Set*   set_a;
   const Tuple* tuple;
   const Elem*  elem;
   const Numb*  numb;
   int          dim;
   int          idx;
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
         fprintf(stderr, "*** Error 127: Illegal value type in tuple: ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " only numbers are possible\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
      numb = elem_get_numb(elem);
      
      if (!numb_is_int(numb))
      {
         fprintf(stderr, "*** Error 128: Index value ");
         numb_print(stderr, numb);
         fprintf(stderr, " in proj too big or not an integer\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
      idx = numb_toint(numb);
      
      /* Are all the number between 1 and dim(set) ?
       */
      if (idx < 1 || idx > dim)
      {
         fprintf(stderr, "*** Error 129: Illegal index %d, ", idx);
         fprintf(stderr, " set has only dimension %d\n", dim);
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
   }
   code_value_set(self, set_proj(set_a, tuple));

   return self;
}

CodeNode* i_set_indexset(CodeNode* self)
{
   const Symbol* sym;
   
   Trace("i_set_indexset");

   assert(code_is_valid(self));

   sym = code_eval_child_symbol(self, 0);

   assert(sym != NULL);

   code_value_set(self, set_copy(symbol_get_iset(sym)));

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
      tuple_set_elem(tuple, i, elem_copy(list_get_elem(list, &le)));

   code_value_tuple(self, tuple);

   return self;
}

CodeNode* i_tuple_empty(CodeNode* self)
{
   Trace("i_tuple_empty");
   
   assert(code_is_valid(self));

   code_value_tuple(self, tuple_new(0));

   return self;
}

/* ----------------------------------------------------------------------------
 * Symbol Funktionen
 * ----------------------------------------------------------------------------
 */
static Set* set_from_idxset(const IdxSet* idxset)
{
   const Tuple*  pattern;
   const Tuple*  tuple;
   Set*          newset;
   int           idx;
   const Set*    set;
   CodeNode*     lexpr;

   assert(idxset != NULL);
   
   set     = idxset_get_set(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   pattern = idxset_get_tuple(idxset);
   newset  = set_new(tuple_get_dim(pattern), set_get_used(set));
   idx     = 0;
      
   while((tuple = set_match_next(set, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
         set_add_member(newset, tuple_copy(tuple), SET_ADD_END, SET_CHECK_WARN);

      local_drop_frame();
   }
   return newset;
}

CodeNode* i_newsym_set1(CodeNode* self)
{
   const char*   name;
   const IdxSet* idxset;
   Set*          iset;
   Symbol*       sym;

   const Tuple*  pattern;
   const Tuple*  tuple;
   int           idx;
   
   Trace("i_newsym_set1");

   name    = code_eval_child_name(self, 0);
   idxset  = code_eval_child_idxset(self, 1);
   iset    = set_from_idxset(idxset);
   sym     = symbol_new(name, SYM_SET, iset, set_get_used(iset), NULL);

   assert(code_is_valid(self));

   pattern = idxset_get_tuple(idxset);
   idx     = 0;
      
   while((tuple = set_match_next(iset, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      symbol_add_entry(sym,
         entry_new_set(tuple,
            code_eval_child_set(self, 2)));

      local_drop_frame();
   }
   set_free(iset);

   code_value_void(self);

   return self;
}

static Set* iset_from_list(const CodeNode* self, const List* list)
{
   const Entry* entry;
   const Tuple* tuple;
   ListElem*    lelem = NULL;
   Set*         set;

   assert(self                 != NULL);
   assert(list                 != NULL);
   assert(list_get_elems(list) > 0);
   
   entry  = list_get_entry(list, &lelem);
   tuple  = entry_get_tuple(entry);
   set    = set_new(tuple_get_dim(tuple), list_get_elems(list));

   assert(set != NULL);

   for(;;)
   {
      set_add_member(set, tuple_copy(tuple), SET_ADD_END, SET_CHECK_NONE);
      
      entry = list_get_entry(list, &lelem);

      if (entry == NULL)
         break;
      
      tuple  = entry_get_tuple(entry);

      if (set_lookup(set, tuple))
      {
         fprintf(stderr, "*** Error 130: Duplicate index ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " for initialisation\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
   }
   return set;
}
   
CodeNode* i_newsym_set2(CodeNode* self)
{
   const char*   name;
   const IdxSet* idxset;
   const Set*    iset;
   Symbol*       sym;
   const List*   list;
   ListElem*     lelem;
   const Entry*  entry;
   const Tuple*  tuple;
   int           count;
   int           i;
   
   Trace("i_newsym_set2");

   assert(code_is_valid(self));

   name   = code_eval_child_name(self, 0);
   idxset = code_eval_child_idxset(self, 1);
   iset   = idxset_get_set(idxset);
   list   = code_eval_child_list(self, 2);
   count  = list_get_elems(list);

   assert(list_is_entrylist(list));

   /* Empty set ?
    */
   if (set_get_dim(iset) > 0)
      sym  = symbol_new(name, SYM_SET, iset, count, NULL);
   else
   {
      Set* set;
      
      set  = iset_from_list(code_get_child(self, 2), list);
      sym  = symbol_new(name, SYM_SET, set, count, NULL);
      iset = symbol_get_iset(sym);
      set_free(set);
   }

   lelem = NULL;
   
   for(i = 0; i < count; i++)
   {
      entry  = list_get_entry(list, &lelem);
      tuple  = entry_get_tuple(entry);
      
      if (set_lookup(iset, tuple))
         symbol_add_entry(sym, entry_copy(entry));
      else
      {
         fprintf(stderr, "*** Error 131: Illegal element ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " for symbol\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
   }
   code_value_void(self);

   return self;
}

/* initialisation per list
 */
CodeNode* i_newsym_para1(CodeNode* self)
{
   const char*   name;
   Set*          iset;
   const IdxSet* idxset;
   Symbol*       sym;
   const List*   list;
   ListElem*     lelem;
   const Entry*  entry;
   const Tuple*  tuple;
   CodeNode*     child3;
   const Entry*  deflt;
   int           count;
   int           i;
   
   Trace("i_newsym_para1");

   assert(code_is_valid(self));

   name   = code_eval_child_name(self, 0);
   idxset = code_eval_child_idxset(self, 1);
   iset   = set_from_idxset(idxset);
   list   = code_eval_child_list(self, 2);
   child3 = code_eval_child(self, 3);

   if (code_get_type(child3) == CODE_VOID)
      deflt = ENTRY_NULL;
   else
      deflt = code_get_entry(code_eval(child3));
         
   if (!list_is_entrylist(list))
   {
      /* This errors occurs, if the parameter is mssing in the template
       * for a "read" statement.
       */
      assert(list_is_tuplelist(list));
      
      fprintf(stderr, "*** Error 132: Values in parameter list missing,\n");
      fprintf(stderr, "               probably wrong read template\n");      
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   
   /* First element will determine the type (see SYM_ERR below)
    */
   count = list_get_elems(list);

   /* I found no way to make the following error happen.
    * You will get either an error 157 or an parse error.
    * In case there is a way a parse error with
    * message "Symbol xxx not initialised" will result.
    * In this case the code below should be reactivated.
    */
   assert(count > 0);
#if 0
   /* So if there is no first element, we are in trouble.
    */
   if (count == 0)
   {
      fprintf(stderr, "*** Error xxx: Empty initialisation for parameter \"%s\"\n",
         name);
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
#endif
   sym   = symbol_new(name, SYM_ERR, iset, count, deflt);
   lelem = NULL;
   
   for(i = 0; i < count; i++)
   {
      entry  = list_get_entry(list, &lelem);
      tuple  = entry_get_tuple(entry);
      
      if (set_lookup(iset, tuple))
         symbol_add_entry(sym, entry_copy(entry));
      else
      {
         fprintf(stderr, "*** Error 134: Illegal element ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " for symbol\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
   }
   code_value_void(self);

   set_free(iset);

   return self;
}

/* initialisation per element
 */
CodeNode* i_newsym_para2(CodeNode* self)
{
   const char*   name;
   Set*          iset;
   const IdxSet* idxset;
   Symbol*       sym;
   Entry*        entry;
   const Entry*  deflt;
   CodeNode*     child3;
   CodeNode*     child;
   const Tuple*  tuple;
   const Tuple*  pattern;
   int           idx = 0;
   
   Trace("i_newsym_para2");

   assert(code_is_valid(self));

   name    = code_eval_child_name(self, 0);
   idxset  = code_eval_child_idxset(self, 1);
   iset    = set_from_idxset(idxset);
   child3  = code_eval_child(self, 3);

   if (set_get_used(iset) == 0)
   {
      fprintf(stderr, "*** Error 135: Index set for parameter \"%s\" is empty\n",
         name);
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   
   if (code_get_type(child3) == CODE_VOID)
      deflt = ENTRY_NULL;
   else
      deflt = code_get_entry(code_eval(child3));
   
   sym     = symbol_new(name, SYM_ERR, iset, set_get_used(iset), deflt);
   pattern = idxset_get_tuple(idxset);

   while((tuple = set_match_next(iset, pattern, &idx)) != NULL)
   {
      /* bool is not needed, because iset has only true elemens
       */
      local_install_tuple(pattern, tuple);

      child = code_eval_child(self, 2);

      switch(code_get_type(child))
      {
      case CODE_NUMB:
         entry = entry_new_numb(tuple, code_get_numb(child));
         break;
      case CODE_STRG :
         entry = entry_new_strg(tuple, code_get_strg(child));
         break;
      case CODE_NAME :
         fprintf(stderr, "*** Error 133: Unknown local symbol \"%s\"\n",
            code_get_name(child));
         code_errmsg(self);
         exit(EXIT_FAILURE);
      default :
         abort();
      }
      symbol_add_entry(sym, entry);
      
      local_drop_frame();
   }
   code_value_void(self);

   set_free(iset);

   return self;
}

CodeNode* i_newsym_var(CodeNode* self)
{
   const char*   name;
   const IdxSet* idxset;
   Set*          iset;
   Symbol*       sym;
   const Tuple*  tuple;
   const Tuple*  pattern;
   VarClass      varclass;
   VarClass      usevarclass;
   Var*          var;
   int           idx = 0;
   Bound*        lower;
   Bound*        upper;
   const Numb*   priority;
   const Numb*   startval;
   char*         tuplestr;
   char*         varname;
   Numb*         temp;
   
   Trace("i_newsym_var");

   assert(code_is_valid(self));

   name     = code_eval_child_name(self, 0);
   idxset   = code_eval_child_idxset(self, 1);
   varclass = code_eval_child_varclass(self, 2);
   iset     = set_from_idxset(idxset);
   pattern  = idxset_get_tuple(idxset);
   sym      = symbol_new(name, SYM_VAR, iset, set_get_used(iset), NULL);

   while((tuple = set_match_next(iset, pattern, &idx)) != NULL)
   {
      local_install_tuple(pattern, tuple);
      
      lower       = bound_copy(code_eval_child_bound(self, 3));
      upper       = bound_copy(code_eval_child_bound(self, 4));
      priority    = code_eval_child_numb(self, 5);
      startval    = code_eval_child_numb(self, 6);
      usevarclass = varclass;

      /* If it is a binary variable the bounds have to be 0 and 1.
       */
      assert(varclass != VAR_BIN || bound_get_type(lower) == BOUND_VALUE);
      assert(varclass != VAR_BIN || bound_get_type(upper) == BOUND_VALUE);
      assert(varclass != VAR_BIN || numb_equal(bound_get_value(lower), numb_zero()));
      assert(varclass != VAR_BIN || numb_equal(bound_get_value(upper), numb_one()));

      if (bound_get_type(lower) == BOUND_INFTY)
      {
         bound_free(lower);
         lower = bound_new(BOUND_VALUE, numb_zero());

         fprintf(stderr,
            "--- Warning 136: Lower bound for var %s set to infinity -- ignored\n",
            name);
      }
      if (bound_get_type(upper) == BOUND_MINUS_INFTY)
      {
         bound_free(upper);
         upper = bound_new(BOUND_INFTY, NULL);

         fprintf(stderr,
            "--- Warning 137: Upper bound for var %s set to -infinity -- ignored\n",
            name);
      }

      if ((varclass == VAR_CON)
         && (!numb_equal(priority, numb_zero()) || !numb_equal(startval, numb_zero())))
         fprintf(stderr,
            "--- Warning 138: Priority/Startval for continous var %s ignored\n",
            name);
      
      /* Integral bounds for integral variables ?
       */
      if (varclass != VAR_CON)
      {
         if (bound_get_type(lower) == BOUND_VALUE)
         {
            temp = numb_copy(bound_get_value(lower));
            numb_ceil(temp);
      
            if (!numb_equal(temp, bound_get_value(lower)))
            {
               bound_free(lower);
               lower = bound_new(BOUND_VALUE, temp);
               
               fprintf(stderr,
                  "--- Warning 139: Lower bound for integral var %s truncated to ", name);
               numb_print(stderr, temp);
               fputc('\n', stderr);
            }
            numb_free(temp);
         }
         if (bound_get_type(upper) == BOUND_VALUE)
         {
            temp = numb_copy(bound_get_value(upper));
            numb_floor(temp);
      
            if (!numb_equal(temp, bound_get_value(upper)))
            {
               bound_free(upper);
               upper = bound_new(BOUND_VALUE, temp);
               
               fprintf(stderr,
                  "--- Warning 140: Upper bound for integral var %s truncated to ", name);
               numb_print(stderr, temp);
               fputc('\n', stderr);
            }
            numb_free(temp);
         }
      }
      if (varclass == VAR_INT
         && bound_get_type(lower) == BOUND_VALUE
         && bound_get_type(upper) == BOUND_VALUE
         && numb_equal(bound_get_value(lower), numb_zero())
         && numb_equal(bound_get_value(upper), numb_one()))
      {
         usevarclass = VAR_BIN;
      }

      if (  bound_get_type(lower) == BOUND_VALUE
         && bound_get_type(upper) == BOUND_VALUE
         && numb_cmp(bound_get_value(lower), bound_get_value(upper)) > 0)
      {
         fprintf(stderr, "*** Error 141: Infeasible due to conflicting bounds for var %s\n",
            name);
         fprintf(stderr, "               lower=%g > upper=%g\n",
            numb_todbl(bound_get_value(lower)),
            numb_todbl(bound_get_value(upper)));
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }

      /* Hier geben wir der Variable einen eindeutigen Namen
       */
      tuplestr = tuple_tostr(tuple);
      varname  = malloc(strlen(name) + strlen(tuplestr) + 2);

      assert(varname != NULL);
      
      sprintf(varname, "%s%s", name, tuplestr);

      /* Und nun legen wir sie an.
       */
      var = xlp_addvar(varname, usevarclass, lower, upper, priority, startval);

      symbol_add_entry(sym, entry_new_var(tuple, var));

      free(varname);
      free(tuplestr);
      
      local_drop_frame();

      bound_free(lower);
      bound_free(upper);
   }
   code_value_void(self);

   set_free(iset);

   return self;
}

CodeNode* i_symbol_deref(CodeNode* self)
{
   const Tuple* tuple;
   const Entry* entry;
   Symbol*      sym;
   Term*        term;
   
   Trace("i_symbol_deref");
   
   assert(code_is_valid(self));

   sym   = code_eval_child_symbol(self, 0);
   tuple = code_eval_child_tuple(self, 1);   

   /* wurde schon in mmlscan ueberprueft
    */
   assert(sym != NULL);

   entry = symbol_lookup_entry(sym, tuple);

   if (NULL == entry)
   {
      fprintf(stderr, "*** Error 142: Unknown index ");
      tuple_print(stderr, tuple);
      fprintf(stderr, " for symbol \"%s\"\n", symbol_get_name(sym));
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }

   switch(symbol_get_type(sym))
   {
   case SYM_NUMB :
      code_value_numb(self, numb_copy(entry_get_numb(entry)));
      break;
   case SYM_STRG :
      code_value_strg(self, entry_get_strg(entry));
      break;
   case SYM_SET :
      code_value_set(self, set_copy(entry_get_set(entry)));
      break;
   case SYM_VAR :
      term = term_new(1);
      term_add_elem(term, entry, numb_one());
      code_value_term(self, term);
      break;
   default :
      abort();
   }
   return self;
}

/* ----------------------------------------------------------------------------
 * Index Set Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_set_idxset(CodeNode* self)
{
   const IdxSet* idxset;

   Trace("i_set_idxset");

   idxset = code_eval_child_idxset(self, 0);
   
   code_value_set(self, set_from_idxset(idxset));

   return self;
}

CodeNode* i_idxset_new(CodeNode* self)
{
   Tuple*       tuple;
   Tuple*       t0;
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
         tuple_set_elem(tuple, i, elem_new_name(str_new(name)));
      }
   }
   code_value_idxset(self, idxset_new(tuple, set, code_get_child(self, 2)));

   tuple_free(t0);
   tuple_free(tuple);

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
         code_value_numb(self, numb_copy(elem_get_numb(elem)));
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
   Term*       term;
   const Numb* coeff;
   
   Trace("i_term_coeff");
   
   assert(code_is_valid(self));

   term  = term_copy(code_eval_child_term(self, 0));
   coeff = code_eval_child_numb(self, 1);

   term_mul_coeff(term, coeff);
   
   code_value_term(self, term);

   return self;
}

CodeNode* i_term_const(CodeNode* self)
{
   Term*       term;
   const Numb* numb;
   
   Trace("i_term_const");
   
   assert(code_is_valid(self));

   term = term_copy(code_eval_child_term(self, 0));
   numb = code_eval_child_numb(self, 1);

   term_add_constant(term, numb);
   
   code_value_term(self, term);

   return self;
}

CodeNode* i_term_add(CodeNode* self)
{
   const Term* term_a;
   const Term* term_b;

   Trace("i_term_add");
   
   assert(code_is_valid(self));

   term_a = code_eval_child_term(self, 0);
   term_b = code_eval_child_term(self, 1);
   
   code_value_term(self, term_add_term(term_a, term_b));

   return self;
}

CodeNode* i_term_sub(CodeNode* self)
{
   const Term* term_a;
   const Term* term_b;
   
   Trace("i_term_sub");
   
   assert(code_is_valid(self));

   term_a = code_eval_child_term(self, 0);
   term_b = code_eval_child_term(self, 1);
   
   code_value_term(self, term_sub_term(term_a, term_b));

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
   Term*         term_r;

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
         term_append_term(term_r, code_eval_child_term(self, 1));

      local_drop_frame();
   }
   code_value_term(self, term_r);

   return self;
}

CodeNode* i_term_expr(CodeNode* self)
{
   Term*       term;
   const Numb* numb;
   
   Trace("i_term_expr");
   
   assert(code_is_valid(self));

   term  = term_new(1);
   numb  = code_eval_child_numb(self, 0);

   term_add_constant(term, numb);
   
   code_value_term(self, term);

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
   case CODE_SET :
      entry = entry_new_set(tuple, code_get_set(child));
      break;
   case CODE_NAME :
      fprintf(stderr, "*** Error 133: Unknown local symbol \"%s\"\n",
         code_get_name(child));
      code_errmsg(self);
      exit(EXIT_FAILURE);
   default :
      abort();
   }
   code_value_entry(self, entry);

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
   code_value_list(self, list_new_elem(elem));

   elem_free(elem);

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

   return self;
}

CodeNode* i_tuple_list_new(CodeNode* self)
{
   Trace("i_tuple_list_new");

   assert(code_is_valid(self));

   code_value_list(self,
      list_new_tuple(
         code_eval_child_tuple(self, 0)));

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

   return self;
}

CodeNode* i_entry_list_new(CodeNode* self)
{
   Trace("i_entry_list_new");

   assert(code_is_valid(self));

   code_value_list(self,
      list_new_entry(
         code_eval_child_entry(self, 0)));

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
   
   return self;
}

CodeNode* i_entry_list_subsets(CodeNode* self)
{
   const Set*  set;
   const Numb* numb;
   int         subset_size;
   int         idx  = 0;
   int         used;
   
   Trace("i_entry_list_subsets");

   assert(code_is_valid(self));

   set         = code_eval_child_set(self, 0);
   used        = set_get_used(set);
   numb        = code_eval_child_numb(self, 1);

   if (!numb_is_int(numb))
   {
      fprintf(stderr, "*** Error 143: Size for subsets ");
      numb_print(stderr, numb);
      fprintf(stderr, " is too big or not an integer\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   subset_size = numb_toint(numb);
   
   if (used < 1)
   {
      fprintf(stderr, "*** Error 144: Tried to build subsets of empty set\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   assert(set_get_dim(set) > 0);
   
   if ((subset_size < 1) || (subset_size > used))
   {
      fprintf(stderr, "*** Error 145: Illegal size for subsets %d,\n",
         subset_size);
      fprintf(stderr, "               should be between 1 and %d\n",
         set_get_used(set));
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   code_value_list(self, set_subsets_list(set, subset_size, NULL, &idx));
   
   return self;
}

CodeNode* i_entry_list_powerset(CodeNode* self)
{
   const Set* set;
   List*      list = NULL;
   int        idx  = 0;
   int        i;
   int        used;

   Trace("i_entry_list_powerset");

   assert(code_is_valid(self));

   set  = code_eval_child_set(self, 0);
   used = set_get_used(set);
   
   if (used < 1)
   {
      fprintf(stderr, "*** Error 146: Tried to build powerset of empty set\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   assert(set_get_dim(set) > 0);

   for(i = 1; i <= used; i++)
      list = set_subsets_list(set, i, list, &idx);

   assert(list != NULL);
   
   code_value_list(self, list);
   
   return self;
}

static void objective(CodeNode* self, Bool minimize)
{
   const char* name;
   const Term* term;
   
   assert(code_is_valid(self));

   name = code_eval_child_name(self, 0);
   term = code_eval_child_term(self, 1);

   xlp_objname(name);
   xlp_setdir(minimize);
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

CodeNode* i_print(CodeNode* self)
{
   CodeNode*     child;
   const Symbol* sym;
   const char*   name;

   Trace("i_print");

   assert(code_is_valid(self));

   child = code_eval(code_get_child(self, 0));

   switch(code_get_type(child))
   {
   case CODE_NUMB :
      printf("%.16g", numb_todbl(code_get_numb(child)));
      break;
   case CODE_STRG :
      printf("\"%s\"", code_get_strg(child));
      break;
   case CODE_TUPLE :
      tuple_print(stdout, code_get_tuple(child));
      break;
   case CODE_SET :
      set_print(stdout, code_get_set(child));
      break;
   case CODE_NAME :
      name = code_get_name(child);

      if (NULL != (sym = symbol_lookup(name)))
         symbol_print(stdout, sym);
      else
         printf("%s", name);
      break;
   default :
      abort();
   }
   fputc('\n', stdout);
   
   code_value_void(self);

   return self;
}

CodeNode* i_bound_new(CodeNode* self)
{
   Trace("i_bound_new");

   assert(code_is_valid(self));

   code_value_bound(self,
      bound_new(BOUND_VALUE,
         code_eval_child_numb(self, 0)));

   return self;
}




