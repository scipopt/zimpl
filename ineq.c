#ident "@(#) $Id: ineq.c,v 1.5 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: ineq.c                                                        */
/*   Name....: Inequality Functions                                          */
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

Bool ineq_is_valid(const Ineq* ineq)
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
      abort();
   }
   fprintf(fp, "%s %.16g", sign, ineq->rhs);
}
