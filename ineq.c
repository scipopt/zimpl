#ident "@(#) $Id: ineq.c,v 1.2 2001/01/28 19:16:13 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: ineq.c                                                        */
/*   Name....: Inequality Functions                                          */
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

#define INEQ_SID  0x496e6571

struct inequality
{
   SID
   int      refc;
   IneqType type;
   double   rhs;
   Term*    term;
};

Ineq* ineq_new(IneqType type, Term* term, double rhs)
{
   Ineq*  ineq = calloc(1, sizeof(*ineq));

   assert(term != NULL);
   assert(ineq != NULL);

   ineq->refc = 1;
   ineq->rhs  = rhs;
   ineq->type = type;
   ineq->term = term_copy(term);

   SID_set(ineq, INEQ_SID);
   assert(ineq_is_valid(ineq));

   return ineq;
}

void ineq_free(Ineq* ineq)
{
   assert(ineq_is_valid(ineq));

   ineq->refc--;

   if (ineq->refc == 0)
   {
      SID_del(ineq);
   
      term_free(ineq->term);
      free(ineq);
   }
}

int ineq_is_valid(const Ineq* ineq)
{
   return ((ineq != NULL) && SID_ok(ineq, INEQ_SID) && (ineq->refc > 0));
}

Ineq* ineq_copy(Ineq* ineq)
{
   assert(ineq_is_valid(ineq));

   ineq->refc++;

   return ineq;
}

void ineq_print(FILE* fp, const Ineq* ineq)
{
   const char* sign;
   
   assert(ineq_is_valid(ineq));

   term_print(fp, ineq->term, TERM_PRINT_INDEX);
   
   switch(ineq->type)
   {
   case INEQ_EQ :
      sign = " =  ";
      break;
   case INEQ_LE :
      sign = " <= ";
      break;
   case INEQ_GE :
      sign = " >= ";
      break;
   default :
      assert(0);
   }
   fprintf(fp, "%s %g", sign, ineq->rhs);
}
