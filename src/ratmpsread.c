#pragma ident "@(#) $Id: ratmpsread.c,v 1.2 2003/09/04 13:09:09 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: readmps.c                                                     */
/*   Name....: Read .mps file                                                */
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
#include "mpsinput.h"
#include "gmpmisc.h"

/* Directory separator, so we could redefine it for Windoof.
 */
#ifndef DIRSEP
#define DIRSEP '/'
#endif /* DIRSEP */

static const char* corename(char* filename)
{
   char* s;
   char* t;

   assert(filename != NULL);
   assert(strlen(filename) > 0);
   
   s = strrchr(filename, DIRSEP);
   s = (s == NULL) ? filename : s + 1;

   if (NULL != (t = strrchr(s, '.')))
      *t = '\0';

   return s;
}

/* Process NAME section.
 */
static void read_name(
   MpsInput* mpsi,
   Lps*      lp)
{
   assert(mpsi != NULL);

   do
   {
      /* This has to be the Line with the NAME section.
       */
      if (!mpsi_readline(mpsi) || mpsi_field0(mpsi) == NULL || strcmp(mpsi_field0(mpsi), "NAME"))
         break;

      /* Sometimes the name is omitted.
       */
      if (mpsi_field1(mpsi) == 0)
         mpsi_setprobname(mpsi,  "_MPS_");
      else
      {
         mpsi_setprobname(mpsi, mpsi_field1(mpsi));
         lps_setprobname(lp, mpsi_field1(mpsi));
      }
      printf("Problem name   : %s\n", mpsi_probname(mpsi));
 
      /* This hat to be a new section
       */
      if (!mpsi_readline(mpsi) || (mpsi_field0(mpsi) == NULL))
         break;

      if (!strcmp(mpsi_field0(mpsi), "ROWS"))
         mpsi_setsection(mpsi, MPS_ROWS);
      else if (!strncmp(mpsi_field0(mpsi), "OBJSEN", 6))
         mpsi_setsection(mpsi, MPS_OBJSEN);
      else if (!strcmp(mpsi_field0(mpsi), "OBJNAME"))
         mpsi_setsection(mpsi, MPS_OBJNAME);
      else
         break;

      return;
   }
   while(FALSE);

   mpsi_syntaxerror(mpsi);
}

/* Process OBJSEN section. This Section is an ILOG extension.
 */
static void read_objsen(MpsInput* mpsi)
{
   assert(mpsi != NULL);

   do
   {
      /* This has to be the Line with MIN or MAX.
       */
      if (!mpsi_readline(mpsi) || (mpsi_field1(mpsi) == 0))
         break;

      if (!strcmp(mpsi_field1(mpsi), "MIN"))
         mpsi_setminimizing(mpsi, TRUE);
      else if (!strcmp(mpsi_field1(mpsi), "MAX"))
         mpsi_setminimizing(mpsi, FALSE);
      else
         break;

      /* Look for ROWS or OBJNAME Section
       */
      if (!mpsi_readline(mpsi) || mpsi_field0(mpsi) == NULL)
         break;

      if (!strcmp(mpsi_field0(mpsi), "ROWS"))
         mpsi_setsection(mpsi, MPS_ROWS);
      else if (!strcmp(mpsi_field0(mpsi), "OBJNAME"))
         mpsi_setsection(mpsi, MPS_OBJNAME);
      else
         break;

      return;
   }
   while(FALSE);

   mpsi_syntaxerror(mpsi);
}

/* Process OBJNAME section. This Section is an ILOG extension.
 */
static void read_objname(
   MpsInput* mpsi,
   Lps*      lp)
{
   assert(mpsi != NULL);

   do
   {
      /* This has to be the Line with the name.
       */
      if (!mpsi_readline(mpsi) || mpsi_field1(mpsi) == NULL)
         break;

      mpsi_setobjname(mpsi, mpsi_field1(mpsi));
      lps_setobjname(lp, mpsi_field1(mpsi));

      /* Look for ROWS Section
       */
      if (!mpsi_readline(mpsi) || mpsi_field0(mpsi) == NULL)
         break;

      if (strcmp(mpsi_field0(mpsi), "ROWS"))
         break;

      mpsi_setsection(mpsi, MPS_ROWS);
      return;
   }
   while(FALSE);

   mpsi_syntaxerror(mpsi);
}

/* Process ROWS section. 
 */
static void read_rows(
   MpsInput* mpsi,
   Lps*      lp)
{
   Con*    con;
   mpq_t   zero;

   mpq_init(zero);

   while(mpsi_readline(mpsi))
   {
      if (mpsi_field0(mpsi) != NULL)
      {
         printf("Objective name : %s\n", mpsi_objname(mpsi));

         if (strcmp(mpsi_field0(mpsi), "COLUMNS"))
            break;

         mpsi_setsection(mpsi, MPS_COLUMNS);

         mpq_clear(zero);

         return;
      }
      if (*mpsi_field1(mpsi) == 'N')
      {
         if (*mpsi_objname(mpsi) == '\0')
         {
            mpsi_setobjname(mpsi, mpsi_field2(mpsi));
            lps_setobjname(lp, mpsi_field2(mpsi));
         }
      }
      else
      {
         if (NULL != lps_getcon(lp, mpsi_field2(mpsi)))
            break;

         con = lps_addcon(lp, mpsi_field2(mpsi));

         switch(*mpsi_field1(mpsi))
         {
         case 'G' :
            lps_setlhs(con, zero);
            break;
         case 'E' :
            lps_setlhs(con, zero);
            lps_setrhs(con, zero);
            break;
         case 'L' :
            lps_setrhs(con, zero);
            break;
         default :
            mpsi_syntaxerror(mpsi);
            mpq_clear(zero);
            return;
         }
      }
      assert(*mpsi_field1(mpsi) == 'N' || lps_getcon(lp, mpsi_field2(mpsi)) != NULL);
   }
   mpsi_syntaxerror(mpsi);

   mpq_clear(zero);
}

/* Process COLUMNS section. 
 */
static void read_cols(
   MpsInput* mpsi,
   Lps*      lp) 
{
   char     colname[MPS_MAX_LINELEN] = { '\0' };
   Con*     con;
   Var*     var = NULL;
   mpq_t    zero;
   mpq_t    one;
   mpq_t    val;

   mpq_init(zero);
   mpq_init(one);
   mpq_init(val);

   mpq_set_ui(one, 1, 1); /* = 1 */

   while(mpsi_readline(mpsi))
   {
      if (mpsi_field0(mpsi) != 0)
      {
         if (strcmp(mpsi_field0(mpsi), "RHS"))
            break;

#if 0
         if (colname[0] != '\0')
         {
         }
#endif
         mpsi_setsection(mpsi, MPS_RHS);

         mpq_clear(zero);
         mpq_clear(one);
         mpq_clear(val);
         return;
      }
      if (mpsi_field1(mpsi) == NULL || mpsi_field2(mpsi) == NULL || mpsi_field3(mpsi) == NULL)
         break;

      /* new column?
       */
      if (strcmp(colname, mpsi_field1(mpsi)))
      {
#if 0
         /* first column?
          */
         if (colname[0] != '\0')
         {
         }
#endif
         strcpy(colname, mpsi_field1(mpsi));

         /* cost is 0 as default, Bounds are 0..+oo */
         var = lps_addvar(lp, colname); 
         lps_setlower(var, zero);

         if (mpsi_is_integer(mpsi))
         {
            /* For Integer variable the default bounds are 0/1 
             */
            lps_setupper(var, one);
            lps_setclass(var, VAR_INT);
         }
      }
      assert(var != NULL);

      gmp_str2mpq(val, mpsi_field3(mpsi));

      if (!strcmp(mpsi_field2(mpsi), mpsi_objname(mpsi)))
         lps_setcost(var, val);
      else 
      {
         if (NULL == (con = lps_getcon(lp, mpsi_field2(mpsi))))
            mpsi_entryignored(mpsi, "Column", mpsi_field1(mpsi), "row", mpsi_field2(mpsi));
         else
            if (!mpq_equal(val, zero))
               lps_addnzo(lp, con, var, val);
      }
      if (mpsi_field5(mpsi) != NULL)
      {
         assert(mpsi_field4(mpsi) != NULL);

         gmp_str2mpq(val, mpsi_field5(mpsi));

         if (!strcmp(mpsi_field4(mpsi), mpsi_objname(mpsi)))
            lps_setcost(var, val);
         else 
         {
            if (NULL == (con = lps_getcon(lp, mpsi_field4(mpsi))))
               mpsi_entryignored(mpsi, "Column", mpsi_field1(mpsi), "row", mpsi_field4(mpsi));
            else
               if (!mpq_equal(val, zero))
                  lps_addnzo(lp, con, var, val);
         }
      }
   }
   mpsi_syntaxerror(mpsi);

   mpq_clear(zero);
   mpq_clear(one);
   mpq_clear(val);
}

/* Process RHS section. 
 */
static void read_rhs(
   MpsInput*  mpsi,
   Lps*       lp)
{
   char   rhsname[MPS_MAX_LINELEN] = { '\0' };
   Con*   con;
   mpq_t  val;

   mpq_init(val);

   while(mpsi_readline(mpsi))
   {
      if (mpsi_field0(mpsi) != NULL)
      {
         printf("RHS name       : %s\n", rhsname);
         lps_setrhsname(lp, rhsname);

         if (!strcmp(mpsi_field0(mpsi), "RANGES"))
            mpsi_setsection(mpsi, MPS_RANGES);
         else if (!strcmp(mpsi_field0(mpsi), "BOUNDS"))
            mpsi_setsection(mpsi, MPS_BOUNDS);
         else if (!strcmp(mpsi_field0(mpsi), "ENDATA"))
            mpsi_setsection(mpsi, MPS_ENDATA);
         else
            break;

         mpq_clear(val);
         return;
      }
      if ((mpsi_field2(mpsi) != NULL && mpsi_field3(mpsi) == NULL)
         || (mpsi_field4(mpsi) != NULL && mpsi_field5(mpsi) == NULL))
         mpsi_insertname(mpsi, "_RHS_", FALSE);
      
      if (mpsi_field1(mpsi) == NULL || mpsi_field2(mpsi) == NULL || mpsi_field3(mpsi) == NULL)
         break;

      if (*rhsname == '\0')
         strcpy(rhsname, mpsi_field1(mpsi));
      
      if (!strcmp(rhsname, mpsi_field1(mpsi)))
      {
         if (NULL == (con = lps_getcon(lp, mpsi_field2(mpsi))))
            mpsi_entryignored(mpsi, "RHS", mpsi_field1(mpsi), "row", mpsi_field2(mpsi));
         else
         {
            gmp_str2mpq(val, mpsi_field3(mpsi));

            assert(lps_contype(con) == CON_RHS 
               ||  lps_contype(con) == CON_LHS 
               ||  lps_contype(con) == CON_EQUAL);

            if (lps_contype(con) == CON_RHS)
               lps_setrhs(con, val);
            else if (lps_contype(con) == CON_LHS)
               lps_setlhs(con, val);
            else if (lps_contype(con) == CON_EQUAL)
            {
               lps_setrhs(con, val);
               lps_setlhs(con, val);
            }
         }
      }
      if (mpsi_field5(mpsi) != NULL)
      {
         if (NULL == (con = lps_getcon(lp, mpsi_field4(mpsi))))
            mpsi_entryignored(mpsi, "RHS", mpsi_field1(mpsi), "row", mpsi_field4(mpsi));
         else
         {
            gmp_str2mpq(val, mpsi_field5(mpsi));
         
            assert(lps_contype(con) == CON_RHS 
               ||  lps_contype(con) == CON_LHS 
               ||  lps_contype(con) == CON_EQUAL);

            if (lps_contype(con) == CON_RHS)
               lps_setrhs(con, val);
            else if (lps_contype(con) == CON_LHS)
               lps_setlhs(con, val);
            else if (lps_contype(con) == CON_EQUAL)
            {
               lps_setrhs(con, val);
               lps_setlhs(con, val);
            }
         }
      }
   }
   mpsi_syntaxerror(mpsi);
   mpq_clear(val);
}

/* Process RANGES section. 
 */
static void read_ranges(
   MpsInput* mpsi,
   Lps*      lp)
{
   char   rngname[MPS_MAX_LINELEN] = { '\0' };
   Con*   con;
   mpq_t  zero;
   mpq_t  tmp;
   mpq_t  val;

   mpq_init(zero);
   mpq_init(tmp);
   mpq_init(val);
   
   while(mpsi_readline(mpsi))
   {
      if (mpsi_field0(mpsi) != NULL)
      {
         printf("Range name     : %s\n", rngname);
         lps_setrngname(lp, rngname);

         if (!strcmp(mpsi_field0(mpsi), "BOUNDS"))
            mpsi_setsection(mpsi, MPS_BOUNDS);
         else if (!strcmp(mpsi_field0(mpsi), "ENDATA"))
            mpsi_setsection(mpsi, MPS_ENDATA);
         else
            break;

         mpq_clear(zero);
         mpq_clear(tmp);
         mpq_clear(val);
         return;
      }
      if ((mpsi_field2(mpsi) != NULL && mpsi_field3(mpsi) == NULL)
         || (mpsi_field4(mpsi) != NULL && mpsi_field5(mpsi) == NULL))
         mpsi_insertname(mpsi, "_RNG_", FALSE);

      if (mpsi_field1(mpsi) == NULL || mpsi_field2(mpsi) == NULL || mpsi_field3(mpsi) == NULL)
         break;

      if (*rngname == '\0')
         strcpy(rngname, mpsi_field1(mpsi));

      /* The rules are:
       * Row Sign   LHS             RHS
       * ----------------------------------------
       *  G   +/-   rhs             rhs + |range|
       *  L   +/-   rhs - |range|   rhs
       *  E   +     rhs             rhs + range
       *  E   -     rhs + range     rhs 
       * ----------------------------------------
       */  
      if (!strcmp(rngname, mpsi_field1(mpsi)))
      {
         if (NULL == (con = lps_getcon(lp, mpsi_field2(mpsi))))
            mpsi_entryignored(mpsi, "Range", mpsi_field1(mpsi), "row", mpsi_field2(mpsi));
         else
         {
            gmp_str2mpq(val, mpsi_field3(mpsi));

            assert(lps_contype(con) != CON_RANGE);

            if (lps_contype(con) == CON_EQUAL)
            {
               if (mpq_cmp(val, zero) >= 0)
               {
                  lps_getrhs(con, tmp);
                  mpq_add(tmp, tmp, val);
                  lps_setrhs(con, tmp);
               }
               else
               {
                  lps_getlhs(con, tmp);
                  mpq_add(tmp, tmp, val);
                  lps_setlhs(con, tmp);
               }
            }
            else
            {
               mpq_abs(val, val);

               if (lps_contype(con) == CON_LHS)
               {
                  lps_getlhs(con, tmp);
                  mpq_add(tmp, tmp, val);
                  lps_setrhs(con, tmp);
               }
               else 
               {
                  assert(lps_contype(con) == CON_RHS);

                  lps_getrhs(con, tmp);
                  mpq_sub(tmp, tmp, val);
                  lps_setlhs(con, tmp);
               }
            }
         }
         if (mpsi_field5(mpsi) != NULL)
         {
            if (NULL == (con = lps_getcon(lp, mpsi_field4(mpsi))))
               mpsi_entryignored(mpsi, "Range", mpsi_field1(mpsi), "row", mpsi_field4(mpsi));
            else
            {
               assert(lps_contype(con) != CON_RANGE);

               gmp_str2mpq(val, mpsi_field5(mpsi));
               mpq_abs(val, val);

               if (lps_contype(con) == CON_EQUAL)
               {
                  if (mpq_cmp(val, zero) >= 0)
                  {
                     lps_getrhs(con, tmp);
                     mpq_add(tmp, tmp, val);
                     lps_setrhs(con, tmp);
                  }
                  else
                  {
                     lps_getlhs(con, tmp);
                     mpq_add(tmp, tmp, val);
                     lps_setlhs(con, tmp);
                  }
               }
               else
               {
                  mpq_abs(val, val);

                  if (lps_contype(con) == CON_LHS)
                  {
                     lps_getlhs(con, tmp);
                     mpq_add(tmp, tmp, val);
                     lps_setrhs(con, tmp);
                  }
                  else 
                  {
                     assert(lps_contype(con) == CON_RHS);
                     
                     lps_getrhs(con, tmp);
                     mpq_sub(tmp, tmp, val);
                     lps_setlhs(con, tmp);
                  }
               }
            }
         }
      }
   }
   mpsi_syntaxerror(mpsi);
   mpq_clear(zero);
   mpq_clear(tmp);
   mpq_clear(val);
}

/* Process BOUNDS section. 
 */
static void read_bounds(
   MpsInput* mpsi,
   Lps*      lp)
{
   char   bndname[MPS_MAX_LINELEN] = { '\0' };
   Var*   var;
   mpq_t    zero;
   mpq_t    one;
   mpq_t    val;

   mpq_init(zero);
   mpq_init(one);
   mpq_init(val);

   mpq_set_ui(one, 1, 1); /* = 1 */

   while(mpsi_readline(mpsi))
   {
      if (mpsi_field0(mpsi) != 0)
      {
         printf("Bound name     : %s\n", bndname);
         lps_setbndname(lp, bndname);

         if (strcmp(mpsi_field0(mpsi), "ENDATA"))
            break;

         mpsi_setsection(mpsi, MPS_ENDATA);

         mpq_clear(zero);
         mpq_clear(one);
         mpq_clear(val);
         return;
      }
      /* Is the value field used ?
       */
      if (  (!strcmp(mpsi_field1(mpsi), "LO"))
         || (!strcmp(mpsi_field1(mpsi), "UP"))
         || (!strcmp(mpsi_field1(mpsi), "FX"))
         || (!strcmp(mpsi_field1(mpsi), "LI"))
         || (!strcmp(mpsi_field1(mpsi), "UI")))
      {
         if (mpsi_field3(mpsi) != NULL && mpsi_field4(mpsi) == NULL)
            mpsi_insertname(mpsi, "_BND_", TRUE);
      }
      else
      {
         if (mpsi_field2(mpsi) != NULL && mpsi_field3(mpsi) == NULL)
            mpsi_insertname(mpsi, "_BND_", TRUE);
      }

      if (mpsi_field1(mpsi) == NULL || mpsi_field2(mpsi) == NULL || mpsi_field3(mpsi) == NULL)
         break;

      if (*bndname == '\0')
         strcpy(bndname, mpsi_field2(mpsi));
      
      /* Only read the first Bound in section
       */
      if (!strcmp(bndname, mpsi_field2(mpsi)))
      {
         if (NULL == (var = lps_getvar(lp, mpsi_field3(mpsi))))
            mpsi_entryignored(mpsi, "column", mpsi_field3(mpsi), "bound", bndname);
         else
         {            
            gmp_str2mpq(val, (mpsi_field4(mpsi) == 0) ? "0" : mpsi_field4(mpsi));

            switch(*mpsi_field1(mpsi))
            {
            case 'L':
               lps_setlower(var, val);
               
               /* ILOG extension (Integer Lower Bound)
                */
                if (mpsi_field1(mpsi)[1] == 'I')
                   lps_setclass(var, VAR_INT);
               break;
            case 'U':
               lps_setupper(var, val);
               
               /* ILOG extension (Integer Upper Bound)
                */
               if (mpsi_field1(mpsi)[1] == 'I')
                  lps_setclass(var, VAR_INT);
               break;
            case 'F':
               if (mpsi_field1(mpsi)[1] == 'X')
               {
                  lps_setlower(var, val);
                  lps_setupper(var, val);
               }
               else
               {
                  lps_setvartype(var, VAR_FREE);
               }
               break;
            case 'M':
               if (lps_vartype(var) == VAR_LOWER)
                  lps_setvartype(var,  VAR_FREE);
               else if (lps_vartype(var) == VAR_BOXED || lps_vartype(var) == VAR_FIXED)
                  lps_setvartype(var,  VAR_UPPER);
               break;
            case 'P':
               if (lps_vartype(var) == VAR_UPPER)
                  lps_setvartype(var,  VAR_FREE);
               else if (lps_vartype(var) == VAR_BOXED || lps_vartype(var) == VAR_FIXED)
                  lps_setvartype(var,  VAR_LOWER);
               break;
            case 'B' : /* Ilog extension (Binary) */
               lps_setlower(var, zero);
               lps_setupper(var, one);
               lps_setclass(var, VAR_BIN);
               break;
            default:
               mpsi_syntaxerror(mpsi);
               return;
            }
         }
      }
   }
   mpsi_syntaxerror(mpsi);
   mpq_clear(zero);
   mpq_clear(one);
   mpq_clear(val);
}

/* Read LP in "MPS File Format".
 * 
 *  The specification is taken from the
 *
 *  IBM Optimization Library Guide and Reference
 *
 *  Online available at http://www.software.ibm.com/sos/features/libuser.htm
 *
 *  and from the 
 *
 *  ILOG CPLEX 7.0 Reference Manual, Appendix E, Page 531.
 *
 *  This routine should read all valid MPS format files. 
 *  What it will not do, is find all cases where a file is ill formed. 
 *  If this happens it may complain and read nothing or read "something".
 *
 *  @return TRUE if the file was read correctly.
 */  
Lps* lps_readmps(
   const char* filename)
{
   gzFile    fp;
   Lps*      lp;
   MpsInput* mpsi;
   char*     probname;

   if (NULL == (fp = gzopen(filename, "r")))
   {
      perror(filename);
      return NULL;
   }   
   probname = strdup(filename);

   lp = lps_alloc(corename(probname));

   assert(lp != NULL);

   free(probname);

   mpsi = mpsi_init(fp);

   read_name(mpsi, lp);

   if (mpsi_section(mpsi) == MPS_OBJSEN)
      read_objsen(mpsi);

   if (mpsi_section(mpsi) == MPS_OBJNAME)
      read_objname(mpsi, lp);

   if (mpsi_section(mpsi) == MPS_ROWS)
      read_rows(mpsi, lp);

   if (mpsi_section(mpsi) == MPS_COLUMNS)
      read_cols(mpsi, lp);

   if (mpsi_section(mpsi) == MPS_RHS)
      read_rhs(mpsi, lp);

   if (mpsi_section(mpsi) == MPS_RANGES)
      read_ranges(mpsi, lp);

   if (mpsi_section(mpsi) == MPS_BOUNDS)
      read_bounds(mpsi, lp);

   if (mpsi_section(mpsi) != MPS_ENDATA)
      mpsi_syntaxerror(mpsi);

   if (mpsi_has_error(mpsi))
   {
      lps_free(lp);
      lp = NULL;
   }
   else
   {
      lps_setdir(lp, mpsi_minimizing(mpsi) ? LP_MIN : LP_MAX);

      printf("Objective sense: %s\n",
         mpsi_minimizing(mpsi) ? "Minimize" : "Maximize"); 
   }
   mpsi_exit(mpsi);

   gzclose(fp);

   return lp;
}

/* ------------------------------------------------------------------------- */
/* Emacs Local Variables:                                                    */
/* Emacs mode:c                                                              */
/* Emacs c-basic-offset:3                                                    */
/* Emacs tab-width:8                                                         */
/* Emacs indent-tabs-mode:nil                                                */
/* Emacs End:                                                                */
/* ------------------------------------------------------------------------- */

