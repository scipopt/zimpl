#pragma ident "@(#) $Id: ratrlpwrite.c,v 1.4 2007/08/01 10:17:14 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: rlpwrite.c                                                    */
/*   Name....: Renumbered permutated LP Format File Writer                   */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2007 by Thorsten Koch <koch@zib.de>
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
#include "random.h"

static void permute(int size, void** tab)
{
   int i;

   if (size < 3)
      return;

   assert(size >= 3);
   
   for(i = 0; i < size; i++)
   {
      void* t;
      int   a = rand_get_range(0, size - 1);
      int   b = rand_get_range(0, size - 1);

      assert(a >= 0);
      assert(a <  size);
      assert(b >= 0);
      assert(b <  size);
      
      t      = tab[a];
      tab[a] = tab[b];
      tab[b] = t;
   }
}

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
   Nzo*  nzo;
   Nzo** nzotab;
   int   cnt = 0;
   int   i;
   
   assert(fp        != NULL);
   assert(con       != NULL);
   assert(name      != NULL);

   nzotab = calloc((size_t)con->size, sizeof(*con));

   assert(nzotab != NULL);

   for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
      nzotab[cnt++] = nzo;

   assert(cnt == con->size);
   
   permute(con->size, (void**)nzotab);

   for(i = 0; i < con->size; i++)
   {
      nzo = nzotab[i];

      lps_makename(name, name_size, nzo->var->name, nzo->var->number);

      if (mpq_equal(nzo->value, const_one))
         fprintf(fp, " + %s", name);
      else if (mpq_equal(nzo->value, const_minus_one))
         fprintf(fp, " - %s", name);
      else
         fprintf(fp, " %+.15g %s", mpq_get_d(nzo->value), name);
      
      if ((i + 1) % 6 == 0)
         fprintf(fp, "\n ");         
   }
   free(nzotab);
}

/* A specification for the LP file format can be found in the
 * ILOG CPLEX 7.0 Reference Manual page 527.
 * ILOG CPLEX 8.0 Reference Manual page 595.
 * The "Lazy Constraints" section seems to be undocumented.
 */
void rlp_write(
   const Lps*  lp,
   FILE*       fp,
   const char* text)
{
   const Var*  var;
   Con*        con;
   Con**       contab;
   
   Bool  have_integer  = FALSE;
   Bool  have_separate = FALSE;
   int   cnt;
   int   i;
   int   k;
   int   name_size;
   char* name;
   
   assert(lp != NULL);
   assert(fp != NULL);

   /* Store constraint pointers and permute them
    */
   contab = calloc((size_t)lp->cons, sizeof(*contab));

   assert(contab != NULL);

   k = 0;
   for(con = lp->con_root; con != NULL; con = con->next)
      contab[k++] = con;

   assert(k == lp->cons);

   permute(lp->cons, (void**)contab);

   /* Now do as usual
    */
   
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
    * lazy constraints, if any.
    */
   for(i = 0; i < 2; i++)
   {      
      fprintf(fp, "\n%s\n", (i == 0) ? "Subject to" : "Lazy Constraints");

      for(k = 0; k < lp->cons; k++)
      {
         con = contab[k];
         
         if (con->size == 0)
            continue;

         if ((con->flags & LP_FLAG_CON_SEPAR) == LP_FLAG_CON_SEPAR)
         {
            have_separate = TRUE;

            if (i == 0)
               continue;
         }
         else if (i == 1)
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
            fprintf(fp, " %s:\n ", name);

            write_row(fp, con, name, name_size);
            write_rhs(fp, con, con->type);
         }
      }
      /* Shortcut to break out of the loop if there are no lazy constraints.
       */
      if (!have_separate)
         break;
   }

   /* ---------------------------------------------------------------------- */

   fprintf(fp, "Bounds\n");

   for(var = lp->var_root; var != NULL; var = var->next)
   {
      /* A variable without any entries in the matrix
       * or the objective function can be ignored.
       */
      if (var->size == 0 && mpq_equal(var->cost, const_zero))
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
            fprintf(fp, " -Inf");
         
         fprintf(fp, " <= %s <= ", name);
         
         if (var->type == VAR_UPPER || var->type == VAR_BOXED)
            fprintf(fp, "%.15g\n", mpq_get_d(var->upper));
         else
            fprintf(fp, "+Inf\n");
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

         if (var->size == 0 && mpq_equal(var->cost, const_zero))
            continue;
         
         lps_makename(name, name_size, var->name, var->number);

         fprintf(fp, " %s\n", name);
      }
   }
   fprintf(fp, "End\n");

   free(name);
   free(contab);
}   

/* ------------------------------------------------------------------------- */
/* Emacs Local Variables:                                                    */
/* Emacs mode:c                                                              */
/* Emacs c-basic-offset:3                                                    */
/* Emacs tab-width:8                                                         */
/* Emacs indent-tabs-mode:nil                                                */
/* Emacs End:                                                                */
/* ------------------------------------------------------------------------- */