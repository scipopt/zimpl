#pragma ident "@(#) $Id: term.c,v 1.29 2007/08/02 08:36:56 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: term.c                                                        */
/*   Name....: Term Functions                                                */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2007 by Thorsten Koch <koch@zib.de>
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

/* #define TRACE 1 */

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "xlpglue.h"

#define TERM_EXTEND_SIZE 16
#define TERM_SID         0x5465726d

typedef struct term_element TermElem;

struct term_element
{
   const Entry* entry;
   Numb*        coeff;
};

struct term
{
   SID
   Numb*     constant;
   int       size;
   int       used;
   TermElem* elem;
};

Term* term_new(int size)
{
   Term* term = calloc(1, sizeof(*term));

   Trace("term_new");
   
   assert(term != NULL);
   assert(size >  0);
   
   term->constant = numb_new_integer(0);
   term->size     = size;
   term->used     = 0;
   term->elem     = calloc(term->size, sizeof(*term->elem));
   
   SID_set(term, TERM_SID);
   assert(term_is_valid(term));

   return term;
}

void term_add_elem(Term* term, const Entry* entry, const Numb* coeff)
{
   Trace("term_add_elem");

   assert(term_is_valid(term));
   assert(entry_is_valid(entry));
   assert(!numb_equal(coeff, numb_zero()));
   assert(term->used <= term->size);
   
   if (term->used == term->size)
   {
      term->size   += TERM_EXTEND_SIZE;
      term->elem    = realloc(
         term->elem, (size_t)term->size * sizeof(*term->elem));

      assert(term->elem != NULL);
   }
   assert(term->used < term->size);

   term->elem[term->used].entry = entry;
   term->elem[term->used].coeff = numb_copy(coeff);
   term->used++;

   assert(term_is_valid(term));
}

void term_free(Term* term)
{
   int i;

   Trace("term_free");

   assert(term_is_valid(term));

   SID_del(term);

   for(i = 0; i < term->used; i++)
      numb_free(term->elem[i].coeff);

   numb_free(term->constant);
   
   free(term->elem);
   free(term);
}

Bool term_is_valid(const Term* term)
{
   return term != NULL && SID_ok(term, TERM_SID) && (term->used <= term->size);
}

Term* term_copy(const Term* term)
{
   Term* tnew = term_new(term->used + TERM_EXTEND_SIZE);
   int   i;
   
   Trace("term_copy");
   
   assert(term_is_valid(term));
   assert(term_is_valid(tnew));

   for(i = 0; i < term->used; i++)
   {
      tnew->elem[i].entry = term->elem[i].entry;
      tnew->elem[i].coeff = numb_copy(term->elem[i].coeff);
   }
   tnew->used     = term->used;
   numb_set(tnew->constant, term->constant);

   assert(term_is_valid(tnew));

   return tnew;
}

void term_append_term(
   Term* term_a,
   const Term* term_b)
{
   int i;
   
   Trace("term_append_term");

   assert(term_is_valid(term_a));
   assert(term_is_valid(term_b));

   numb_add(term_a->constant, term_b->constant);

   for(i = 0; i < term_b->used; i++)
      term_add_elem(term_a, term_b->elem[i].entry, term_b->elem[i].coeff);

   assert(term_is_valid(term_a));
}

Term* term_add_term(const Term* term_a, const Term* term_b)
{
   Term* term;
   int   i;
   
   Trace("term_add_term");

   assert(term_is_valid(term_a));
   assert(term_is_valid(term_b));

   term           = term_new(term_a->used + term_b->used + TERM_EXTEND_SIZE);
   term->used     = term_a->used + term_b->used;

   numb_set(term->constant, term_a->constant);
   numb_add(term->constant, term_b->constant);

   assert(term->size >= term->used);

   for(i = 0; i < term_a->used; i++)
   {
      term->elem[i].entry = term_a->elem[i].entry;
      term->elem[i].coeff = numb_copy(term_a->elem[i].coeff);
   }
   for(i = 0; i < term_b->used; i++)
   {
      term->elem[i + term_a->used].entry = term_b->elem[i].entry;
      term->elem[i + term_a->used].coeff = numb_copy(term_b->elem[i].coeff);
   }
   assert(term_is_valid(term));

   return term;
}

Term* term_sub_term(const Term* term_a, const Term* term_b)
{
   Term* term;
   int   i;

   Trace("term_sub_term");
   
   assert(term_is_valid(term_a));
   assert(term_is_valid(term_b));

   term           = term_new(term_a->used + term_b->used + TERM_EXTEND_SIZE);
   term->used     = term_a->used + term_b->used;

   numb_set(term->constant, term_a->constant);
   numb_sub(term->constant, term_b->constant);

   assert(term->size >= term->used);

   for(i = 0; i < term_a->used; i++)
   {
      term->elem[i].entry = term_a->elem[i].entry;
      term->elem[i].coeff = numb_copy(term_a->elem[i].coeff);
   }
   for(i = 0; i < term_b->used; i++)
   {
      term->elem[i + term_a->used].entry = term_b->elem[i].entry;
      term->elem[i + term_a->used].coeff = numb_copy(term_b->elem[i].coeff);
      numb_neg(term->elem[i + term_a->used].coeff);
   }
   assert(term_is_valid(term));

   return term;
}

/*lint -e{818} supress "Pointer parameter 'term' could be declared as pointing to const" */
void term_add_constant(
   Term* term, 
   const Numb* value)
{
   Trace("term_add_constant");

   assert(term_is_valid(term));

   numb_add(term->constant, value);

   assert(term_is_valid(term));
}

/*lint -e{818} supress "Pointer parameter 'term' could be declared as pointing to const" */
void term_sub_constant(Term* term, const Numb* value)
{
   Trace("term_sub_constant");

   assert(term_is_valid(term));

   numb_sub(term->constant, value);

   assert(term_is_valid(term));
}

void term_mul_coeff(Term* term, const Numb* value)
{
   int i;

   Trace("term_mul_coeff");

   assert(term_is_valid(term));

   numb_mul(term->constant, value);

   if (numb_equal(value, numb_zero()))
   {
      for(i = 0; i < term->used; i++)
         numb_free(term->elem[i].coeff);
      
      term->used = 0;
   }
   else
   {
      for(i = 0; i < term->used; i++)
         numb_mul(term->elem[i].coeff, value);
   }
   assert(term_is_valid(term));
}

const Numb* term_get_constant(const Term* term)
{
   assert(term_is_valid(term));
   
   return term->constant;
}

void term_negate(Term* term)
{
   Trace("term_negate");

   assert(term_is_valid(term));

   term_mul_coeff(term, numb_minusone());
}

void term_to_nzo(const Term* term, Con* con)
{
   Var*   var;
   int    i;

   Trace("term_to_nzo");
   
   assert(con != NULL);
   assert(term_is_valid(term));
   assert(numb_equal(term->constant, numb_zero()));

   for(i = 0; i < term->used; i++)
   {
      assert(!numb_equal(term->elem[i].coeff, numb_zero()));

      var = entry_get_var(term->elem[i].entry);

      xlp_addtonzo(var, con, term->elem[i].coeff);
   }
}

Bool term_to_sos(const Term* term, Sos* sos)
{
   Hash*  hash;
   Var*   var;
   int    i;
   Bool   has_duplicates = FALSE;
   
   Trace("term_to_sos");
   
   assert(sos != NULL);
   assert(term_is_valid(term));
   assert(numb_equal(term->constant, numb_zero()));

   hash = hash_new(HASH_NUMB, term->used);

   assert(hash != NULL);
   
   for(i = 0; i < term->used; i++)
   {
      assert(!numb_equal(term->elem[i].coeff, numb_zero()));

      var = entry_get_var(term->elem[i].entry);

      /* Each weight is allowed only once.
       */
      if (hash_has_numb(hash, term->elem[i].coeff))
         has_duplicates = TRUE;

      hash_add_numb(hash, term->elem[i].coeff);
      xlp_addtosos(sos, var, term->elem[i].coeff);
   }
   hash_free(hash);
   
   return has_duplicates;
}

void term_to_objective(const Term* term)
{
   Var*   var;
   int    i;
   
   Trace("term_to_objective");

   assert(term_is_valid(term));

   /* If we have a constant in the objective, generate an artificial
    * variable @OOffset fixed to the value. This works allways.
    * This is needed because there is no general way for example in
    * MPS format to specify an objective value offset.
    */
   if (!numb_equal(term->constant, numb_zero()))
   {
      const char* format = "%sObjOffset";
   
      Bound* lower = bound_new(BOUND_VALUE, numb_one());
      Bound* upper = bound_new(BOUND_VALUE, numb_one());
      char*  vname = malloc(strlen(SYMBOL_NAME_INTERNAL) + strlen(format) + 1);

      sprintf(vname, format, SYMBOL_NAME_INTERNAL);
      var = xlp_addvar(vname, VAR_CON, lower, upper, numb_zero(), numb_zero());
      xlp_addtocost(var, term->constant);

      free(vname);
      bound_free(upper);
      bound_free(lower);
   }
   
   for(i = 0; i < term->used; i++)
   {
      assert(!numb_equal(term->elem[i].coeff, numb_zero()));

      var = entry_get_var(term->elem[i].entry);
      
      xlp_addtocost(var, term->elem[i].coeff);
   }
}

int term_get_elements(const Term* term)
{
   assert(term_is_valid(term));

   return term->used;
}

Bound* term_get_lower_bound(const Term* term)
{
   Bound*      bound;
   Numb*       lower;
   Numb*       value;
   int         i;
   int         sign;
   
   lower = numb_new_integer(0);
      
   for(i = 0; i < term->used; i++)
   {
      sign = numb_get_sgn(term->elem[i].coeff);

      assert(sign != 0);

      bound = (sign > 0)
         ? xlp_getlower(entry_get_var(term->elem[i].entry))
         : xlp_getupper(entry_get_var(term->elem[i].entry));
      
      if (bound_get_type(bound) != BOUND_VALUE)
         goto finish;

      value = numb_new_mul(bound_get_value(bound), term->elem[i].coeff);

      numb_add(lower, value);
      
      bound_free(bound);
      numb_free(value);
   }
   numb_add(lower, term->constant);

   bound = bound_new(BOUND_VALUE, lower);

 finish:
   numb_free(lower);

   return bound;
}

Bound* term_get_upper_bound(const Term* term)
{
   Bound*      bound;
   Numb*       upper;
   Numb*       value;
   int         i;
   int         sign;
   
   upper = numb_new_integer(0);
      
   for(i = 0; i < term->used; i++)
   {
      sign = numb_get_sgn(term->elem[i].coeff);

      assert(sign != 0);

      bound = (sign > 0)
         ? xlp_getupper(entry_get_var(term->elem[i].entry))
         : xlp_getlower(entry_get_var(term->elem[i].entry));
      
      if (bound_get_type(bound) != BOUND_VALUE)
         goto finish;

      value = numb_new_mul(bound_get_value(bound), term->elem[i].coeff);

      numb_add(upper, value);
      
      bound_free(bound);
      numb_free(value);
   }
   numb_add(upper, term->constant);
   
   bound = bound_new(BOUND_VALUE, upper);

 finish:
   numb_free(upper);

   return bound;
}

Bool term_is_all_integer(const Term* term)
{
   VarClass vc;
   int      i;
   
   for(i = 0; i < term->used; i++)
   {
      vc = xlp_getclass(entry_get_var(term->elem[i].entry));

      if (vc != VAR_INT && vc != VAR_IMP)
         return FALSE;
   }
   return TRUE;
}

#ifndef NDEBUG
void term_print(FILE* fp, const Term* term, int flag)
{
   const Tuple* tuple;
   int          i;
   Numb*        coeff;
   
   assert(term_is_valid(term));

   for(i = 0; i < term->used; i++)
   {
      assert(!numb_equal(term->elem[i].coeff, numb_zero()));

      coeff = numb_copy(term->elem[i].coeff);
      numb_abs(coeff);
      
      fprintf(fp, " %s ", (numb_cmp(term->elem[i].coeff, numb_zero()) >= 0) ? "+" : "-");
      
      if (!numb_equal(coeff, numb_one()))
         fprintf(fp, "%.16g ", numb_todbl(coeff));

      tuple = entry_get_tuple(term->elem[i].entry);
      
      if (flag & TERM_PRINT_SYMBOL)
         tuple_print(fp, tuple);

      numb_free(coeff);
   }
   if (!numb_equal(term->constant, numb_zero()))
   {
      if (numb_cmp(term->constant, numb_zero()) >= 0)
         fprintf(fp, " + %.16g ", numb_todbl(term->constant));
      else
         fprintf(fp, " - %.16g ", -numb_todbl(term->constant));
   }
}
#endif /* !NDEBUG */






