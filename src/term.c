#ident "@(#) $Id: term.c,v 1.12 2003/02/17 16:13:48 bzfkocht Exp $"
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

#include "portab.h"
#include "mshell.h"
#include "mme.h"

#define TERM_EXTEND_SIZE 16
#define TERM_SID         0x5465726d

typedef struct term_element TermElem;

struct term_element
{
   const Entry* entry;
   double       coeff;
};

struct term
{
   SID
   double    constant;
   int       size;
   int       used;
   TermElem* elem;
};

Term* term_new(int size)
{
   Term* term = calloc(1, sizeof(*term));

   assert(term != NULL);
   assert(size >  0);
   
   term->constant = 0.0;
   term->size     = size;
   term->used     = 0;
   term->elem     = calloc(term->size, sizeof(*term->elem));
   
   SID_set(term, TERM_SID);
   assert(term_is_valid(term));

   return term;
}

void term_add_elem(Term* term, const Entry* entry, double coeff)
{
   assert(term_is_valid(term));
   assert(entry_is_valid(entry));
   assert(NE(coeff, 0.0));
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
   term->elem[term->used].coeff = coeff;
   term->used++;
}

void term_free(Term* term)
{
   assert(term_is_valid(term));

   SID_del(term);

   free(term->elem);
   free(term);
}

#ifndef NDEBUG
Bool term_is_valid(const Term* term)
{
   return ((term != NULL)
      && SID_ok(term, TERM_SID)
      && (term->used <= term->size));
}
#endif /* !NDEBUG */

Term* term_copy(const Term* term)
{
   Term* tnew = term_new(term->size);
   
   assert(term_is_valid(term));
   assert(term_is_valid(tnew));

   memcpy(tnew->elem, term->elem, (size_t)term->used * sizeof(*term->elem));

   tnew->used     = term->used;
   tnew->constant = term->constant;
   
   return tnew;
}

void term_append_term(Term* term_a, const Term* term_b)
{
   int i;
   
   assert(term_is_valid(term_a));
   assert(term_is_valid(term_b));

   term_a->constant += term_b->constant;

   for(i = 0; i < term_b->used; i++)
      term_add_elem(term_a, term_b->elem[i].entry, term_b->elem[i].coeff);

   assert(term_is_valid(term_a));
}

Term* term_add_term(const Term* term_a, const Term* term_b)
{
   Term* term;
   
   assert(term_is_valid(term_a));
   assert(term_is_valid(term_b));

   term           = term_new(term_a->size + term_b->size);
   term->used     = term_a->used + term_b->used;
   term->constant = term_a->constant + term_b->constant;

   assert(term->size >= term->used);

   memcpy(&term->elem[0], term_a->elem,
      (size_t)term_a->used * sizeof(*term->elem));

   memcpy(&term->elem[term_a->used], term_b->elem,
      (size_t)term_b->used * sizeof(*term->elem));

   assert(term_is_valid(term));

   return term;
}

Term* term_sub_term(const Term* term_a, const Term* term_b)
{
   Term* term;
   int   i;
   
   assert(term_is_valid(term_a));
   assert(term_is_valid(term_b));

   term           = term_new(term_a->size + term_b->size);
   term->constant = term_a->constant - term_b->constant;

   assert(term->size >= term->used);

   for(i = 0; i < term_a->used; i++)
      term_add_elem(term, term_a->elem[i].entry, term_a->elem[i].coeff);

   for(i = 0; i < term_b->used; i++)
      term_add_elem(term, term_b->elem[i].entry, -term_b->elem[i].coeff);

   assert(term_is_valid(term));

   return term;
}

void term_add_constant(Term* term, double value)
{
   assert(term_is_valid(term));

   term->constant += value;
}

void term_mul_coeff(Term* term, double value)
{
   int i;

   assert(term_is_valid(term));

   term->constant *= value;

   if (EQ(value, 0.0))
      term->used = 0;
   else
   {
      for(i = 0; i < term->used; i++)
         term->elem[i].coeff *= value;
   }
}

double term_get_constant(const Term* term)
{
   assert(term_is_valid(term));
   
   return term->constant;
}

void term_negate(Term* term)
{
   assert(term_is_valid(term));

   term_mul_coeff(term, -1.0);
}

void term_to_nzo(const Term* term, Con* con)
{
   Var*   var;
   Nzo*   nzo;
   double value;
   int    i;
   
   assert(con != NULL);
   assert(term_is_valid(term));
   assert(EQ(term->constant, 0.0));

   for(i = 0; i < term->used; i++)
   {
      assert(NE(term->elem[i].coeff, 0.0));

      var = entry_get_var(term->elem[i].entry);
      nzo = lps_getnzo(con, var);

      if (nzo == NULL)
         lps_addnzo(con, var, term->elem[i].coeff);
      else
      {
         value = lps_getval(nzo) + term->elem[i].coeff;

         if (EQ(value, 0.0))
            lps_delnzo(nzo);
         else
            lps_setval(nzo, value);
      }
   }
}

void term_to_objective(const Term* term)
{
   Var* var;
   int  i;
   
   assert(term_is_valid(term));
   assert(EQ(term->constant, 0.0));

   for(i = 0; i < term->used; i++)
   {
      assert(NE(term->elem[i].coeff, 0.0));

      var = entry_get_var(term->elem[i].entry);
      
      lps_setcost(var, lps_getcost(var) + term->elem[i].coeff);
   }
}

#ifndef NDEBUG
void term_print(FILE* fp, const Term* term, int flag)
{
   const Tuple* tuple;
   int          i;
   double       coeff;
   
   assert(term_is_valid(term));

   for(i = 0; i < term->used; i++)
   {
      assert(NE(term->elem[i].coeff, 0.0));

      coeff = fabs(term->elem[i].coeff);
      
      fprintf(fp, " %s ", GE(term->elem[i].coeff, 0.0) ? "+" : "-");
      
      if (NE(coeff, 1.0))
         fprintf(fp, "%.16g ", coeff);

      tuple = entry_get_tuple(term->elem[i].entry);
      
      /* fprintf(fp, "%s", symbol_get_name(term->elem[i].symbol));
       */
      if (flag & TERM_PRINT_SYMBOL)
         tuple_print(fp, tuple);
   }
   if (NE(term->constant, 0.0))
   {
      if (GE(term->constant, 0.0))
         fprintf(fp, " + %.16g ", term->constant);
      else
         fprintf(fp, " - %.16g ", -term->constant);
   }
}
#endif /* !NDEBUG */





