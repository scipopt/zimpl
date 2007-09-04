#pragma ident "@(#) $Id: xlpglue.c,v 1.29 2007/09/04 16:19:00 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: xlpglue.c                                                     */
/*   Name....: LP Storage Interface                                          */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2007 by Thorsten Koch <koch@zib.de>
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
#include "mshell.h"
#include "ratlp.h"
#include "mme.h"
#include "gmpmisc.h"
#include "xlpglue.h"

static Lps*        lp           = NULL;
static const char* title        = "This file was automatically generated by Zimpl";
static Bool        use_startval = FALSE;

void xlp_alloc(const char* name, Bool need_startval)
{
   assert(name != NULL);
   
   lp           = lps_alloc(name);
   use_startval = need_startval;
}

void xlp_setnamelen(int name_len)
{
   lps_setnamelen(lp, name_len);
}

void xlp_scale()
{
   lps_scale(lp);
}

void xlp_write(FILE* fp, LpFormat format, const char* text)
{
   assert(fp != NULL);

   lps_write(lp, fp, format, text);
}

void xlp_transtable(FILE* fp, LpFormat format)
{
   assert(fp != NULL);
   
   lps_transtable(lp, fp, format, "zimpl");
}

void xlp_orderfile(FILE* fp, LpFormat format)
{
   assert(fp != NULL);

   lps_orderfile(lp, fp, format, title);
}

void xlp_mstfile(FILE* fp, LpFormat format)
{
   assert(fp != NULL);

   lps_mstfile(lp, fp, format, title);
}

void xlp_sosfile (FILE* fp, LpFormat format)
{
  assert(fp != NULL);

  lps_sosfile(lp, fp, format, title);
}

void xlp_free()
{
   lps_free(lp);
   lp = NULL;
}

void xlp_stat()
{
   lps_stat(lp);
}

Bool xlp_hassos()
{
   return lps_has_sos(lp);
}

Bool xlp_conname_exists(const char* conname)
{
   assert(conname != NULL);
   
   return NULL != lps_getcon(lp, conname);
}

Con* xlp_addcon(
   const char*  name,
   ConType      contype,
   const Numb*  lhs,
   const Numb*  rhs,
   unsigned int flags)
{
   Con*  con;
   mpq_t tlhs;
   mpq_t trhs;
   
   assert(name != NULL);
   assert(lhs  != NULL);
   assert(rhs  != NULL);

   con = lps_addcon(lp, name);

   assert(con != NULL);

   mpq_init(tlhs);
   mpq_init(trhs);
   
   numb_get_mpq(rhs, trhs);
   numb_get_mpq(lhs, tlhs);
   
   switch(contype)
   {
   case CON_RHS :
      lps_setrhs(con, trhs);
      break;
   case CON_LHS :
      lps_setlhs(con, tlhs);
      break;
   case CON_EQUAL : /* In case of EQUAL, both should be equal */
      assert(mpq_equal(tlhs, trhs));
      /*lint -fallthrough*/
   case CON_RANGE :
      lps_setlhs(con, tlhs);
      lps_setrhs(con, trhs);
      break;
   default :
      abort();
   }
   lps_addflags(con, flags);

   mpq_clear(trhs);
   mpq_clear(tlhs);
   
   return con;
}

Con* xlp_addcon_term(
   const char*  name,
   ConType      contype,
   const Numb*  lhs,
   const Numb*  rhs,
   unsigned int flags,
   const Term*  term)
{
   return 0; /* ??? */
}
     
Var* xlp_addvar(
   const char*  name,
   VarClass     varclass,
   const Bound* lower,
   const Bound* upper,
   const Numb*  priority,
   const Numb*  startval)
{
   Var*      var;
   mpq_t     temp;
   
   assert(name     != NULL);
   assert(lower    != NULL);
   assert(upper    != NULL);
   assert(priority != NULL);
   assert(startval != NULL);
   
   var = lps_addvar(lp, name);

   lps_setclass(var, varclass);
   
   mpq_init(temp);

   if (bound_get_type(lower) == BOUND_VALUE)
   {
      numb_get_mpq(bound_get_value(lower), temp);
      lps_setlower(var, temp);
   }
   if (bound_get_type(upper) == BOUND_VALUE)
   {
      numb_get_mpq(bound_get_value(upper), temp);
      lps_setupper(var, temp);
   }
   numb_get_mpq(startval, temp);
   
   lps_setstartval(var, temp);

   numb_get_mpq(priority, temp);
   
   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   if (mpz_get_si(mpq_denref(temp)) != 1)
      if (verbose > VERB_QUIET)
         fprintf(stderr, "*** Warning variable priority has to be integral\n");

   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   lps_setpriority(var, (int)mpz_get_si(mpq_numref(temp)));

   mpq_clear(temp);
   
   return var;
}

Sos* xlp_addsos(
   const char*  name,
   SosType      type,
   const Numb*  priority)
{
   Sos*   sos;
   mpq_t  temp;
   
   assert(name     != NULL);
   assert(priority != NULL);

   mpq_init(temp);

   numb_get_mpq(priority, temp);
   
   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   if (mpz_get_si(mpq_denref(temp)) != 1)
      if (verbose > VERB_QUIET)
         fprintf(stderr, "*** Warning SOS priority has to be integral\n");

   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   sos = lps_addsos(lp, name, type, (int)mpz_get_si(mpq_numref(temp)));

   mpq_clear(temp);
   
   return sos;
}

void xlp_addtosos(Sos* sos, Var* var, const Numb* weight)
{
   mpq_t temp;
   
   assert(weight != NULL);
   assert(var    != NULL);
   assert(sos    != NULL);

   mpq_init(temp);

   numb_get_mpq(weight, temp);
   
   lps_addsse(sos, var, temp);

   mpq_clear(temp);
}

VarClass xlp_getclass(const Var* var)
{
   assert(var != NULL);

   return lps_getclass(var);
}

Bound* xlp_getlower(const Var* var)
{
   Bound* bound;
   Numb*  numb;
   mpq_t  lower;
   
   assert(var != NULL);

   if (!lps_haslower(var))
      bound = bound_new(BOUND_MINUS_INFTY, numb_zero());
   else
   {
      mpq_init(lower);
      lps_getlower(var, lower);
      numb  = numb_new_mpq(lower);
      bound = bound_new(BOUND_VALUE, numb);
      numb_free(numb);
      mpq_clear(lower);
   }
   return bound;
}
     
Bound* xlp_getupper(const Var* var)
{
   Bound* bound;
   Numb*  numb;
   mpq_t  upper;
   
   assert(var != NULL);

   if (!lps_hasupper(var))
      bound = bound_new(BOUND_INFTY, numb_zero());
   else
   {
      mpq_init(upper);
      lps_getupper(var, upper);
      numb  = numb_new_mpq(upper);
      bound = bound_new(BOUND_VALUE, numb);
      numb_free(numb);
      mpq_clear(upper);
   }
   return bound;
}
     
void xlp_objname(const char* name)
{
   assert(name != NULL);

   lps_setobjname(lp, name);
}

void xlp_setdir(Bool minimize)
{
   lps_setdir(lp, minimize ? LP_MIN : LP_MAX);
}

void xlp_addtonzo(Var* var, Con* con, const Numb* value)
{
   Nzo*  nzo;
   mpq_t val1;

   assert(var   != NULL);
   assert(con   != NULL);
   assert(value != NULL);
   
   mpq_init(val1);
   numb_get_mpq(value, val1);
   
   nzo = lps_getnzo(lp, con, var);

   if (nzo == NULL)
      lps_addnzo(lp, con, var, val1);
   else
   {
      mpq_t val2;

      mpq_init(val2);

      lps_getval(nzo, val2);
      
      mpq_add(val1, val1, val2);

      if (mpq_equal(val1, const_zero))
         lps_delnzo(lp, nzo);
      else
         lps_setval(nzo, val1);

      mpq_clear(val2);
   }
   mpq_clear(val1);
}

void xlp_addtocost(Var* var, const Numb* cost)
{
   mpq_t val1;
   mpq_t val2;

   assert(var  != NULL);
   assert(cost != NULL);
   
   mpq_init(val1);
   mpq_init(val2);

   lps_getcost(var, val1);
   numb_get_mpq(cost, val2);
   mpq_add(val1, val1, val2);
   lps_setcost(var, val1);

   mpq_clear(val1);
   mpq_clear(val2);
}

Bool xlp_concheck(const Con* con)
{
   Bool  ok = TRUE;
   mpq_t lhs;
   mpq_t rhs;
   mpq_t sum;

   if (!use_startval)
      return TRUE;
   
   mpq_init(sum);
   mpq_init(lhs);
   mpq_init(rhs);

   /* Get sum of coefficients time startval if usable
    * (no continous variables)
    */
   if (lps_con_sumup(con, sum))
   {
      lps_getlhs(con, lhs);
      lps_getrhs(con, rhs);

      switch(lps_contype(con))
      {
      case CON_RHS :
         if (mpq_cmp(sum, rhs) > 0 && stmt_trigger_warning(215))
         {
            ok = FALSE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) <= RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(rhs));
         }
         break;
      case CON_LHS :
         if (mpq_cmp(lhs, sum) > 0 && stmt_trigger_warning(215))
         {
            ok = FALSE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) >= RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(lhs));
         }
         break;
      case CON_EQUAL : /* In case of EQUAL, both should be equal */
         if ((!mpq_equal(lhs, sum) || !mpq_equal(rhs, sum)) && stmt_trigger_warning(215))
         {
            ok = FALSE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) == RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(rhs));
         }
         break;
      case CON_RANGE :
         if ((mpq_cmp(sum, rhs) > 0 || mpq_cmp(lhs, sum) > 0) && stmt_trigger_warning(215))
         {
            ok = FALSE;
            fprintf(stderr, "--- Warning 215: Startvals violate range, LHS (%g) <= Sum (%g) <= RHS (%g)\n",
               mpq_get_d(lhs), mpq_get_d(sum), mpq_get_d(rhs));
         }
         break;
      default :
         abort();
      }
   }
   mpq_clear(sum);
   mpq_clear(rhs);
   mpq_clear(lhs);

   return ok;
}

Bool xlp_presolve()
{
   PSResult res;
   
   res = lps_presolve(lp, verbose);

   switch(res)
   {
   case PRESOLVE_INFEASIBLE :
      printf("Model is infeasible\n");
      return FALSE;
   case PRESOLVE_UNBOUNDED :
      printf("Model is unbounded\n");
      return FALSE;
   case PRESOLVE_VANISHED :
      printf("Presolve solved the problem -- see .tbl file for values\n");
      break;
   case PRESOLVE_OKAY :
      break;
   default :
      abort();
   }
   return TRUE;
}

     

