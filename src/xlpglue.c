#pragma ident "@(#) $Id: xlpglue.c,v 1.32 2009/09/13 16:15:56 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: xlpglue.c                                                     */
/*   Name....: LP Storage Interface                                          */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2009 by Thorsten Koch <koch@zib.de>
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
#include "mono.h"
#include "gmpmisc.h"
#include "xlpglue.h"

static Lps*              lp           = NULL;
static const char* const title        = "This file was automatically generated by Zimpl";
static Bool              use_startval = FALSE;

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

static Bool check_con_is_invalid(const Con* con)
{
   Bool  is_invalid = FALSE;
   mpq_t lhs;
   mpq_t rhs;
   mpq_t sum;

   if (!use_startval)
      return FALSE;
   
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
            is_invalid = TRUE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) <= RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(rhs));
         }
         break;
      case CON_LHS :
         if (mpq_cmp(lhs, sum) > 0 && stmt_trigger_warning(215))
         {
            is_invalid = TRUE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) >= RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(lhs));
         }
         break;
      case CON_EQUAL : /* In case of EQUAL, both should be equal */
         if ((!mpq_equal(lhs, sum) || !mpq_equal(rhs, sum)) && stmt_trigger_warning(215))
         {
            is_invalid = TRUE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) == RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(rhs));
         }
         break;
      case CON_RANGE :
         if ((mpq_cmp(sum, rhs) > 0 || mpq_cmp(lhs, sum) > 0) && stmt_trigger_warning(215))
         {
            is_invalid = TRUE;
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

   return is_invalid;
}

Bool xlp_addcon_term(
   const char*  name,
   ConType      contype,
   const Numb*  lhs,
   const Numb*  rhs,
   unsigned int flags,
   const Term*  term_org)
{
   Term* term;
   Con*  con;
   mpq_t tlhs;
   mpq_t trhs;
   int   i;

   assert(name != NULL);
   assert(lhs  != NULL);
   assert(rhs  != NULL);
   assert(term_is_valid(term_org));
   assert(numb_equal(term_get_constant(term_org), numb_zero()));

   term = term_simplify(term_org);

   if (term_get_degree(term) > 2)
   {
      fprintf(stderr, "Warning 600 Can only handle linear and quadratic constraints -- ignored\n");
      term_free(term);

      return TRUE;
   }
  
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

   for(i = 0; i < term_get_elements(term); i++)
   {
      const Mono* mono = term_get_element(term, i);
      Var*        var  = mono_get_var(mono, 0);
      MFun        mfun = mono_get_function(mono);
      mpq_t       val1;

      if (mfun == MFUN_TRUE || mfun == MFUN_FALSE)
      {
         lps_setindicator(con, var, mfun == MFUN_TRUE);
         continue;
      }

      assert(!numb_equal(mono_get_coeff(mono), numb_zero()));

      mpq_init(val1);

      numb_get_mpq(mono_get_coeff(mono), val1);

      if (mono_is_linear(mono))
      {
         Nzo* nzo = lps_getnzo(lp, con, var);
      
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
      }
      else
      {
         assert(mono_get_degree(mono) == 2);

         lps_addqme(lp, con, var, mono_get_var(mono, 1), val1);
      }
      mpq_clear(val1);
   }
   term_free(term);

   return check_con_is_invalid(con);
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

Bool xlp_addsos_term(
   const char*  name,
   SosType      type,
   const Numb*  priority,
   const Term*  term)
{
   Sos*   sos;
   mpq_t  temp;
   Hash*  hash;
   int    i;
   Bool   has_duplicates = FALSE;
   Bool   has_nonintprio = FALSE;

   assert(name     != NULL);
   assert(priority != NULL);
   assert(term_is_valid(term));
   assert(numb_equal(term_get_constant(term), numb_zero()));

   mpq_init(temp);

   numb_get_mpq(priority, temp);
   
   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   if (mpz_get_si(mpq_denref(temp)) != 1)
   {
      if (verbose > VERB_QUIET)
      {
         fprintf(stderr, "*** Warning SOS priority has to be integral\n");
         has_nonintprio = TRUE;
      }
   }

   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   sos = lps_addsos(lp, name, type, (int)mpz_get_si(mpq_numref(temp)));

   assert(sos != NULL);

   
   hash = hash_new(HASH_NUMB, term_get_elements(term));

   assert(hash != NULL);
   
   for(i = 0; i < term_get_elements(term); i++)
   {
      const Mono* mono  = term_get_element(term, i);
      const Numb* coeff = mono_get_coeff(mono);
      Var*        var   = mono_get_var(mono, 0);
      
      assert(!numb_equal(coeff, numb_zero()));
      assert(mono_is_linear(mono));
      
      /* Each weight is allowed only once.
       */
      if (hash_has_numb(hash, coeff))
         has_duplicates = TRUE;

      hash_add_numb(hash, coeff);

      numb_get_mpq(coeff, temp);
   
      lps_addsse(sos, var, temp);
   }
   hash_free(hash);

   mpq_clear(temp);
   
   return has_duplicates || has_nonintprio;
}

const char* xlp_getvarname(const Var* var)
{
   assert(var != NULL);

   return lps_varname(var);
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

     

