#pragma ident "@(#) $Id: term.c,v 1.18 2003/08/19 10:11:26 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: term.c                                                        */
/*   Name....: Term Functions                                                */
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

#ifndef NDEBUG
Bool term_is_valid(const Term* term)
{
   return term != NULL && SID_ok(term, TERM_SID) && (term->used <= term->size);
}
#endif /* !NDEBUG */

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

void term_append_term(Term* term_a, const Term* term_b)
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

void term_add_constant(Term* term, const Numb* value)
{
   Trace("term_add_constant");

   assert(term_is_valid(term));

   numb_add(term->constant, value);

   assert(term_is_valid(term));
}

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

void term_to_objective(const Term* term)
{
   Var* var;
   int  i;

   Trace("term_to_objective");

   assert(term_is_valid(term));
   assert(numb_equal(term->constant, numb_zero()));

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
      
      /* fprintf(fp, "%s", symbol_get_name(term->elem[i].symbol));
       */
      if (flag & TERM_PRINT_SYMBOL)
         tuple_print(fp, tuple);
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





