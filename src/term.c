#ident "@(#) $Id: term.c,v 1.1 2001/01/26 07:11:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: term.c                                                        */
/*   Name....: Term Functions                                                */
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

#define TERM_SID  0x5465726d

typedef struct term_element TermElem;

struct term_element
{
   const Symbol* symbol;
   int           index;
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

void term_add_elem(Term* term, const Symbol* sym, int idx, double coeff)
{
   TermElem* te = calloc(1, sizeof(*te));

   assert(te  != NULL);
   assert(term_is_valid(term));
   assert(sym != NULL);
   assert(idx >= 0);
   assert(NE(coeff, 0.0));

   te->symbol       = sym;
   te->index        = idx;
   te->coeff        = coeff;
   te->next         = NULL;
   term->last->next = te;
   term->last       = te;
}

void term_free(Term* term)
{
   TermElem* p;
   TermElem* q;
   
   assert(term_is_valid(term));

   for(p = term->first.next; p != NULL; p = q)
   {
      q = p->next;
      free(p);
   }
   SID_del(term);
   free(term);
}

int term_is_valid(const Term* term)
{
   return ((term != NULL) && SID_ok(term, TERM_SID));
}

Term* term_copy(Term* term)
{
   Term*     tnew = term_new();
   TermElem* enew;
   TermElem* te;
   
   assert(term_is_valid(term));
   assert(term_is_valid(tnew));
   
   for(te = term->first.next; te != NULL; te = te->next)
      term_add_elem(tnew, te->symbol, te->index, te->coeff);

   tnew->constant = term->constant;
   
   return tnew;
}

void term_print(FILE* fp, const Term* term)
{
   TermElem* te;
   
   assert(term_is_valid(term));

   for(te = term->first.next; te != NULL; te = te->next)
   {
      assert(NE(te->coeff, 0.0));

      if (GE(te->coeff, 0.0))
         fprintf(fp, " + %g ", te->coeff);
      else
         fprintf(fp, " - %g ", -te->coeff);
         
      fprintf(fp, "%s_%d", symbol_get_name(te->symbol), te->index);
   }
   if (NE(term->constant, 0.0))
   {
      if (GE(term->constant, 0.0))
         fprintf(fp, " + %g ", term->constant);
      else
         fprintf(fp, " - %g ", -term->constant);
   }
}

Term* term_add_term(Term* term_a, Term* term_b)
{
   Term*     term;
   TermElem* te;
   
   assert(term_is_valid(term_a));
   assert(term_is_valid(term_b));

   term = term_new();
   term->constant = term_a->constant + term_b->constant;

   for(te = term_a->first.next; te != NULL; te = te->next)
      term_add_elem(term, te->symbol, te->index, te->coeff);

   for(te = term_b->first.next; te != NULL; te = te->next)
      term_add_elem(term, te->symbol, te->index, te->coeff);
   
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

   for(te = term->first.next; te != NULL; te = te->next)
      te->coeff *= value; 
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








