/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: obpwrite.c                                                    */
/*   Name....: OBP Format File Writer                                        */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2003-2025 by Thorsten Koch <koch@zib.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <gmp.h>

#include "zimpl/lint.h"
#include "zimpl/attribute.h"
#include "zimpl/mshell.h"

#include "zimpl/gmpmisc.h"
#include "zimpl/ratlptypes.h"
#include "zimpl/numb.h"
#include "zimpl/bound.h"
#include "zimpl/mme.h"
#include "zimpl/mono.h"
#include "zimpl/term.h"
#include "zimpl/ratlp.h"
#include "zimpl/ratlpstore.h"
#include "zimpl/random.h"

static void write_val(
   FILE*    const fp,
   LpFormat const format,
   bool     const force_sign,
   bool     const do_negate,
   bool     const omit_one,
   mpq_t    const val)
{
   if (omit_one && mpq_equal(val, const_one))
      fprintf(fp, do_negate ? "-" : "+");
   else if (omit_one && mpq_equal(val, const_minus_one))
      fprintf(fp, do_negate ? "+" : "-");
   else
   {
      if (!do_negate)
         fprintf(fp, force_sign ? "%+.15g" : "%.15g", mpq_get_d(val));
      else
      {
         mpq_t t;
         mpq_init(t);
         mpq_neg(t, val);
         fprintf(fp, force_sign ? "%+.15g" : "%.15g", mpq_get_d(t));
         mpq_clear(t);
      }
   }
}

expects_NONNULL
static void write_term(
   FILE*       const fp,
   LpFormat    const format,
   Term const* const term_org,
   char*       const name,
   int         const name_size,
   bool        const is_objective,
   bool        const do_negate)
{
   Term* const term         = term_simplify(term_org);   
   //   int   const term_degree  = term_get_degree(term);
   bool        only_comment = false;

   if (term_has_realfunction(term))
   {
      only_comment = true;

      if (verbose > 0)
      {
         fprintf(stderr, "--- Warning 6XX: File format can only handle polynomial constraints\n");
         fprintf(stderr, "                 Constraint \"%s\" ignored\n", name);
      }
   }
   int cnt = 0;
      
   if (only_comment)
      fprintf(fp, "* ");

   for(int i = 0; i < term_get_elements(term); i++)
   {
      Mono const* const mono   = term_get_element(term, i);
      Numb const* const coeff  = mono_get_coeff(mono);
      MFun        const fun    = mono_get_function(mono);
      int         const degree = mono_get_degree(mono);

      assert(fun == MFUN_NONE);

      mpq_t t;
      mpq_init(t);
      numb_get_mpq(coeff, t);
      fprintf(fp, " ");         
      write_val(fp, format, true, do_negate, true, t);      
      mpq_clear(t);

      fputc(' ', fp);

      for(int k = 0; k < degree; k++)
      {
         Var* var = mono_get_var(mono, k);
         int  j;
         
         if (k > 0)
            fprintf(fp, " "); // *
         
         for(j = 1; k + j < degree; j++)
            if (var != mono_get_var(mono, k + j))
               break;
         
         lps_makename(name, name_size, var->name, var->number);
         
         if (j == 1)
         {
            fprintf(fp, "%s%s", name, (degree == 1) ? "^2" : "");
         }
         else
         {
            fprintf(fp, "%s^%d", name, j);
            k += j - 1; /*lint !e850 loop index variable is modified in body of the loop */
         }
      }
      if (++cnt % 6 == 0)
         fprintf(fp, "\n%s ", only_comment ? "* " : "");         
   }
   term_free(term);
}


static void write_row(
   FILE*       const fp,
   LpFormat    const format,
   Con const*  const con,
   ConType     const type,
   char*       const name,
   int         const name_size)
{
   Nzo*  nzo;
   int   cnt;

   assert(fp        != NULL);
   assert(con       != NULL);
   assert(name      != NULL);

   assert(type == CON_RHS || type == CON_LHS || type == CON_EQUAL);
      
   for(cnt = 0, nzo = con->first; nzo != NULL; nzo = nzo->con_next)
   {
      lps_makename(name, name_size, nzo->var->name, nzo->var->number);

      fprintf(fp, " ");
      write_val(fp, format, true, type == CON_RHS, true, nzo->value);      
      fprintf(fp, " %s", name);

      if (++cnt % 6 == 0)
         fprintf(fp, "\n ");         
   }
   if (con->term != NULL)
      write_term(fp, format, con->term, name, name_size, false, type == CON_RHS);

   switch(type)
   {
   case CON_RHS :
      fprintf(fp, " >= ");
      write_val(fp, format, false, true, false, con->rhs);
      break;
   case CON_LHS :
      fprintf(fp, " >= ");
      write_val(fp, format, false, false, false, con->lhs);
      break;
   case CON_EQUAL :
      fprintf(fp, " = ");
      write_val(fp, format, false, false, false, con->rhs);
      break;
   default :
      abort();
   }
   fprintf(fp, ";\n");
}


/* A specification for the OBP file format can be found in the
 * https://www.cril.univ-artois.fr/PB12/format.pdf
 */
void opb_write(
   Lps const*  lp,
   FILE*       fp,
   LpFormat    format,
   char const* text)
{
   Var const* var;
   Con*       con;
   //bool       have_integer   = false;
   //bool       have_separate  = false;
   //bool       have_checkonly = false;
   int        cnt;
   int        name_size;
   char*      name;

   assert(lp != NULL);
   assert(fp != NULL);
   
   name_size = lps_getnamesize(lp, LP_FORM_LPF);
   name      = malloc((size_t)name_size);

   assert(name != NULL);
   
   if (text != NULL)
      fprintf(fp, "%s", text);   
      
   fprintf(fp, "* Problem name: %s\n", lp->name);

   if (lp->objname != NULL)
   {
      fprintf(fp, "min:\n");
      
      for(var = lp->var_root, cnt = 0; var != NULL; var = var->next)
      {
         /* If cost is zero, do not include in objective function
          */
         if (mpq_equal(var->cost, const_zero))
            continue;

         lps_makename(name, name_size, var->name, format == LP_FORM_HUM ? -1 : var->number);

         fprintf(fp, " ");         
         write_val(fp, format, true, (lp->direct == LP_MAX), true, var->cost);      
         fprintf(fp, " %s", name);

         if (++cnt % 6 == 0)
            fprintf(fp, "\n ");
      }
      if (lp->obj_term != NULL)
         write_term(fp, format, lp->obj_term, name, name_size, true, (lp->direct == LP_MAX));
   }
   /* ---------------------------------------------------------------------- */

   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if (con->size == 0 && con->term == NULL)
         continue;
      
      if (con->type == CON_RANGE)
      {
         /* Split ranges, because OPB format can't handle them.
          */
         lps_makename(name, name_size, con->name, con->number);
         
         write_row(fp, format, con, CON_RHS, name, name_size); /* changes name */
         
         lps_makename(name, name_size, con->name, con->number);
         
         write_row(fp, format, con, CON_LHS, name, name_size); /* changes name */
      }
      else
      {
         lps_makename(name, name_size, con->name, format == LP_FORM_HUM ? -1 : con->number);
         
         write_row(fp, format, con, con->type, name, name_size);
      }
   }

   /* ---------------------------------------------------------------------- */
#if 0
   fprintf(fp, "Bounds\n");

   for(var = lp->var_root; var != NULL; var = var->next)
   {
      /* A variable without any entries in the matrix
       * or the objective function can be ignored.
       * (also not part of an SOS or quadratic constraint)
       */
      if (var->size == 0 && mpq_equal(var->cost, const_zero) && !var->is_used)
         continue;

      lps_makename(name, name_size, var->name, format == LP_FORM_HUM ? -1 : var->number);

      if (var->type == VAR_FIXED)
      {
         fprintf(fp, " %s = ", name);
         write_val(fp, format, false, var->lower);      
         fprintf(fp, "\n");
      }
      else
      {
         /* Check if we have integer variables
          */
         if (var->vclass == VAR_INT)
            have_integer = true;
         
         fprintf(fp, " ");

         if (var->type == VAR_LOWER || var->type == VAR_BOXED)
            write_val(fp, format, false, var->lower);      
         else
            fprintf(fp, "-inf");
         
         fprintf(fp, " <= %s <= ", name);
         
         if (var->type == VAR_UPPER || var->type == VAR_BOXED)
         {
            write_val(fp, format, false, var->upper);      
            fprintf(fp, "\n");
         }
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
         
         lps_makename(name, name_size, var->name, format == LP_FORM_HUM ? -1 : var->number);

         fprintf(fp, " %s\n", name);
      }
   }

   /* ---------------------------------------------------------------------- */

   if (lps_has_sos(lp))
   {
      Sos const* sos;
      Sse const* sse;

      fprintf(fp, "SOS\n");

      for(sos = lp->sos_root; sos != NULL; sos = sos->next)
      {
         cnt = 0;

         fprintf(fp, " %s:S%d:: ", 
            sos->name,
            sos->type == SOS_TYPE1 ? 1 : 2);
    
         for(sse = sos->first; sse != NULL; sse = sse->next)
         {
            lps_makename(name, name_size, sse->var->name, format == LP_FORM_HUM ? -1 : sse->var->number);

            fprintf(fp, " %s:", name);
            write_val(fp, format, false, sse->weight);      

            if (++cnt % 6 == 0)
               fputc('\n', fp);         
         }
         if (cnt % 6 != 0)
            fputc('\n', fp);         
      }
   }
   fprintf(fp, "End\n");
#endif
   
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
