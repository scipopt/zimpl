#pragma ident "@(#) $Id: vinst.c,v 1.1 2003/10/08 08:03:06 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: vinst.c                                                       */
/*   Name....: Variable Instructions                                         */
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

static CodeNode* handle_vbool_eq_ne(CodeNode* self, Bool eq)
{
   Symbol*      sym;
   Term*        term;
   const Term*  term_lhs;
   const Term*  term_rhs;
   Con*         con;
   Numb*        rhs;
   unsigned int flags;
   const char*  cname;
   char*        temp;
   Bound*       lower;
   Bound*       upper;
   Bound*       bound_zero;
   Bound*       bound_one;
   Entry*       entry_xplus;
   Entry*       entry_xminus;
   Entry*       entry_bplus;
   Entry*       entry_bminus;
   Entry*       entry_result;
   Var*         var;
   Tuple*       tuple;
   Numb*        numb;
   
   Trace("i_vbool");
   
   assert(code_is_valid(self));

   term_lhs   = code_eval_child_term(self, 0);
   term_rhs   = code_eval_child_term(self, 1);
   flags      = 0;

   rhs        = numb_new_sub(term_get_constant(term_rhs), term_get_constant(term_lhs));
   term       = term_sub_term(term_lhs, term_rhs);

   term_add_constant(term, rhs);

   /* Check if trival infeasible
    */
   if (term_get_elements(term) == 0)
   {
      fprintf(stderr, "*** Error ???: Empty LHS, in boolean constraint\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   if (!term_is_all_integer(term))
   {
      fprintf(stderr, "*** Error ???: Boolean constraint not all integer\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }
   cname = conname_get();
   temp  = malloc(strlen(SYMBOL_NAME_INTERNAL) + strlen(cname) + 5);
   
   bound_zero = bound_new(BOUND_VALUE, numb_zero());
   bound_one  = bound_new(BOUND_VALUE, numb_one());
   lower      = term_get_lower_bound(term);
   upper      = term_get_upper_bound(term);
   
   if (numb_cmp(bound_get_value(lower), numb_zero()) < 0)
   {
      numb = numb_copy(bound_get_value(lower));
      numb_abs(numb);
      bound_free(lower);
      lower = bound_new(BOUND_VALUE, numb);
      numb_free(numb);
   }
   else
   {
      bound_free(lower);
      lower = bound_new(BOUND_VALUE, numb_zero());
   }
   
   if (numb_cmp(bound_get_value(upper), numb_zero()) < 0)
   {
      bound_free(upper);
      upper = bound_new(BOUND_VALUE, numb_zero());
   }
   /* Create x^+
    */
   sprintf(temp, "%s%s_xp", SYMBOL_NAME_INTERNAL, cname);
   var   = xlp_addvar(temp, VAR_INT, bound_zero, upper, numb_zero(), numb_zero());
   tuple = tuple_new(1);
   tuple_set_elem(tuple, 0, elem_new_strg(str_new(temp + strlen(SYMBOL_NAME_INTERNAL))));
   entry_xplus = entry_new_var(tuple, var);
   tuple_free(tuple);
   
   /* Create x^-
    */
   sprintf(temp, "%s%s_xm", SYMBOL_NAME_INTERNAL, cname);
   var   = xlp_addvar(temp, VAR_INT, bound_zero, lower, numb_zero(), numb_zero());
   tuple = tuple_new(1);
   tuple_set_elem(tuple, 0, elem_new_strg(str_new(temp + strlen(SYMBOL_NAME_INTERNAL))));
   entry_xminus = entry_new_var(tuple, var);
   tuple_free(tuple);

   /* Create b^+
    */
   sprintf(temp, "%s%s_bp", SYMBOL_NAME_INTERNAL, cname);
   var   = xlp_addvar(temp, VAR_BIN, bound_zero, bound_one, numb_zero(), numb_zero());
   tuple = tuple_new(1);
   tuple_set_elem(tuple, 0, elem_new_strg(str_new(temp + strlen(SYMBOL_NAME_INTERNAL))));
   entry_bplus = entry_new_var(tuple, var);
   tuple_free(tuple);
   
   /* Create b^-
    */
   sprintf(temp, "%s%s_bm", SYMBOL_NAME_INTERNAL, cname);
   var   = xlp_addvar(temp, VAR_BIN, bound_zero, bound_one, numb_zero(), numb_zero());
   tuple = tuple_new(1);
   tuple_set_elem(tuple, 0, elem_new_strg(str_new(temp + strlen(SYMBOL_NAME_INTERNAL))));
   entry_bminus = entry_new_var(tuple, var);
   tuple_free(tuple);

   /* Create result
    */
   sprintf(temp, "%s%s_re", SYMBOL_NAME_INTERNAL, cname);
   var   = xlp_addvar(temp, VAR_BIN, bound_zero, bound_one, numb_zero(), numb_zero());
   tuple = tuple_new(1);
   tuple_set_elem(tuple, 0, elem_new_strg(str_new(temp + strlen(SYMBOL_NAME_INTERNAL))));
   entry_result = entry_new_var(tuple, var);
   tuple_free(tuple);

   /* add term = x^+ + x^-
    */
   term_add_elem(term, entry_xplus, numb_minusone());
   term_add_elem(term, entry_xminus, numb_one());

   sprintf(temp, "%s_a", cname);
   con = xlp_addcon(temp, CON_EQUAL, rhs, rhs, flags);
   term_to_nzo(term, con);
   term_free(term);
   numb_free(rhs);
   
   /* bplus <= xplus */
   term = term_new(2);
   term_add_elem(term, entry_bplus, numb_one());
   term_add_elem(term, entry_xplus, numb_minusone());
   sprintf(temp, "%s_b", cname);
   con = xlp_addcon(temp, CON_RHS, numb_zero(), numb_zero(), flags);
   term_to_nzo(term, con);
   term_free(term);
   
   /* bminus <= xminus */
   term = term_new(2);
   term_add_elem(term, entry_bminus, numb_one());
   term_add_elem(term, entry_xminus, numb_minusone());
   sprintf(temp, "%s_c", cname);
   con = xlp_addcon(temp, CON_RHS, numb_zero(), numb_zero(), flags);
   term_to_nzo(term, con);
   term_free(term);

   /* bplus * upper >= xplus */
   term = term_new(2);
   if (!numb_equal(bound_get_value(upper), numb_zero()))
      term_add_elem(term, entry_bplus, bound_get_value(upper));
   term_add_elem(term, entry_xplus, numb_minusone());
   sprintf(temp, "%s_d", cname);
   con = xlp_addcon(temp, CON_LHS, numb_zero(), numb_zero(), flags);
   term_to_nzo(term, con);
   term_free(term);

   /* bminus * lower >= xminus */
   term = term_new(2);
   if (!numb_equal(bound_get_value(lower), numb_zero()))
      term_add_elem(term, entry_bminus, bound_get_value(lower));
   term_add_elem(term, entry_xminus, numb_minusone());
   sprintf(temp, "%s_e", cname);
   con = xlp_addcon(temp, CON_LHS, numb_zero(), numb_zero(), flags);
   term_to_nzo(term, con);
   term_free(term);

   if (eq)
   {
      /* 1 - result = bplus + bminus =>  result + bplus + bminus = 1 */
      term = term_new(3);
      term_add_elem(term, entry_result, numb_one());
      term_add_elem(term, entry_bplus,  numb_one());
      term_add_elem(term, entry_bminus, numb_one());
      sprintf(temp, "%s_f", cname);
      con = xlp_addcon(temp, CON_EQUAL, numb_one(), numb_one(), flags);
      term_to_nzo(term, con);
      term_free(term);
   }
   else
   {
      /* result = bplus + bminus */
      term = term_new(3);
      term_add_elem(term, entry_result, numb_one());
      term_add_elem(term, entry_bplus,  numb_minusone());
      term_add_elem(term, entry_bminus, numb_minusone());
      sprintf(temp, "%s_f", cname);
      con = xlp_addcon(temp, CON_EQUAL, numb_zero(), numb_zero(), flags);
      term_to_nzo(term, con);
      term_free(term);
   }
   //------------------------------------------------------
   term = term_new(1);
   term_add_elem(term, entry_result, numb_one());

   code_value_term(self, term);

   /* Symbol for internal entries
    */
   sym  = symbol_lookup(SYMBOL_NAME_INTERNAL);

   assert(sym != NULL);
   
   symbol_add_entry(sym, entry_xplus);
   symbol_add_entry(sym, entry_xminus);
   symbol_add_entry(sym, entry_bplus);
   symbol_add_entry(sym, entry_bminus);
   symbol_add_entry(sym, entry_result);
   
   bound_free(bound_one);
   bound_free(bound_zero);
   bound_free(lower);
   bound_free(upper);
   free(temp);
   
   return self;
}

CodeNode* i_vbool_ne(CodeNode* self)
{
   return handle_vbool_eq_ne(self, FALSE);
}

CodeNode* i_vbool_eq(CodeNode* self)
{
   return handle_vbool_eq_ne(self, TRUE);
}
   
static void handle_vif_true(
   const CodeNode* self,
   const Term*     vif_term,
   const Term*     lhs_term,
   ConType         con_type,
   const Term*     rhs_term,
   unsigned int    flags)
{
   Term*        term;
   Term*        big_term;
   Con*         con;
   Numb*        rhs;
   Bound*       upper;
   Bound*       lower;
   Numb*        big_m;
   const Numb*  upper_val;
   const Numb*  lower_val;
   
   Trace("i_vif_true");
   
   rhs   = numb_new_sub(term_get_constant(rhs_term), term_get_constant(lhs_term));
   term  = term_sub_term(lhs_term, rhs_term);
   term_add_constant(term, rhs);

   /* Check if trival infeasible
    */
   if (term_get_elements(term) == 0)
   {
      fprintf(stderr, "*** Error ???: Empty LHS, not allowed\n");
      code_errmsg(self);
      exit(EXIT_FAILURE);
   }

   assert(con_type == CON_RHS || con_type == CON_LHS || con_type == CON_EQUAL);
   
   /* <=, ==
    */
   if (con_type == CON_RHS || con_type == CON_EQUAL)
   {
      upper = term_get_upper_bound(lhs_term);

      if (bound_get_type(upper) != BOUND_VALUE)
      {
         fprintf(stderr, "*** Error ???: Conditional only possible on bounded constraints\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
      upper_val = bound_get_value(upper);
      big_m     = numb_new_sub(upper_val, rhs);
      big_term  = term_copy(vif_term);

      term_mul_coeff(big_term, big_m);
      term_append_term(big_term, term);

      con = xlp_addcon(conname_get(), CON_RHS, upper_val, upper_val, flags);
   
      term_to_nzo(big_term, con);
      
      bound_free(upper);
      numb_free(big_m);
      term_free(big_term);
   }
   /* >=, ==
    */
   if (con_type == CON_LHS || con_type == CON_EQUAL)
   {
      lower = term_get_lower_bound(lhs_term);

      if (bound_get_type(lower) != BOUND_VALUE)
      {
         fprintf(stderr, "*** Error ???: Conditional only possible on bounded constraints\n");
         code_errmsg(self);
         exit(EXIT_FAILURE);
      }
      lower_val = bound_get_value(lower);
      big_m     = numb_new_sub(lower_val, rhs);
      big_term  = term_copy(vif_term);

      term_mul_coeff(big_term, big_m);
      term_append_term(big_term, term);

      con = xlp_addcon(conname_get(), CON_LHS, lower_val, lower_val, flags);
   
      term_to_nzo(big_term, con);

      bound_free(lower);
      numb_free(big_m);
      term_free(big_term);
   }
   numb_free(rhs);
   term_free(term);
}

/*ARGSUSED*/
static void handle_vif_false(
   const CodeNode* self,
   const Term*     vif_term,
   const Term*     lhs_term,
   ConType         con_type,
   const Term*     rhs_term,
   unsigned int    flags)
{
}


CodeNode* i_vif_else(CodeNode* self)
{
   const Term*  vif_term;
   const Term*  lhs_term;
   const Term*  rhs_term;
   ConType      con_type;   
   unsigned int flags = 0;
   
   Trace("i_vif_else");
   
   assert(code_is_valid(self));

   vif_term   = code_eval_child_term(self, 0);
   lhs_term   = code_eval_child_term(self, 1);
   con_type   = code_eval_child_contype(self, 2);
   rhs_term   = code_eval_child_term(self, 3);

   handle_vif_true(self, vif_term, lhs_term, con_type, rhs_term, flags);
   
   lhs_term   = code_eval_child_term(self, 4);
   con_type   = code_eval_child_contype(self, 5);
   rhs_term   = code_eval_child_term(self, 6);

   handle_vif_false(self, vif_term, lhs_term, con_type, rhs_term, flags);

   code_value_void(self);

   return self;
}

CodeNode* i_vif(CodeNode* self)
{
   const Term*  vif_term;
   const Term*  lhs_term;
   const Term*  rhs_term;
   ConType      con_type;   
   unsigned int flags = 0;
   
   Trace("i_vif");
   
   assert(code_is_valid(self));

   vif_term   = code_eval_child_term(self, 0);
   lhs_term   = code_eval_child_term(self, 1);
   con_type   = code_eval_child_contype(self, 2);
   rhs_term   = code_eval_child_term(self, 3);

   handle_vif_true(self, vif_term, lhs_term, con_type, rhs_term, flags);
   
   code_value_void(self);

   return self;
}


   





