#pragma ident "@(#) $Id: ratlpfwrite.c,v 1.18 2009/09/13 16:15:55 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: lpfwrite.c                                                    */
/*   Name....: LP Format File Writer                                         */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2003-2009 by Thorsten Koch <koch@zib.de>
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
#include "gmpmisc.h"
#include "ratlp.h"
#include "ratlpstore.h"

static void write_rhs(FILE* fp, const Con* con, ConType type)
{
   assert(fp  != NULL);
   assert(con != NULL);
   
   switch(type)
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
      abort();
   default :
      abort();
   }
}

static void write_row(
   FILE* fp,
   const Con* con,
   char* name,
   int   name_size)
{
   const Nzo* nzo;
   int        cnt = 0;

   assert(fp        != NULL);
   assert(con       != NULL);
   assert(name      != NULL);
   
   for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
   {
      lps_makename(name, name_size, nzo->var->name, nzo->var->number);

      if (mpq_equal(nzo->value, const_one))
         fprintf(fp, " + %s", name);
      else if (mpq_equal(nzo->value, const_minus_one))
         fprintf(fp, " - %s", name);
      else
         fprintf(fp, " %+.15g %s", mpq_get_d(nzo->value), name);
      
      if (++cnt % 6 == 0)
         fprintf(fp, "\n ");         
   }
   if (con->qme_first != NULL)
   {
      Qme* qme;

      if (cnt % 6 != 0)
         fprintf(fp, "\n ");         

      fprintf(fp, " + [");

      for(qme = con->qme_first; qme != NULL; qme= qme->next)
      {
         lps_makename(name, name_size, qme->var1->name, qme->var1->number);

         if (mpq_equal(qme->value, const_one))
            fprintf(fp, " + %s", name);
         else if (mpq_equal(qme->value, const_minus_one))
            fprintf(fp, " - %s", name);
         else
            fprintf(fp, " %+.15g %s", mpq_get_d(qme->value), name);

         if (qme->var1 == qme->var2)
            fprintf(fp, "^2");
         else
         {
            lps_makename(name, name_size, qme->var2->name, qme->var2->number);

            fprintf(fp, " * %s", name);
         }
      }
      fprintf(fp,  " ]\n");
   }
}

/* A specification for the LP file format can be found in the
 * ILOG CPLEX 7.0 Reference Manual page 527.
 * ILOG CPLEX 8.0 Reference Manual page 595.
 * The "Lazy Constraints" section seems to be undocumented.
 */
void lpf_write(
   const Lps*  lp,
   FILE*       fp,
   const char* text)
{
   const Var* var;
   const Con* con;
   Bool  have_integer   = FALSE;
   Bool  have_separate  = FALSE;
   Bool  have_checkonly = FALSE;
   int   cnt;
   int   i;
   int   name_size;
   char* name;

   assert(lp != NULL);
   assert(fp != NULL);

   name_size = lps_getnamesize(lp, LP_FORM_LPF);
   name      = malloc((size_t)name_size);

   assert(name != NULL);
   
   if (text != NULL)
      fprintf(fp, "%s", text);   
      
   fprintf(fp, "\\Problem name: %s\n", lp->name);   
   fprintf(fp, "%s\n", (lp->direct == LP_MIN) ? "Minimize" : "Maximize");
   fprintf(fp, " %s: ", lp->objname == NULL ? "Objective" : lp->objname);
   
   for(var = lp->var_root, cnt = 0; var != NULL; var = var->next)
   {
      /* If cost is zero, do not include in objective function
       */
      if (mpq_equal(var->cost, const_zero))
         continue;

      lps_makename(name, name_size, var->name, var->number);
      
      if (mpq_equal(var->cost, const_one))
         fprintf(fp, " + %s", name);
      else if (mpq_equal(var->cost, const_minus_one))
         fprintf(fp, " - %s", name);
      else
         fprintf(fp, " %+.15g %s", mpq_get_d(var->cost), name);

      if (++cnt % 6 == 0)
         fprintf(fp, "\n ");
   }
   /* ---------------------------------------------------------------------- */

   /* First loop run for normal constraints, second one for
    * user cuts, thrid one for lazy constraints, if any.
    */
   for(i = 0; i < 3; i++)
   {      
      if (i == 0)
         fprintf(fp, "\nSubject to\n");      
      else if (i == 1)
      {
         if (!have_separate)
            continue;
         else
            fprintf(fp, "\nUser Cuts\n");
      }
      else if (i == 2)
      {
         if (!have_checkonly)
            continue;
         else
            fprintf(fp, "\nLazy Constraints\n"); 
      }

      for(con = lp->con_root; con != NULL; con = con->next)
      {
         if (con->size == 0 && con->qme_first == NULL)
            continue;

         if (i == 0 && ((con->flags & (LP_FLAG_CON_SEPAR | LP_FLAG_CON_CHECK)) != 0))
         {
            if ((con->flags & LP_FLAG_CON_SEPAR) == LP_FLAG_CON_SEPAR)
               have_separate = TRUE;
            if ((con->flags & LP_FLAG_CON_CHECK) == LP_FLAG_CON_CHECK)
               have_checkonly = TRUE;

            continue;
         }        
         if (i == 1 && (con->flags & LP_FLAG_CON_SEPAR) != LP_FLAG_CON_SEPAR)
            continue;

         if (i == 2 && (con->flags & LP_FLAG_CON_CHECK) != LP_FLAG_CON_CHECK)
            continue;
    
         if (con->type == CON_RANGE)
         {
            /* Split ranges, because LP format can't handle them.
             */
            lps_makename(name, name_size, con->name, con->number);
            fprintf(fp, " %sR:\n ", name);

            write_row(fp, con, name, name_size); /* changes name */
            write_rhs(fp, con, CON_RHS);

            lps_makename(name, name_size, con->name, con->number);
            fprintf(fp, " %sL:\n ", name);

            write_row(fp, con, name, name_size); /* changes name */
            write_rhs(fp, con, CON_LHS);
         }
         else
         {
            lps_makename(name, name_size, con->name, con->number);

            if (i == 0)
               fprintf(fp, " %s:\n ", name);
            else if (i == 1)
               fprintf(fp, " %sU:\n ", name);
            else if (i == 2)
               fprintf(fp, " %sZ:\n ", name);

            if (con->ind_var != NULL)
            {
               lps_makename(name, name_size, con->ind_var->name, con->ind_var->number);
               fprintf(fp, "%s = %d -> ", name, con->ind_dir ? 1 : 0);
            }
            write_row(fp, con, name, name_size);
            write_rhs(fp, con, con->type);
         }
      }
   }

   /* ---------------------------------------------------------------------- */

   fprintf(fp, "Bounds\n");

   for(var = lp->var_root; var != NULL; var = var->next)
   {
      /* A variable without any entries in the matrix
       * or the objective function can be ignored.
       * (also not part of an SOS or quadratic constraint)
       */
      if (var->size == 0 && mpq_equal(var->cost, const_zero) && !var->is_used)
         continue;

      lps_makename(name, name_size, var->name, var->number);

      if (var->type == VAR_FIXED)
         fprintf(fp, " %s = %.15g\n", name, mpq_get_d(var->lower));
      else
      {
         /* Check if we have integer variables
          */
         if (var->vclass == VAR_INT)
            have_integer = TRUE;
         
         if (var->type == VAR_LOWER || var->type == VAR_BOXED)
            fprintf(fp, " %.15g", mpq_get_d(var->lower));
         else
            fprintf(fp, " -inf");
         
         fprintf(fp, " <= %s <= ", name);
         
         if (var->type == VAR_UPPER || var->type == VAR_BOXED)
            fprintf(fp, "%.15g\n", mpq_get_d(var->upper));
         else
            fprintf(fp, "+inf\n");
      }
   }

   /* ---------------------------------------------------------------------- */

   if (have_integer)
   {
      fprintf(fp, "General\n");
      
      for(var = lp->var_root; var != NULL; var = var->next)
      {
         if (var->vclass != VAR_INT)
            continue;

         if (var->size == 0 && mpq_equal(var->cost, const_zero) && !var->is_used)
            continue;
         
         lps_makename(name, name_size, var->name, var->number);

         fprintf(fp, " %s\n", name);
      }
   }

   /* ---------------------------------------------------------------------- */

   if (lps_has_sos(lp))
   {
      const Sos* sos;
      const Sse* sse;

      fprintf(fp, "SOS\n");

      for(sos = lp->sos_root; sos != NULL; sos = sos->next)
      {
         cnt = 0;

         fprintf(fp, " %s:S%d:: ", 
            sos->name,
            sos->type == SOS_TYPE1 ? 1 : 2);
    
         for(sse = sos->first; sse != NULL; sse = sse->next)
         {
            lps_makename(name, name_size, sse->var->name, sse->var->number);

            fprintf(fp, " %s:%.10g", name, mpq_get_d(sse->weight));

            if (++cnt % 6 == 0)
               fputc('\n', fp);         
         }
         if (cnt % 6 != 0)
            fputc('\n', fp);         
      }
   }
   fprintf(fp, "End\n");

   free(name);
}   

/* ------------------------------------------------------------------------- */
/* Emacs Local Variables:                                                    */
/* Emacs mode:c                                                              */
/* Emacs c-basic-offset:3                                                    */
/* Emacs tab-width:8                                                         */
/* Emacs indent-tabs-mode:nil                                                */
/* Emacs End:                                                                */
/* ------------------------------------------------------------------------- */
