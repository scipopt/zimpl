#pragma ident "@(#) $Id: ratmpswrite.c,v 1.2 2003/07/16 13:32:08 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mpswrite.c                                                    */
/*   Name....: MPS File Write                                                */
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
#include <ctype.h>
#include <assert.h>

#include <gmp.h>

#include "bool.h"

#include "ratlp.h"
#include "ratlpstore.h"

#if 0
#include "mshell.h"
#include "portab.h"
#include "mme.h"
#include "lpstore.h"
#endif

#define MPS_NAME_LEN  8

static void write_data(
   FILE*        fp,
   Bool         use_field5,
   const int    indicator1,
   const int    indicator2,
   const char*  name1,
   const char*  name2,
   const mpq_t  value)
{
   assert(name1 != NULL);
   assert(name2 != NULL);
   
   if (use_field5)
   {
      fprintf(fp, " %c%c %-8.8s  %-8.8s  %12g\n",
         toupper(indicator1), toupper(indicator2), name1, name2, mpq_get_d(value));
   }
   else
   {
      fprintf(fp, " %c%c %-8.8s  %-8.8s\n",
         toupper(indicator1), toupper(indicator2), name1, name2);
   }
}   

static void write_vars(
   const Lps* lp,
   FILE*      fp,
   VarClass   varclass)
{
   const Var*  var;
   const Nzo*  nzo;
   char  vtmp  [13];
   char  ctmp  [13];
   mpq_t zero;
   mpq_t temp;
   
   assert(lp != NULL);
   assert(fp != NULL);

   mpq_init(zero);
   mpq_init(temp);
   
   for(var = lp->var_root; var != NULL; var = var->next)
   {
      if (var->class != varclass)
         continue;

      /* Only variables with a cost not equal zero, need to be included
       * in the objective function
       */
      if (!mpq_equal(var->cost, zero))
      {
#if 0    /* We should do this in the preprocessing step
          * otherwise we have problems in case of variable
          * that have only bounds, but are not part of any
          * constraint.
          */
         /* If the variable not free or when minimizing the lower or
          * when maximizing the upper bound is not zero,
          * we have to include it.
          */
         if ((var->size != 0) || (fabs((lp->direct == LP_MIN) ?
            var->lower : var->upper) > EPS_ZERO))
         
#endif
         lps_makename(vtmp, MPS_NAME_LEN + 1, var->name, var->number);

         if (lp->direct == LP_MIN)
            mpq_set(temp, var->cost);
         else
         {
            mpq_neg(temp, var->cost);

            fprintf(stderr, "%s\n%s\n",
               "*** Warning: Objective function inverted to make",
               "             minimization problem for MPS output\n");
         }
         write_data(fp, TRUE, ' ', ' ', vtmp, "OBJECTIV", temp);
      }
      for(nzo = var->first; nzo != NULL; nzo = nzo->var_next)
      {
         assert(nzo->var == var);
         assert(!mpq_equal(nzo->value, zero));

         lps_makename(vtmp, MPS_NAME_LEN + 1, var->name, var->number);
         lps_makename(ctmp, MPS_NAME_LEN + 1, nzo->con->name, nzo->con->number);

         write_data(fp, TRUE, ' ', ' ', vtmp, ctmp, nzo->value);
      }
   }   
   mpq_clear(zero);
   mpq_clear(temp);
}

void mps_write(
   const Lps*  lp,
   FILE*       fp,
   const char* text)
{
   const Var*  var;
   const Con*  con;
   char  vtmp  [13];
   char  ctmp  [13];
   mpq_t zero;
   int   indicator;
   
   assert(lp != NULL);
   assert(fp != NULL);

   mpq_init(zero);

   if (text != NULL)
      fprintf(fp, "* %s\n", text);
   
   fprintf(fp, "NAME        %8.8s\n", lp->name);
   fprintf(fp, "ROWS\n");
   
   write_data(fp, FALSE, 'N', ' ', "OBJECTIV", "", zero);

   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if (con->size > 0)
      {
         lps_makename(ctmp, MPS_NAME_LEN + 1, con->name, con->number);

         switch(lps_contype(con))
         {
         case CON_EQUAL:
            indicator = 'N';
            break;
         case CON_LHS:
            indicator = 'G';
            break;
         case CON_RHS:
            indicator = 'L';
            break;
         case CON_RANGE:
            indicator = 'L';
            fprintf(stderr, "*** WARNING: Ranges not yet supported\n");
            break;
         default :
            abort();
         }
         write_data(fp, FALSE, indicator, ' ', ctmp, "", zero);
      }
   }
   fprintf(fp, "COLUMNS\n");

   /* First integer/binary variables
    */
#if 0 /* ??? Would be nice if known in advance */
   if (lp->type != LP_LP)
#endif 
   {
      fprintf(fp, "    MARK0000  'MARKER'                 'INTORG'\n");

      write_vars(lp, fp, VAR_INT);
      write_vars(lp, fp, VAR_BIN);

      fprintf(fp, "    MARK0001  'MARKER'                 'INTEND'\n");
   }
   write_vars(lp, fp, VAR_CON);

   fprintf(fp, "RHS\n");
   
   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if (!mpq_equal(con->rhs, zero))
      {
         lps_makename(ctmp, MPS_NAME_LEN + 1, con->name, con->number);
         write_data(fp, TRUE, ' ', ' ', "RHS", ctmp, con->rhs);
      }
   }
   fprintf(fp, "BOUNDS\n");

   /* Variables with size == 0, have to be included because
    * they might contain bounds. Detailed checking should
    * be done in a preprocessing routine.
    */
   for(var = lp->var_root; var != NULL; var = var->next)
   {
      /* This could be put into a preprocessor
       */
      if (var->size == 0 && mpq_equal(var->cost, zero))
         continue;
      
      /*   0, oo  -> nix
       *   l, oo  -> LO
       *   0, u   -> UP
       *   l = u  -> FX
       * -oo      -> MI
       *  oo      -> PL
       */
      lps_makename(vtmp, MPS_NAME_LEN + 1, var->name, var->number);

      if (var->type == VAR_LOWER || var->type == VAR_BOXED)
         write_data(fp, TRUE, 'L', 'O', "BOUND", vtmp, var->lower);
      else
         write_data(fp, FALSE, 'M', 'I', "BOUND", vtmp, zero);
         
      if (var->type == VAR_UPPER || var->type == VAR_BOXED)
         write_data(fp, TRUE, 'U', 'P', "BOUND", vtmp, var->upper);
      else
         write_data(fp, FALSE, 'P', 'L', "BOUND", vtmp, zero);            
   }
   fprintf(fp, "ENDATA\n");

   mpq_clear(zero);
}   



