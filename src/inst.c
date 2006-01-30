#pragma ident "@(#) $Id: inst.c,v 1.97 2006/01/30 11:19:42 bzfkocht Exp $"
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
      fprintf(stderr, "*** Error 105: Duplicate constraint name \"%s\"\n", name);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
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
         fprintf(stderr, "*** Error 106: Empty LHS, constraint trivially violated\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   else
   {
      con = xlp_addcon(conname_get(), type, rhs, rhs, flags);

      term_add_constant(term, rhs);
      term_to_nzo(term, con);
      conname_next();
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
      zpl_exit(EXIT_FAILURE);
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
         fprintf(stderr,
            "*** Error 108: Empty Term with nonempty LHS/RHS, constraint trivially violated\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   else
   {
      if (numb_cmp(lhs, rhs) > 0)
      {
         fprintf(stderr, "*** Error 109: LHS/RHS contradiction, constraint trivially violated\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      con = xlp_addcon(conname_get(), CON_RANGE, lhs, rhs, flags);
         
      term_sub_constant(term, term_get_constant(term));
      term_to_nzo(term, con);
      conname_next();
   }
   code_value_void(self);

   numb_free(rhs);
   numb_free(lhs);
   term_free(term);
   
   return self;
}

CodeNode* i_sos(CodeNode* self)
{
   const char* name;
   
   Trace("i_sos");
   
   assert(code_is_valid(self));

   name = code_eval_child_name(self, 0);

   if (!conname_set(name))
   {
      fprintf(stderr, "*** Error 105: Duplicate constraint name \"%s\"\n", name);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   (void)code_eval_child(self, 1); /* soset */

   conname_free();
   
   code_value_void(self);

   return self;
}

CodeNode* i_soset(CodeNode* self)
{
   const Term*  term;
   const Numb*  typenumb;
   const Numb*  priority;
   Sos*         sos;
   SosType      type;
   
   Trace("i_constraint");
   
   assert(code_is_valid(self));

   term       = code_eval_child_term(self, 0);
   typenumb   = code_eval_child_numb(self, 1);
   priority   = code_eval_child_numb(self, 2);

   if (!numb_equal(term_get_constant(term), numb_zero()))
   {
      fprintf(stderr, "*** Error 199: Constants are not allowed in SOS declarations\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   if (numb_equal(typenumb, numb_one()))
      type = SOS_TYPE1;
   else
      type = SOS_TYPE2;
   
   sos = xlp_addsos(conname_get(), type, priority);

   assert(sos != NULL);

   if (term_to_sos(term, sos))
   {
      if (verbose > VERB_QUIET)
         fprintf(stderr,
            "--- Warning 200: Weights are not unique for SOS %s\n", conname_get());
      code_errmsg(self);
   }
   conname_next();

   code_value_void(self);

   return self;
}

/* lint */
static void warn_if_pattern_has_no_name(
   const CodeNode* self,
   const Tuple*    pattern)
{
   if (verbose > VERB_QUIET)
   {
      int dim = tuple_get_dim(pattern);
      int i;

      for(i = 0; i < dim; i++)
         if (ELEM_NAME == elem_get_type(tuple_get_elem(pattern, i)))
            break;

      if (i == dim && dim > 0)
      {
         fprintf(stderr, "--- Warning 203: Indexing tuple is fixed\n");
         code_errmsg(self);
      }
   }
}

CodeNode* i_forall(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   Tuple*        tuple;
   CodeNode*     lexpr;
   SetIter*      iter;
   
   Trace("i_forall");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);  
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(set, pattern);

   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);
   
   while((tuple = set_iter_next(iter, set)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
         (void)code_eval_child(self, 1); /* z.B. constraint */

      local_drop_frame();

      tuple_free(tuple);
   }
   set_iter_exit(iter, set);
   
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
      zpl_exit(EXIT_FAILURE);
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
      zpl_exit(EXIT_FAILURE);
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
      zpl_exit(EXIT_FAILURE);
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
      zpl_exit(EXIT_FAILURE);
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

CodeNode* i_expr_sgn(CodeNode* self)
{
   Numb* numb;
   
   Trace("i_expr_sgn");

   assert(code_is_valid(self));

   numb = numb_copy(code_eval_child_numb(self, 0));
   numb_sgn(numb);
   
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
   Numb* numb;
   
   Trace("i_expr_log");

   assert(code_is_valid(self));

   numb = numb_new_log(code_eval_child_numb(self, 0));
   
   if (numb == NULL)
   {
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }      
   code_value_numb(self, numb);
   
   return self;
}

CodeNode* i_expr_ln(CodeNode* self)
{
   Numb* numb;
   
   Trace("i_expr_ln");

   assert(code_is_valid(self));

   numb = numb_new_ln(code_eval_child_numb(self, 0));
   
   if (numb == NULL)
   {
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }      
   code_value_numb(self, numb);
   
   return self;
}

CodeNode* i_expr_sqrt(CodeNode* self)
{
   Numb* numb;
   
   Trace("i_expr_sqrt");

   assert(code_is_valid(self));

   numb = numb_new_sqrt(code_eval_child_numb(self, 0));
   
   if (numb == NULL)
   {
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }      
   code_value_numb(self, numb);
   
   return self;
}

CodeNode* i_expr_exp(CodeNode* self)
{
   Trace("i_expr_exp");

   assert(code_is_valid(self));

   code_value_numb(self, numb_new_exp(code_eval_child_numb(self, 0)));
   
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
      zpl_exit(EXIT_FAILURE);
   }
   n = numb_toint(fac);

   if (n < 0)
   {
      fprintf(stderr, "*** Error 114: Negative factorial value\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   if (n > 1000)
   {
      fprintf(stderr, "*** Error 115: Timeout!\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
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

   code_value_numb(self, numb_new_integer(set_get_members(set)));

   return self;
}

CodeNode* i_expr_rand(CodeNode* self)
{
   const Numb* mini;
   const Numb* maxi;
   
   Trace("i_rand");

   assert(code_is_valid(self));

   mini = code_eval_child_numb(self, 0);
   maxi = code_eval_child_numb(self, 1);

   if (numb_cmp(mini, maxi) >= 0)
   {
      fprintf(stderr, "*** Error 204: Randomfunction parameter minimum= ");
      numb_print(stderr, mini);
      fprintf(stderr, " >= maximum= ");
      numb_print(stderr, maxi);
      fprintf(stderr, "\n");
      code_errmsg(code_get_child(self, 0));
      zpl_exit(EXIT_FAILURE);
   }
   code_value_numb(self, numb_new_rand(mini, maxi));
   
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
   Tuple*        tuple;
   CodeNode*     lexpr;
   SetIter*      iter;
   const Numb*   value;
   Numb*         min   = numb_new();
   Bool          first = TRUE;
   
   Trace("i_expr_min");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(set, pattern);

   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);

   while((tuple = set_iter_next(iter, set)) != NULL)
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

      tuple_free(tuple);
   }
   set_iter_exit(iter, set);
   
   if (first)
   {
      if (verbose > VERB_QUIET)
      {
         fprintf(stderr, "--- Warning 186: Minimizing over empty set -- zero assumed\n");
         code_errmsg(code_get_child(self, 0));
      }
   }
   code_value_numb(self, min);

   return self;
}

CodeNode* i_expr_max(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   Tuple*        tuple;
   CodeNode*     lexpr;
   SetIter*      iter;
   const Numb*   value;
   Numb*         max   = numb_new();
   Bool          first = TRUE;

   Trace("i_expr_max");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(set, pattern);

   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);
   
   while((tuple = set_iter_next(iter, set)) != NULL)
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

      tuple_free(tuple);
   }
   set_iter_exit(iter, set);
   
   if (first)
   {
      if (verbose > VERB_QUIET)
      {
         fprintf(stderr, "--- Warning 187: Maximizing over empty set -- zero assumed\n");
         code_errmsg(code_get_child(self, 0));
      }
   }
   code_value_numb(self, max);

   return self;
}

CodeNode* i_expr_sum(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   Tuple*        tuple;
   CodeNode*     lexpr;
   SetIter*      iter;
   Numb*         sum = numb_new();

   Trace("i_expr_sum");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(set, pattern);
   
   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);

   while((tuple = set_iter_next(iter, set)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
         numb_add(sum, code_eval_child_numb(self, 1));      

      local_drop_frame();

      tuple_free(tuple);
   }
   set_iter_exit(iter, set);

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

   assert(n > 0);

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
         zpl_exit(EXIT_FAILURE);
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

   assert(n > 0);
   
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
         zpl_exit(EXIT_FAILURE);
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

CodeNode* i_expr_ord(CodeNode* self)
{
   const Set*  set;
   const Elem* elem;
   const Numb* tuple_no;
   const Numb* compo_no;
   Tuple*      tuple;
   int         tno;
   int         cno;
   
   Trace("i_expr_ord");

   assert(code_is_valid(self));

   set      = code_eval_child_set(self, 0);
   tuple_no = code_eval_child_numb(self, 1);
   compo_no = code_eval_child_numb(self, 2);

   if (!numb_is_int(tuple_no))
   {
      fprintf(stderr, "*** Error 189: Tuple number ");
      numb_print(stderr, tuple_no);
      fprintf(stderr, " is too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   if (!numb_is_int(compo_no))
   {
      fprintf(stderr, "*** Error 190: Component number ");
      numb_print(stderr, compo_no);
      fprintf(stderr, " is too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   tno = numb_toint(tuple_no);
   cno = numb_toint(compo_no);

   if (tno < 1 || tno > set_get_members(set))
   {
      fprintf(stderr, "*** Error 191: Tuple number %d", tno);
      fprintf(stderr, " is not a valid value between 1..%d\n", set_get_members(set));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   if (cno < 1 || cno > set_get_dim(set))
   {
      fprintf(stderr, "*** Error 192: Component number %d", cno);
      fprintf(stderr, " is not a valid value between 1..%d\n", set_get_dim(set));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   tuple = set_get_tuple(set, tno - 1);
   elem  = tuple_get_elem(tuple, cno - 1);
   
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
   tuple_free(tuple);

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

CodeNode* i_bool_xor(CodeNode* self)
{
   Bool a;
   Bool b;
   
   Trace("i_bool_or");

   assert(code_is_valid(self));

   a = code_eval_child_bool(self, 0);
   b = code_eval_child_bool(self, 1);

   code_value_bool(self, (a && !b) || (b && !a));

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
      zpl_exit(EXIT_FAILURE);
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
   case CODE_NAME :
      fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
         code_get_name(op1));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
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
   CodeNode* op1;
   CodeNode* op2;
   CodeType  tp1;
   CodeType  tp2;
   Bool      result;
   
   Trace("i_bool_ge");

   assert(code_is_valid(self));

   op1 = code_eval_child(self, 0);
   op2 = code_eval_child(self, 1);
   tp1 = code_get_type(op1);
   tp2 = code_get_type(op2);

   if (tp1 != tp2)
   {
      fprintf(stderr, "*** Error 118: Comparison of different types\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   assert(tp1 == tp2);

   switch(tp1)
   {
   case CODE_NUMB :
      result = numb_cmp(code_get_numb(op1), code_get_numb(op2)) >= 0;
      break;
   case CODE_STRG :
      result = strcmp(code_get_strg(op1), code_get_strg(op2)) >= 0;
      break;
   case CODE_NAME :
      fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
         code_get_name(op1));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   default :
      abort();
   }
   code_value_bool(self, result);

   return self;
}

CodeNode* i_bool_gt(CodeNode* self)
{
   CodeNode* op1;
   CodeNode* op2;
   CodeType  tp1;
   CodeType  tp2;
   Bool      result;
   
   Trace("i_bool_gt");

   assert(code_is_valid(self));

   op1 = code_eval_child(self, 0);
   op2 = code_eval_child(self, 1);
   tp1 = code_get_type(op1);
   tp2 = code_get_type(op2);

   if (tp1 != tp2)
   {
      fprintf(stderr, "*** Error 118: Comparison of different types\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   assert(tp1 == tp2);

   switch(tp1)
   {
   case CODE_NUMB :
      result = numb_cmp(code_get_numb(op1), code_get_numb(op2)) > 0;
      break;
   case CODE_STRG :
      result = strcmp(code_get_strg(op1), code_get_strg(op2)) > 0;
      break;
   case CODE_NAME :
      fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
         code_get_name(op1));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   default :
      abort();
   }
   code_value_bool(self, result);

   return self;
}

CodeNode* i_bool_le(CodeNode* self)
{
   Trace("i_bool_le");

   assert(code_is_valid(self));
   
   code_value_bool(self, !code_get_bool(i_bool_gt(self)));

   return self;
}

CodeNode* i_bool_lt(CodeNode* self)
{
   Trace("i_bool_lt");

   assert(code_is_valid(self));
   
   code_value_bool(self, !code_get_bool(i_bool_ge(self)));

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

static void check_tuple_set_compatible(
   const CodeNode* self,
   const Tuple*    tuple_a,
   const Set*      set_b)
{
   Tuple*      tuple_b;
   ElemType    elem_type_a;
   ElemType    elem_type_b;
   int         i;
   int         dim;
   
   /* An empty set is compatible with any tuple.
    */
   if (set_get_members(set_b) == 0)
      return;
   
   dim = set_get_dim(set_b);

   if (tuple_get_dim(tuple_a) != dim)
   {
      fprintf(stderr, "*** Error 188: Index tuple has wrong dimension\n");
      tuple_print(stderr, tuple_a);
      fprintf(stderr, " should have dimension %d\n", dim);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   tuple_b = set_get_tuple(set_b, 0);

   assert(tuple_get_dim(tuple_a) == tuple_get_dim(tuple_b));

   for(i = 0; i < tuple_get_dim(tuple_a); i++)
   {
      elem_type_a = elem_get_type(tuple_get_elem(tuple_a, i));
      elem_type_b = elem_get_type(tuple_get_elem(tuple_b, i));

      assert(elem_type_b == ELEM_NUMB || elem_type_b == ELEM_STRG);

      if (elem_type_a != elem_type_b)
      {
         fprintf(stderr, "*** Error 198: Incompatible index tuple\nTuple ");
         tuple_print(stderr, tuple_a);
         fprintf(stderr, " component %d is not compatible with ", i + 1);
         tuple_print(stderr, tuple_b);
         fprintf(stderr, "\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   tuple_free(tuple_b);
}

CodeNode* i_bool_is_elem(CodeNode* self)
{
   const Tuple* tuple;
   const Set*   set;
   
   Trace("i_bool_is_elem");

   assert(code_is_valid(self));

   tuple = code_eval_child_tuple(self, 0);
   set   = code_eval_child_set(self, 1);

   check_tuple_set_compatible(self, tuple, set);
   
   code_value_bool(self, set_lookup(set, tuple));

   return self;
}

CodeNode* i_bool_exists(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   Tuple*        tuple;
   CodeNode*     lexpr;
   SetIter*      iter;
   Bool          exists = FALSE;

   Trace("i_bool_exists");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(set, pattern);
   
   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);

   while(!exists && (tuple = set_iter_next(iter, set)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      exists = code_get_bool(code_eval(lexpr));

      local_drop_frame();

      tuple_free(tuple);
   }
   set_iter_exit(iter, set);

   code_value_bool(self, exists);

   return self;
}

/* ----------------------------------------------------------------------------
 * Set Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* i_set_new_tuple(CodeNode* self)
{
   const List*  list;
   const Tuple* tuple;
   ListElem*    le    = NULL;
   ElemType     elem_type;
   int          dim;
   int          i;
   
   Trace("i_set_new_tuple");
   
   assert(code_is_valid(self));

   list  = code_eval_child_list(self, 0);
   tuple = list_get_tuple(list, &le);

   assert(tuple != NULL);

   dim   = tuple_get_dim(tuple);
   le    = NULL; /* Start again */
   
   while(NULL != (tuple = list_get_tuple(list, &le)))
   {
      if (tuple_get_dim(tuple) != dim)
      {
         le = NULL;
         fprintf(stderr, "*** Error 193: Different dimension tuples in set initialisation\n");
         tuple_print(stderr, tuple);
         fprintf(stderr, " vs. ");
         tuple_print(stderr, list_get_tuple(list, &le));
         fprintf(stderr, "\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      for(i = 0; i < dim; i++)
      {
         elem_type = elem_get_type(tuple_get_elem(tuple, i));
      
         if (elem_type != ELEM_NUMB && elem_type != ELEM_STRG)
         {
            assert(elem_type == ELEM_NAME);
         
            fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
               elem_get_name(tuple_get_elem(tuple, i)));
            code_errmsg(self);
            zpl_exit(EXIT_FAILURE);
         }
      }
   }
   code_value_set(self, set_new_from_list(list, SET_CHECK_WARN));

   return self;
}

CodeNode* i_set_new_elem(CodeNode* self)
{
   const List*  list;
   const Elem*  elem;
   ListElem*    le    = NULL;
   ElemType     elem_type;
   
   Trace("i_set_new_elem");

   assert(code_is_valid(self));

   list = code_eval_child_list(self, 0);

   while(NULL != (elem = list_get_elem(list, &le)))
   {
      elem_type = elem_get_type(elem);
      
      if (elem_type != ELEM_NUMB && elem_type != ELEM_STRG)
      {
         assert(elem_type == ELEM_NAME);
         
         fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
            elem_get_name(elem));
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   code_value_set(self, set_new_from_list(list, SET_CHECK_WARN));

   return self;
}

CodeNode* i_set_pseudo(CodeNode* self)
{
   Trace("i_set_pseudo");

   assert(code_is_valid(self));

   code_value_set(self, set_pseudo_new());

   return self;
}

CodeNode* i_set_empty(CodeNode* self)
{
   int  dim;
   
   Trace("i_set_empty");

   assert(code_is_valid(self));

   dim = code_eval_child_size(self, 0);

   code_value_set(self, set_empty_new(dim));

   return self;
}

static void check_sets_compatible(
   const CodeNode* self,
   const Set*      set_a,
   const Set*      set_b,
   const char*     op_name)
{
   Tuple*      tuple_a;
   Tuple*      tuple_b;
   ElemType    elem_type_a;
   ElemType    elem_type_b;
   int         i;

   /* If one of the two involved sets is empty, the dimension of the
    * other one does not matter.
    */
   if (set_get_members(set_a) == 0 || set_get_members(set_b) == 0)
      return;
   
   if (set_get_dim(set_a) != set_get_dim(set_b))
   {
      fprintf(stderr, "*** Error 119: %s of sets with different dimension\n", op_name);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   tuple_a = set_get_tuple(set_a, 0);
   tuple_b = set_get_tuple(set_b, 0);

   assert(tuple_get_dim(tuple_a) == set_get_dim(set_b));
   assert(tuple_get_dim(tuple_a) == tuple_get_dim(tuple_b));

   for(i = 0; i < tuple_get_dim(tuple_a); i++)
   {
      elem_type_a = elem_get_type(tuple_get_elem(tuple_a, i));
      elem_type_b = elem_get_type(tuple_get_elem(tuple_b, i));

      assert(elem_type_a == ELEM_NUMB || elem_type_a == ELEM_STRG);
      assert(elem_type_b == ELEM_NUMB || elem_type_b == ELEM_STRG);
      
      if (elem_type_a != elem_type_b)
      {
         fprintf(stderr, "*** Error 120: %s of sets with different types\n", op_name);
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   tuple_free(tuple_a);
   tuple_free(tuple_b);
}

CodeNode* i_set_union(CodeNode* self)
{
   const Set*  set_a;
   const Set*  set_b;
   
   Trace("i_set_union");

   assert(code_is_valid(self));

   set_a = code_eval_child_set(self, 0);
   set_b = code_eval_child_set(self, 1);

   check_sets_compatible(self, set_a, set_b, "Union");
   
   code_value_set(self, set_union(set_a, set_b));

   return self;
}

CodeNode* i_set_union2(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   CodeNode*     lexpr;
   SetIter*      iter;
   Tuple*        tuple;
   Set*          set_r = NULL;
   Set*          set_old;
   const Set*    set_new;
   
   Trace("i_set_union2");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(set, pattern);

   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);
   
   /* This routine is not efficient.
    * It would be better to make pairs and then unite the pairs, etc.
    * Now it is O(n) and it could be O(log n)
    */
   while((tuple = set_iter_next(iter, set)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         if (set_r == NULL)
            set_r = set_copy(code_eval_child_set(self, 1));
         else
         {
            assert(set_r != NULL);
            
            set_old = set_copy(set_r);
            set_new = code_eval_child_set(self, 1);

            check_sets_compatible(self, set_old, set_new, "Union");

            set_free(set_r);

            set_r = set_union(set_old, set_new);

            set_free(set_old);
         }
      }
      local_drop_frame();

      tuple_free(tuple);
   }
   set_iter_exit(iter, set);

   if (set_r == NULL)
      set_r = set_empty_new(tuple_get_dim(pattern));
   
   code_value_set(self, set_r);

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

   check_sets_compatible(self, set_a, set_b, "Minus");
   
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

   check_sets_compatible(self, set_a, set_b, "Intersection");

   code_value_set(self, set_inter(set_a, set_b));

   return self;
}

CodeNode* i_set_inter2(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    set;
   const Tuple*  pattern;
   CodeNode*     lexpr;
   SetIter*      iter;
   Tuple*        tuple;
   Set*          set_r = NULL;
   Set*          set_old;
   const Set*    set_new;
   
   Trace("i_set_inter2");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(set, pattern);

   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);
   
   /* This routine is not efficient.
    * It would be better to make pairs and then unite the pairs, etc.
    * Now it is O(n) and it could be O(log n)
    */
   while((tuple = set_iter_next(iter, set)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         if (set_r == NULL)
            set_r = set_copy(code_eval_child_set(self, 1));
         else
         {
            set_old = set_copy(set_r);
            set_new = code_eval_child_set(self, 1);

            check_sets_compatible(self, set_old, set_new, "Intersection");

            set_free(set_r);

            set_r = set_inter(set_old, set_new);

            set_free(set_old);
         }
      }
      local_drop_frame();

      tuple_free(tuple);
   }
   set_iter_exit(iter, set);
   
   if (set_r == NULL)
      set_r = set_empty_new(tuple_get_dim(pattern));
   
   code_value_set(self, set_r);

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

   check_sets_compatible(self, set_a, set_b, "Symmetric Difference");

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

   code_value_set(self, set_prod_new(set_a, set_b));

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
   int         diff;
   
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
      zpl_exit(EXIT_FAILURE);
   }
   if (!numb_is_int(upto))
   {
      fprintf(stderr, "*** Error 124: \"upto\" value ");
      numb_print(stderr, upto);
      fprintf(stderr, " in range too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   if (!numb_is_int(step))
   {
      fprintf(stderr, "*** Error 125: \"step\" value ");
      numb_print(stderr, step);
      fprintf(stderr, " in range too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   int_from = numb_toint(from);
   int_upto = numb_toint(upto);
   int_step = numb_toint(step);
   diff     = int_upto - int_from;
   int_step = Sgn(diff) * abs(int_step);

   if (diff == 0)
      int_step = 1;
   
   if (int_step == 0) 
   {
      fprintf(stderr, "*** Error 126: Zero \"step\" value in range\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   code_value_set(self, set_range_new(int_from, int_upto, int_step));

   return self;
}

CodeNode* i_set_range2(CodeNode* self)
{
   const Numb* from;
   const Numb* upto;
   const Numb* step;
   int         int_from;
   int         int_upto;
   int         int_step;
   int         diff;
   
   Trace("i_set_range2");

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
      zpl_exit(EXIT_FAILURE);
   }
   if (!numb_is_int(upto))
   {
      fprintf(stderr, "*** Error 124: \"upto\" value ");
      numb_print(stderr, upto);
      fprintf(stderr, " in range too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   if (!numb_is_int(step))
   {
      fprintf(stderr, "*** Error 125: \"step\" value ");
      numb_print(stderr, step);
      fprintf(stderr, " in range too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   int_from = numb_toint(from);
   int_upto = numb_toint(upto);
   int_step = numb_toint(step);
   diff     = int_upto - int_from;

   if (int_step == 0) 
   {
      fprintf(stderr, "*** Error 126: Zero \"step\" value in range\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   if ((Sgn(int_step) > 0 && diff < 0)
    || (Sgn(int_step) < 0 && diff > 0))
      code_value_set(self, set_empty_new(1));
   else
      code_value_set(self, set_range_new(int_from, int_upto, int_step));

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
         zpl_exit(EXIT_FAILURE);
      }
      numb = elem_get_numb(elem);
      
      if (!numb_is_int(numb))
      {
         fprintf(stderr, "*** Error 128: Index value ");
         numb_print(stderr, numb);
         fprintf(stderr, " in proj too big or not an integer\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      idx = numb_toint(numb);
      
      /* Are all the number between 1 and dim(set) ?
       */
      if (idx < 1 || idx > dim)
      {
         fprintf(stderr, "*** Error 129: Illegal index %d, ", idx);
         fprintf(stderr, " set has only dimension %d\n", dim);
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
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

static int noneval_get_dim(const CodeNode* code_cexpr_or_tuple)
{
   const CodeNode* code_cexpr_list;
   int             dim = 1;
   
   assert(code_is_valid(code_cexpr_or_tuple));

   /* Is it a tuple or a cexpr ?
    */
   if (code_get_inst(code_cexpr_or_tuple) == (Inst)i_tuple_new)
   {
      for(code_cexpr_list = code_get_child(code_cexpr_or_tuple, 0);
          code_get_inst(code_cexpr_list) == (Inst)i_elem_list_add;
          code_cexpr_list = code_get_child(code_cexpr_list, 0))
      {
         dim++;
      }
   }
   return dim;
}

CodeNode* i_set_expr(CodeNode* self)
{
   const IdxSet* idxset;
   const Set*    iset;
   const Tuple*  pattern;
   Tuple*        tuple;
   CodeNode*     lexpr;
   SetIter*      iter;
   CodeNode*     cexpr_or_tuple;
   Elem*         elem          = NULL;
   List*         list          = NULL;
   Bool          is_tuple_list = FALSE;
   
   Trace("i_expr_max");

   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   iset    = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(iset, pattern);
   
   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);

   while((tuple = set_iter_next(iter, iset)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
      {
         cexpr_or_tuple = code_eval_child(self, 1);      

         switch(code_get_type(cexpr_or_tuple))
         {
         case CODE_TUPLE :
            assert(list == NULL || is_tuple_list);

            is_tuple_list = TRUE;
            break;
         case CODE_NUMB :
            assert(!is_tuple_list);
            elem = elem_new_numb(code_get_numb(cexpr_or_tuple));
            break;
         case CODE_STRG :
            assert(!is_tuple_list);
            
            elem = elem_new_strg(code_get_strg(cexpr_or_tuple));
            break;
         case CODE_NAME :
            assert(!is_tuple_list);

            fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
               code_get_name(cexpr_or_tuple));
            code_errmsg(self);
            zpl_exit(EXIT_FAILURE);
         default :
            abort();
         }
         if (list == NULL)
         {
            list  = is_tuple_list
               ? list_new_tuple(code_get_tuple(cexpr_or_tuple))
               : list_new_elem(elem);
         }
         else
         {
            if (is_tuple_list)
               list_add_tuple(list, code_get_tuple(cexpr_or_tuple));
            else
               list_add_elem(list, elem);
         }
         if (!is_tuple_list)
         {
            assert(elem != NULL);

            elem_free(elem);
         }
      }
      local_drop_frame();

      tuple_free(tuple);
   }
   set_iter_exit(iter, iset);
   
   if (list == NULL)
   {
      if (verbose > VERB_QUIET)
      {
         fprintf(stderr, "--- Warning 202: Indexing over empty set\n");
         code_errmsg(code_get_child(self, 0));
      }
      /* If it is an cexpr list the dimension is 1, if it is a
       * tuple list, it is the dimension of the tuple.
       * Because of <i + j> we are not able to determine the dimension
       * of the tuple just by tuple_get_dim(code_eval_child_tuple(self, 1)).
       */
      code_value_set(self, set_empty_new(noneval_get_dim(code_get_child(self, 1))));
   }
   else
   {
      code_value_set(self, set_new_from_list(list, SET_CHECK_WARN));

      list_free(list);
   }
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
   Tuple*        tuple;
   Set*          newset;
   SetIter*      iter;
   const Set*    set;
   CodeNode*     lexpr;
   List*         list  = NULL;
   
   assert(idxset != NULL);
   
   set     = idxset_get_set(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   pattern = idxset_get_tuple(idxset);

   /* Is this an pseudo(idx)set ?
    */
   if (set_get_dim(set) == 0)
   {
      assert(tuple_get_dim(pattern) == 0);
      assert(code_get_bool(code_eval(lexpr)));

      newset = set_pseudo_new();
   }
   else if (idxset_is_unrestricted(idxset))
   {
      assert(code_get_bool(code_eval(lexpr)));

      newset = set_copy(set);
   }
   else
   {
      assert(tuple_get_dim(pattern) > 0);

      iter = set_iter_init(set, pattern);
      
      while((tuple = set_iter_next(iter, set)) != NULL)
      {
         local_install_tuple(pattern, tuple);

         if (code_get_bool(code_eval(lexpr)))
         {
            if (list == NULL)
               list = list_new_tuple(tuple);
            else
               list_add_tuple(list, tuple);
         }
         local_drop_frame();

         tuple_free(tuple);
      }
      set_iter_exit(iter, set);

      if (list == NULL)
      {
         newset = set_empty_new(tuple_get_dim(pattern));
         /* ??? maybe we need an error here ? */
      }
      else
      {
         newset = set_new_from_list(list, SET_CHECK_WARN);

         list_free(list);
      }
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
   Tuple*        tuple;
   SetIter*      iter;
   
   Trace("i_newsym_set1");

   name    = code_eval_child_name(self, 0);
   idxset  = code_eval_child_idxset(self, 1);
   iset    = set_from_idxset(idxset);
   sym     = symbol_new(name, SYM_SET, iset, set_get_members(iset), NULL);

   assert(code_is_valid(self));

   if (set_get_members(iset) == 0)
   {
      fprintf(stderr, "*** Error 197: Empty index set for set\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   pattern = idxset_get_tuple(idxset);
   iter    = set_iter_init(iset, pattern);

   warn_if_pattern_has_no_name(code_get_child(self, 1), pattern);
   
   while((tuple = set_iter_next(iter, iset)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      symbol_add_entry(sym,
         entry_new_set(tuple,
            code_eval_child_set(self, 2)));

      local_drop_frame();

      tuple_free(tuple);
   }
   set_iter_exit(iter, iset);
   set_free(iset);

   code_value_void(self);

   return self;
}
   
CodeNode* i_newsym_set2(CodeNode* self)
{
   const char*   name;
   const IdxSet* idxset;
   Set*          iset;
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
   iset   = set_from_idxset(idxset);
   list   = code_eval_child_list(self, 2);
   count  = list_get_elems(list);

   assert(list_is_entrylist(list));

   /* Empty set ?
    */
   if (set_get_members(iset) == 0)
   {
      fprintf(stderr, "*** Error 197: Empty index set for set\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   
   /* Pseudo set ?
    */
   if (set_get_dim(iset) == 0)
   {
      set_free(iset);

      iset = set_new_from_list(list, SET_CHECK_WARN);
   }
   sym = symbol_new(name, SYM_SET, iset, count, NULL);

   lelem = NULL;
   
   for(i = 0; i < count; i++)
   {
      entry  = list_get_entry(list, &lelem);
      tuple  = entry_get_tuple(entry);

      if (set_get_dim(iset) != tuple_get_dim(tuple))
      {
         fprintf(stderr, "*** Error 196: Indexing Tuple ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " has wrong dimension %d, expected %d\n",
            tuple_get_dim(tuple),
            set_get_dim(iset));
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }

      if (set_lookup(iset, tuple))
         symbol_add_entry(sym, entry_copy(entry));
      else
      {
         fprintf(stderr, "*** Error 131: Illegal element ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " for symbol\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   code_value_void(self);

   set_free(iset);
   
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

   if (set_get_members(iset) == 0)
   {
      fprintf(stderr, "*** Error 135: Empty index set for parameter\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   
   if (!list_is_entrylist(list))
   {
      /* This errors occurs, if the parameter is missing in the template
       * for a "read" statement.
       */
      assert(list_is_tuplelist(list));
      
      fprintf(stderr, "*** Error 132: Values in parameter list missing,\n");
      fprintf(stderr, "               probably wrong read template\n");      
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
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
      fprintf(stderr, "*** Error ???: Empty initialisation for parameter \"%s\
n",
         name);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
#endif
   sym   = symbol_new(name, SYM_ERR, iset, count, deflt);
   lelem = NULL;
   
   for(i = 0; i < count; i++)
   {
      entry  = list_get_entry(list, &lelem);
      tuple  = entry_get_tuple(entry);

      if (set_get_dim(iset) != tuple_get_dim(tuple))
      {
         fprintf(stderr, "*** Error 194: Indexing tuple ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " has wrong dimension %d, expected %d\n",
            tuple_get_dim(tuple),
            set_get_dim(iset));
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      if (!set_lookup(iset, tuple))
      {
         fprintf(stderr, "*** Error 134: Illegal element ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " for symbol\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      if (i > 0 && symbol_get_type(sym) != entry_get_type(entry))
      {
         fprintf(stderr, "*** Error 173: Illegal type in element ");
         entry_print(stderr, entry);
         fprintf(stderr, " for symbol\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      symbol_add_entry(sym, entry_copy(entry));
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
   Tuple*        tuple;
   const Tuple*  pattern;
   SetIter*      iter;
   int           count = 0;
   
   Trace("i_newsym_para2");

   assert(code_is_valid(self));

   name    = code_eval_child_name(self, 0);
   idxset  = code_eval_child_idxset(self, 1);
   iset    = set_from_idxset(idxset);
   child3  = code_eval_child(self, 3);

   if (set_get_members(iset) == 0)
   {
      fprintf(stderr, "*** Error 135: Index set for parameter \"%s\" is empty\n",
         name);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   
   if (code_get_type(child3) == CODE_VOID)
      deflt = ENTRY_NULL;
   else
      deflt = code_get_entry(code_eval(child3));
   
   sym     = symbol_new(name, SYM_ERR, iset, set_get_members(iset), deflt);
   pattern = idxset_get_tuple(idxset);
   iter    = set_iter_init(iset, pattern);

   warn_if_pattern_has_no_name(code_get_child(self, 1), pattern);
   
   while((tuple = set_iter_next(iter, iset)) != NULL)
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
         fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
            code_get_name(child));
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      default :
         abort();
      }
      if (count > 0 && symbol_get_type(sym) != entry_get_type(entry))
      {
         fprintf(stderr, "*** Error 173: Illegal type in element ");
         entry_print(stderr, entry);
         fprintf(stderr, " for symbol\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      symbol_add_entry(sym, entry);
      
      local_drop_frame();

      tuple_free(tuple);
      
      count++;
   }
   set_iter_exit(iter, iset);

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
   Tuple*        tuple;
   const Tuple*  pattern;
   VarClass      varclass;
   VarClass      usevarclass;
   Var*          var;
   SetIter*      iter;
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
   sym      = symbol_new(name, SYM_VAR, iset, set_get_members(iset), NULL);
   iter    = set_iter_init(iset, pattern);

   warn_if_pattern_has_no_name(code_get_child(self, 1), pattern);
   
   while((tuple = set_iter_next(iter, iset)) != NULL)
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

         if (verbose > VERB_QUIET)
            fprintf(stderr,
               "--- Warning 136: Lower bound for var %s set to infinity -- ignored\n",
               name);
      }
      if (bound_get_type(upper) == BOUND_MINUS_INFTY)
      {
         bound_free(upper);
         upper = bound_new(BOUND_INFTY, NULL);

         if (verbose > VERB_QUIET)
            fprintf(stderr,
               "--- Warning 137: Upper bound for var %s set to -infinity -- ignored\n",
               name);
      }

      if ((varclass == VAR_CON)
         && (!numb_equal(priority, numb_zero()) || !numb_equal(startval, numb_zero())))
         if (verbose > VERB_QUIET)
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
               
               if (verbose > VERB_QUIET)
               {
                  fprintf(stderr,
                     "--- Warning 139: Lower bound for integral var %s truncated to ",
                     name);
                  numb_print(stderr, temp);
                  fputc('\n', stderr);
               }
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
               
               if (verbose > VERB_QUIET)
               {
                  fprintf(stderr,
                     "--- Warning 140: Upper bound for integral var %s truncated to ",
                     name);
                  numb_print(stderr, temp);
                  fputc('\n', stderr);
               }
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
         zpl_exit(EXIT_FAILURE);
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

      tuple_free(tuple);
      bound_free(lower);
      bound_free(upper);
   }
   set_iter_exit(iter, iset);

   code_value_void(self);

   set_free(iset);

   return self;
}

CodeNode* i_symbol_deref(CodeNode* self)
{
   const Symbol* sym;
   const Tuple*  tuple;
   const Entry*  entry;
   const Elem*   elem;
   Term*         term;
   int           i;
   
   Trace("i_symbol_deref");
   
   assert(code_is_valid(self));

   sym   = code_eval_child_symbol(self, 0);
   tuple = code_eval_child_tuple(self, 1);   

   /* wurde schon in mmlscan ueberprueft
    */
   assert(sym != NULL);

   for(i = 0; i < tuple_get_dim(tuple); i++)
   {
      elem = tuple_get_elem(tuple, i);

      /* Are there any unresolved names in the tuple?
       */
      if (ELEM_NAME == elem_get_type(elem))
      {
         fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
            elem_get_name(elem));
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   
   entry = symbol_lookup_entry(sym, tuple);

   if (NULL == entry)
   {
      fprintf(stderr, "*** Error 142: Unknown index ");
      tuple_print(stderr, tuple);
      fprintf(stderr, " for symbol \"%s\"\n", symbol_get_name(sym));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
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

CodeNode* i_newdef(CodeNode* self)
{
   Define* def;
   
   Trace("i_newdef");

   assert(code_is_valid(self));

   def = code_eval_child_define(self, 0);
   
   define_set_param(def, tuple_copy(code_eval_child_tuple(self, 1)));
   define_set_code(def, code_get_child(self, 2));
   
   code_value_void(self);

   return self;
}

CodeNode* i_define_deref(CodeNode* self)
{
   const Define* def;
   const Tuple*  tuple;
   const Tuple*  param;
   ElemType      elem_type;
   int           i;
   
   Trace("i_define_deref");
   
   assert(code_is_valid(self));

   def   = code_eval_child_define(self, 0);
   tuple = code_eval_child_tuple(self, 1);   

   for(i = 0; i < tuple_get_dim(tuple); i++)
   {
      elem_type = elem_get_type(tuple_get_elem(tuple, i));
      
      if (elem_type != ELEM_NUMB && elem_type != ELEM_STRG)
      {
         assert(elem_type == ELEM_NAME);
         
         fprintf(stderr, "*** Error 170: Uninitialised local parameter \"%s\"\n",
            elem_get_name(tuple_get_elem(tuple, i)));
         fprintf(stderr, "               in call of define \"%s\".\n",
            define_get_name(def));
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   
   /* wurde schon in mmlscan ueberprueft
    */
   assert(def != NULL);

   param = define_get_param(def);
   
   if (tuple_get_dim(tuple) != tuple_get_dim(param))
   {
      fprintf(stderr, "*** Error 171: Wrong number of arguments (%d instead of %d)\n",
         tuple_get_dim(tuple),
         tuple_get_dim(param));
      fprintf(stderr, "               for call of define \"%s\".\n",
         define_get_name(def));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   local_install_tuple(param, tuple);

   code_copy_value(self, code_eval(define_get_code(def)));

   local_drop_frame();

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
   Tuple*       t1;
   ElemType     elem_type;
   CodeNode*    lexpr;
   const Set*   set;
   char         name[13]; /* "@-2000000000" */
   int          dim;
   int          i;
   Bool         is_unrestricted;
         
   Trace("i_idxset_new");

   assert(code_is_valid(self));

   t0     = tuple_new(0);
   tuple  = tuple_copy(code_eval_child_tuple(self, 0));
   set    = code_eval_child_set(self, 1);
   lexpr  = code_get_child(self, 2);
   dim    = set_get_dim(set);

   is_unrestricted = code_get_inst(lexpr) == (Inst)i_bool_true;

   /* If we get any empty set with dimension 0, it is not the result
    * of some other operation, but genuine empty.
    * This is an error.
    */
   if (set_get_dim(set) == 0)
   {
      assert(set_get_members(set) == 0);

      fprintf(stderr, "*** Error 195: Genuine empty index as index set\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   /* Attention: set_get_members(set) == 0 is possible!
    */
   assert(set_get_dim(set) > 0);
   
   /* If no index tuple was given, we construct one.
    * This will always be ok.
    */
   if (!tuple_cmp(tuple, t0))
   {
      tuple_free(tuple);

      tuple = tuple_new(dim);
      
      for(i = 0; i < dim; i++)
      {
         sprintf(name, "@%d", i + 1);
         tuple_set_elem(tuple, i, elem_new_name(str_new(name)));
      }
   }
   else
   {
      /* If a index tuple was given, check if
       * - the dimension is correct
       * - any not NAME type entries are compatible.
       * - the set is unrestricted, ie all NAMES, no WITH.
       */
      assert(dim > 0 || set_get_members(set) == 0);
      
      if (dim > 0 && tuple_get_dim(tuple) != dim)
      {
         fprintf(stderr, "*** Error 188: Index tuple has wrong dimension\n");
         tuple_print(stderr, tuple);
         fprintf(stderr, " should have dimension %d\n", dim);
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      if (dim > 0 && set_get_members(set) > 0)
      {
         t1 = set_get_tuple(set, 0);

         for(i = 0; i < dim; i++)
         {
            elem_type = elem_get_type(tuple_get_elem(tuple, i));
               
            if (elem_type != ELEM_NAME)
            {
               is_unrestricted = FALSE;
               
               if (elem_type != elem_get_type(tuple_get_elem(t1, i)))
               {
                  fprintf(stderr, "*** Error 198: Incompatible index tuple\n");
                  tuple_print(stderr, tuple);
                  fprintf(stderr, " component %d is not compatible with ", i + 1);
                  tuple_print(stderr, t1);
                  fprintf(stderr, "\n");
                  code_errmsg(self);
                  zpl_exit(EXIT_FAILURE);
               }
            }
         }
         tuple_free(t1);
      }
   }
   code_value_idxset(self, idxset_new(tuple, set, lexpr, is_unrestricted));

   tuple_free(t0);
   tuple_free(tuple);

   return self;
}

CodeNode* i_idxset_pseudo_new(CodeNode* self)
{
   Tuple*    tuple;
   Set*      set;
         
   Trace("i_idxset_pseudo_new");

   assert(code_is_valid(self));

   tuple = tuple_new(0);
   set   = set_pseudo_new();
   
   code_value_idxset(self, idxset_new(tuple, set, code_get_child(self, 0), TRUE));

   set_free(set);
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
   Tuple*        tuple;
   CodeNode*     lexpr;
   SetIter*      iter;
   Term*         term_r;

   Trace("i_term_sum");
   
   assert(code_is_valid(self));

   idxset  = code_eval_child_idxset(self, 0);
   set     = idxset_get_set(idxset);
   pattern = idxset_get_tuple(idxset);
   lexpr   = idxset_get_lexpr(idxset);
   iter    = set_iter_init(set, pattern);
   term_r  = term_new(1);
   
   warn_if_pattern_has_no_name(code_get_child(self, 0), pattern);

   while((tuple = set_iter_next(iter, set)) != NULL)
   {
      local_install_tuple(pattern, tuple);

      if (code_get_bool(code_eval(lexpr)))
         term_append_term(term_r, code_eval_child_term(self, 1));

      local_drop_frame();

      tuple_free(tuple);
   }
   set_iter_exit(iter, set);
   
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
      fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
         code_get_name(child));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
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
   used        = set_get_members(set);
   numb        = code_eval_child_numb(self, 1);

   if (!numb_is_int(numb))
   {
      fprintf(stderr, "*** Error 143: Size for subsets ");
      numb_print(stderr, numb);
      fprintf(stderr, " is too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   subset_size = numb_toint(numb);
   
   if (used < 1)
   {
      fprintf(stderr, "*** Error 144: Tried to build subsets of empty set\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   assert(set_get_dim(set) > 0);
   
   if ((subset_size < 1) || (subset_size > used))
   {
      fprintf(stderr, "*** Error 145: Illegal size for subsets %d,\n",
         subset_size);
      fprintf(stderr, "               should be between 1 and %d\n",
         set_get_members(set));
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
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
   used = set_get_members(set);
   
   if (used < 1)
   {
      fprintf(stderr, "*** Error 146: Tried to build powerset of empty set\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   assert(set_get_dim(set) > 0);

   for(i = 0; i <= used; i++)
      list = set_subsets_list(set, i, list, &idx);

   assert(list != NULL);
   
   code_value_list(self, list);
   
   return self;
}

CodeNode* i_list_matrix(CodeNode* self)
{
   const List* head_list;
   const List* body_list;
   const List* idx_list;
   const List* val_list;
   List*       list = NULL;
   ListElem*   le_head;
   ListElem*   le_body;
   ListElem*   le_val;
   ListElem*   le_idx;
   int         head_count;
   int         body_count;
   int         idx_count;
   Tuple*      tuple;
   const Elem* elem;
   Entry*      entry;
   int         i;
   int         j;
   int         k;
   
   Trace("i_list_matrix");

   assert(code_is_valid(self));

   head_list  = code_eval_child_list(self, 0);
   body_list  = code_eval_child_list(self, 1);
   head_count = list_get_elems(head_list);
   body_count = list_get_elems(body_list);
   
   assert(head_count > 0);
   assert(body_count > 0);
   assert(body_count % 2 == 0); /* has to be even */

   le_body = NULL;
   
   for(i = 0; i < body_count; i += 2)
   {
      idx_list  = list_get_list(body_list, &le_body);
      val_list  = list_get_list(body_list, &le_body);
      idx_count = list_get_elems(idx_list);

      /* Number of values in a lines has to be equal the
       * number of elements in the head list
       */
      if (list_get_elems(val_list) != head_count)
      {
         fprintf(stderr, "*** Error 172: Wrong number of entries (%d) in table line,\n",
            list_get_elems(val_list));
         fprintf(stderr, "               expected %d entries\n", head_count);
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }

      le_head = NULL;
      le_val  = NULL;

      /* For each element in the head list we end up with
       * one element in the result list
       */
      for(j = 0; j < head_count; j++)
      {
         /* Construct tuple. If idx_count is not constant, we will later
          * get an error when the list is applied to the parameter
          */
         tuple = tuple_new(idx_count + 1);

         le_idx = NULL;
         for(k = 0; k < idx_count; k++)
            tuple_set_elem(tuple, k, elem_copy(list_get_elem(idx_list, &le_idx)));

         tuple_set_elem(tuple, k, elem_copy(list_get_elem(head_list, &le_head)));
      
         elem = list_get_elem(val_list, &le_val);

         switch(elem_get_type(elem))
         {
         case ELEM_NUMB :
            entry = entry_new_numb(tuple, elem_get_numb(elem));
            break;
         case ELEM_STRG :
            entry = entry_new_strg(tuple, elem_get_strg(elem));
            break;
         case ELEM_NAME :
            fprintf(stderr, "*** Error 133: Unknown symbol \"%s\"\n",
               elem_get_name(elem));
            code_errmsg(self);
            zpl_exit(EXIT_FAILURE);
         default :
            abort();
         }
         if (list == NULL)
            list  = list_new_entry(entry);
         else
            list_add_entry(list, entry);

         entry_free(entry);
         tuple_free(tuple);
      }
   }
   assert(list != NULL);
   
   code_value_list(self, list);

   return self;
}

CodeNode* i_matrix_list_new(CodeNode* self)
{
   List* list;
   
   Trace("i_matrix_list_new");

   assert(code_is_valid(self));

   list = list_new_list(code_eval_child_list(self, 0));
   list_add_list(list, code_eval_child_list(self, 1));

   code_value_list(self, list);

   return self;
}

CodeNode* i_matrix_list_add(CodeNode* self)
{
   List* list;
  
   Trace("i_matrix_list_add");

   assert(code_is_valid(self));

   list = list_copy(code_eval_child_list(self, 0));
   list_add_list(list, code_eval_child_list(self, 1));
   list_add_list(list, code_eval_child_list(self, 2));

   code_value_list(self, list);

   return self;
}

static void objective(CodeNode* self, Bool minimize)
{
   const char* name;
   const Term* term;
   
   assert(code_is_valid(self));

   name = code_eval_child_name(self, 0);

   if (!conname_set(name))
   {
      fprintf(stderr, "*** Error 105: Duplicate constraint name \"%s\"\n", name);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   term = code_eval_child_term(self, 1);

   xlp_objname(name);
   xlp_setdir(minimize);
   term_to_objective(term);

   conname_free();
   
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

CodeNode* i_check(CodeNode* self)
{
   Trace("i_check");

   assert(code_is_valid(self));

   if (!code_eval_child_bool(self, 0))
   {
      fprintf(stderr, "*** Error 900: Check failed!\n");
      local_print_all(stderr);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   code_value_void(self);

   return self;
}




