#pragma ident "@(#) $Id: ratlpfwrite.c,v 1.1 2003/07/12 15:24:02 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: lpfwrite.c                                                    */
/*   Name....: LP Format File Writer                                         */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2003 by Thorsten Koch <koch@zib.de>
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

#include <gmp.h>

#include "bool.h"

#include "ratlp.h"
#include "ratlpstore.h"

/* A specification for the LP file format can be found in the
 * ILOG CPLEX 7.0 Reference Manual page 527.
 */
void lpf_write(
   const Lps* lp,
   const char* filename)
{
   FILE*      fp;
   const Var* var;
   const Con* con;
   const Nzo* nzo;
   int   cnt;
   mpq_t zero;
   mpq_t one;
   mpq_t minus_one;

   assert(lp       != NULL);
   assert(filename != NULL);

   if (NULL == (fp = fopen(filename, "w")))
   {
      perror(filename);
      abort();
   }
   mpq_init(zero);
   mpq_init(one);
   mpq_init(minus_one);

   mpq_set_ui(one, 1, 1); /* = 1 */
   mpq_set_si(minus_one, -1, 1); /* = -1 */
   
   fprintf(fp, "\\Problem name: %s\n", lp->name);   
   fprintf(fp, "%s\n", (lp->direct == LP_MIN) ? "Minimize" : "Maximize");
   fprintf(fp, " %s: ", lp->objname == NULL ? "Objective" : lp->objname);
   
   for(var = lp->var_root, cnt = 0; var != NULL; var = var->next)
   {
      /* If cost is zero, do not include in objective function
       */
      if (mpq_equal(var->cost, zero))
         continue;

      if (mpq_equal(var->cost, one))
         fprintf(fp, " + %s", var->name);
      else if (mpq_equal(var->cost, minus_one))
         fprintf(fp, " - %s", var->name);
      else
         fprintf(fp, " %+.15g %s", mpq_get_d(var->cost), var->name);
      
      if (++cnt % 6 == 0)
         fprintf(fp, "\n ");
   }
   /* ---------------------------------------------------------------------- */

   fprintf(fp, "\nSubject to\n");

   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if (con->size == 0)
         continue;

      fprintf(fp, " %s:\n ", con->name);

      if (con->type == CON_RANGE)
         fprintf(fp, " %.15g <= ", mpq_get_d(con->lhs));
         
      for(nzo = con->first, cnt = 0; nzo != NULL; nzo = nzo->con_next)
      {
         if (mpq_equal(nzo->value, one))
            fprintf(fp, " + %s", nzo->var->name);
         else if (mpq_equal(nzo->value, minus_one))
            fprintf(fp, " - %s", nzo->var->name);
         else
            fprintf(fp, " %+.15g %s", mpq_get_d(nzo->value), nzo->var->name);
            
         if (++cnt % 6 == 0)
            fprintf(fp, "\n ");         
      }
      switch(con->type)
      {
      case CON_RHS :
         fprintf(fp, " <= %.15g\n", mpq_get_d(con->rhs));
         break;
      case CON_LHS :
         fprintf(fp, " >= %.15g\n", mpq_get_d(con->lhs));
         break;
      case CON_EQUAL :
         fprintf(fp, " = %.15g\n", mpq_get_d(con->rhs));
         break;
      case CON_RANGE :
         fprintf(fp, " <= %.15g\n", mpq_get_d(con->rhs));
         break;
      default :
         abort();
      }
   }

   /* ---------------------------------------------------------------------- */

   fprintf(fp, "Bounds\n");

   for(var = lp->var_root; var != NULL; var = var->next)
   {
      if (var->size > 0)
      {
         if (var->type == VAR_FIXED)
            fprintf(fp, " %s = %.15g", var->name, mpq_get_d(var->lower));
         else
         {
            if (var->type == VAR_LOWER || var->type == VAR_BOXED)
               fprintf(fp, " %.15g", mpq_get_d(var->lower));
            else
               fprintf(fp, " -Inf");

            fprintf(fp, " <= %s <= ", var->name);

            if (var->type == VAR_UPPER || var->type == VAR_BOXED)
               fprintf(fp, "%.15g\n", mpq_get_d(var->upper));
            else
               fprintf(fp, "+Inf\n");
         }
      }
   }
   fprintf(fp, "End\n");

   fclose(fp);
   
   mpq_clear(zero);
   mpq_clear(one);
   mpq_clear(minus_one);
}   

/* ------------------------------------------------------------------------- */
/* Emacs Local Variables:                                                    */
/* Emacs mode:c                                                              */
/* Emacs c-basic-offset:3                                                    */
/* Emacs tab-width:8                                                         */
/* Emacs indent-tabs-mode:nil                                                */
/* Emacs End:                                                                */
/* ------------------------------------------------------------------------- */
