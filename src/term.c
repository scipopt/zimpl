#ident "@(#) $Id: term.c,v 1.6 2001/01/30 19:14:10 thor Exp $"
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

#define TERM_SID  0x5465726d

typedef struct term_element TermElem;

struct term_element
{
   const Symbol* symbol;
   Entry*        entry;
   double        coeff;
   TermElem*     next;
};

struct term
{
   SID
   double    constant;
   TermElem  first;
   TermElem* last;
};

Term* term_new(void)
{
   Term* term = calloc(1, sizeof(*term));

   assert(term != NULL);

   term->constant = 0.0;
   term->last     = &term->first;
   
   SID_set(term, TERM_SID);
   assert(term_is_valid(term));

   return term;
}

void term_add_elem(Term* term, const Symbol* sym, Entry* entry, double coeff)
{
   TermElem* te = calloc(1, sizeof(*te));

   assert(term_is_valid(term));
   assert(symbol_is_valid(sym));
   assert(entry_is_valid(entry));
   assert(NE(coeff, 0.0));
   assert(te  != NULL);

   te->symbol       = sym;
   te->entry        = entry_copy(entry);
   te->coeff        = coeff;
   te->next         = NULL;
   term->last->next = te;
   term->last       = te;
}

static void term_free_elem(Term* term)
{
   TermElem* p;
   TermElem* q;
   
   assert(term_is_valid(term));

   for(p = term->first.next; p != NULL; p = q)
   {
      q = p->next;
      entry_free(p->entry);
      free(p);
   }
   term->last       = &term->first;
   term->first.next = NULL;
}

void term_free(Term* term)
{
   assert(term_is_valid(term));

   term_free_elem(term);
   SID_del(term);
   free(term);
}

Bool term_is_valid(const Term* term)
{
   return ((term != NULL) && SID_ok(term, TERM_SID));
}

Term* term_copy(Term* term)
{
   Term*     tnew = term_new();
   TermElem* te;
   
   assert(term_is_valid(term));
   assert(term_is_valid(tnew));
   
   for(te = term->first.next; te != NULL; te = te->next)
      term_add_elem(tnew, te->symbol, te->entry, te->coeff);

   tnew->constant = term->constant;
   
   return tnew;
}

void term_print(FILE* fp, const Term* term, int flag)
{
   Tuple*    tuple;
   TermElem* te;
   double    coeff;
   
   assert(term_is_valid(term));

   for(te = term->first.next; te != NULL; te = te->next)
   {
      assert(NE(te->coeff, 0.0));

      coeff = fabs(te->coeff);
      
      fprintf(fp, " %s ", GE(te->coeff, 0.0) ? "+" : "-");
      
      if (NE(coeff, 1.0))
         fprintf(fp, "%.16g ", coeff);

      tuple = entry_get_tuple(te->entry);
      
      fprintf(fp, "%s", symbol_get_name(te->symbol));

      if (flag & TERM_PRINT_SYMBOL)
         tuple_print(fp, tuple);

      if (flag & TERM_PRINT_INDEX)
         fprintf(fp, "_%d", entry_get_index(te->entry));      

      tuple_free(tuple);
   }
   if (NE(term->constant, 0.0))
   {
      if (GE(term->constant, 0.0))
         fprintf(fp, " + %.16g ", term->constant);
      else
         fprintf(fp, " - %.16g ", -term->constant);
   }
}

Term* term_add_term(Term* term_a, Term* term_b)
{
   Term*     term;
   TermElem* te;
   
   assert(term_is_valid(term_a));
   assert(term_is_valid(term_b));

   term           = term_new();
   term->constant = term_a->constant + term_b->constant;

   for(te = term_a->first.next; te != NULL; te = te->next)
      term_add_elem(term, te->symbol, te->entry, te->coeff);

   for(te = term_b->first.next; te != NULL; te = te->next)
      term_add_elem(term, te->symbol, te->entry, te->coeff);
   
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
   TermElem* te;

   assert(term_is_valid(term));

   term->constant *= value;

   if (EQ(value, 0.0))
      term_free_elem(term);
   else
   {
      for(te = term->first.next; te != NULL; te = te->next)
         te->coeff *= value;
   }
}

double term_get_constant(Term* term)
{
   assert(term_is_valid(term));
   
   return term->constant;
}

void term_negate(Term* term)
{
   assert(term_is_valid(term));

   term_mul_coeff(term, -1.0);
}








