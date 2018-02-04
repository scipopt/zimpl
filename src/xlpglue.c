/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: xlpglue.c                                                     */
/*   Name....: LP Storage Interface                                          */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2018 by Thorsten Koch <koch@zib.de>
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
/**@file   xlpglue.c 
 * @brief  Mathematical Program Storage Interface 
 * @author Thorsten Koch 
 *
 * The function defined in this file are the once that have to be implemented
 * to hook Zimpl up as an reader in some other program. You have then to link
 * the Zimpl library together with your xlpglue.c implementation.
 *
 * Please note one important point: While the lps, var, con, and user_data pointers are 
 * passed arround in these routines, they are never used themselves by Zimpl. It is the choice 
 * of the implementor of the xlpglue interface whether to use these, have the different or the 
 * same, or set the to NULL. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <gmp.h>

#include "lint.h"
#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "numb.h"
#include "elem.h"
#include "tuple.h"
#include "mme.h"
#include "bound.h"
#include "mono.h"
#include "term.h"
#include "ratlp.h"
#include "gmpmisc.h"
#include "conname.h"
#include "stmt.h"
#include "hash.h"
#include "xlpglue.h"

#ifdef _MSC_VER
#pragma warning (disable: 4100) /* unreferenced formal parameter */
#endif

/*lint -esym(818, lp) 'lp' could be declared as pointing to const */

static Bool use_startval = FALSE;

/** Allocate storage for the mathematical program instance generated by Zimpl.
 *  xlp_alloc() is the first xlpglue routine that will be called by Zimpl.
 *  The user_data pointer may hold an arbitray value.
 *  
 *  @return The pointer returned from xlp_alloc is passed to all other xlpglue
 *  routines, but never referenced by Zimpl itself. When implementing an
 *  xlpglue interface you may choose to pass the user_data pointer as the Lps
 *  pointer if it suits your needs.
 */
/*ARGSUSED*/
Lps* xlp_alloc(
   const char*  name,          /**< Instance name, will appear e.g. in MPS NAME section */
   Bool         need_startval, /**< Will we supply starting values for the variables? */
   UNUSED void* user_data)     /**< Pointer with arbitrary data. */
{
   assert(name != NULL);
   
   use_startval = need_startval;

   return lps_alloc(name);
}

/** Free storage for mathematical program.
 *  xlp_free() is the last xlpglue routine that will be called by Zimpl.
 */
void xlp_free(
   Lps* lp)         /**< Pointer to storage as given by xlp_alloc */
{
   assert(lp != NULL);

   lps_free(lp);
}

#if 0 /* not used anymore ??? */
/** Does the mathematical program have SOS constraints?
 *  @return True if there is at least one SOS constraint, False otherwise.
 */
Bool xlp_hassos(
   const Lps* lp)         /**< Pointer to storage */
{
   assert(lp != NULL);

   return lps_has_sos(lp);
}
#endif

/** Does there already exists a constraint with the given name?
 *  @return True if there is already a constraint with name conname, False
 *          otherwise.
 */
/*ARGSUSED*/
Bool xlp_conname_exists(
   const Lps*  lp,         /**< Pointer to storage */
   const char* conname)    /**< Constraint name to check */
{
   assert(lp      != NULL);
   assert(conname != NULL);
   
   return NULL != lps_getcon(lp, conname);
}

/** Check if a constraint is violate by the given starting values.
 *  @return True if the constraint violates the start values, False
 *          otherwise.
 */
static Bool check_con_is_invalid(
   const Con* con)      /**< Pointer to a previous added constraint */
{
   Bool  is_invalid = FALSE;
   mpq_t lhs;
   mpq_t rhs;
   mpq_t sum;

   assert(con != NULL);

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

/** Add a constraint.
 *  Add a given a Zimpl term together with a right and/or left hand side as
 *  a constraint to the mathematical program. The form is lhs <= term <= rhs.
 *  @return True if the constraint violates the given start values, False
 *          otherwise.
  */
Bool xlp_addcon_term(
   Lps*         lp,         /**< Pointer to storage */
   const char*  name,       /**< Name of the constraint */
   ConType      contype,    /**< Type of constraint (LHS, RHS, EQUAL, RANGE, etc) */
   const Numb*  lhs,        /**< lhs <= term. Not used if contype is CON_RHS */
   const Numb*  rhs,        /**< term <= rhs. Not used if contype is CON_LHS */
   unsigned int flags,      /**< special treatment flags, see ratlptypes.h */
   const Term*  term_org)   /**< term to use */
{
   Term* term;
   Con*  con;
   mpq_t tlhs;
   mpq_t trhs;

   assert(lp   != NULL);
   assert(name != NULL);
   assert(lhs  != NULL);
   assert(rhs  != NULL);
   assert(term_is_valid(term_org));
   assert(numb_equal(term_get_constant(term_org), numb_zero()));

   term = term_simplify(term_org);

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

   if ((term_get_degree(term) > 2) || !term_is_polynomial(term))
      lps_addterm(lp, con, term);
   else
   {
      for(int i = 0; i < term_get_elements(term); i++)
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
            assert(term_get_degree(term) == 2);

            lps_addqme(lp, con, var, mono_get_var(mono, 1), val1);
         }
         mpq_clear(val1);
      }
   }
   term_free(term);

   return check_con_is_invalid(con);
}
  
/** Add a new variable to the mathematical program. 
 *  @return Pointer to the variable. This is never dereferenced in Zimpl and
 *          my be an arbitrary pointer used to reference this particular 
 *          variable.
 */
Var* xlp_addvar(
   Lps*         lp,         /**< Pointer to storage */
   const char*  name,       /**< Name of the variable */
   VarClass     varclass,   /**< Class of the variable (Continous, Implicit, Integer) */
   const Bound* lower,      /**< Lower bound of the variable */
   const Bound* upper,      /**< Upper bound of the variable */
   const Numb*  priority,   /**< Priority value of branching order */
   const Numb*  startval)   /**< Start value. If equal numb_unknown() startval is undefined */
{
   Var*      var;
   mpq_t     temp;
   
   assert(lp       != NULL);
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
   if (numb_equal(startval, numb_unknown()))
      numb_get_mpq(numb_zero(), temp);
   else
      numb_get_mpq(startval, temp);

   lps_setstartval(var, temp);
   
   numb_get_mpq(priority, temp);
   
   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   if (mpz_get_si(mpq_denref(temp)) != 1)
      if (verbose > VERB_QUIET)
         fprintf(stderr, "--- Warning 301: variable priority has to be integral\n");

   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   lps_setpriority(var, (int)mpz_get_si(mpq_numref(temp)));

   mpq_clear(temp);
   
   return var;
}

/** Add a SOS constraint.
 *  Add a given a Zimpl term as an SOS constraint to the mathematical program.
 *  @return 0 if everything is ok, 1 bit is set if not all of the weights are
 *          unique, 2 bit is set if the weights are not integer.
  */
 int xlp_addsos_term(
   Lps*         lp,          /**< Pointer to storage */
   const char*  name,        /**< Name of the constraint */
   SosType      type,        /**< Type of SOS (1, 2) */
   const Numb*  priority,    /**< Priority value for constraint */
   const Term*  term)        /**< Term to use */
{
   Sos*   sos;
   mpq_t  temp;
   Hash*  hash;
   int    i;
   Bool   has_duplicates = FALSE;
   Bool   has_nonintprio = FALSE;

   assert(lp       != NULL);
   assert(name     != NULL);
   assert(priority != NULL);
   assert(term_is_valid(term));
   assert(numb_equal(term_get_constant(term), numb_zero()));

   mpq_init(temp);

   numb_get_mpq(priority, temp);
   
   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   if (mpz_get_si(mpq_denref(temp)) != 1)
      has_nonintprio = TRUE;

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
   
   return (has_duplicates ? 1 : 0) + (has_nonintprio ? 2 : 0);
}

/** Get name of variable.
 *  @return The name of the variable.
 */
/*ARGSUSED*/
const char* xlp_getvarname(
   UNUSED const Lps* lp,          /**< Pointer to storage */
   const Var*        var)         /**< Pointer to variable as returned by xlp_addvar() */
{
   assert(var != NULL);

   return lps_varname(var);
}

/** Get the class of the variable.
 *  @return The class of the variable.
 */
/*ARGSUSED*/
VarClass xlp_getclass(
   UNUSED const Lps* lp,          /**< Pointer to storage */
   const Var*        var)         /**< Pointer to variable as returned by xlp_addvar() */
{
   assert(var != NULL);

   return lps_getclass(var);
}

/** Get the lower bound of the variable.
 *  @return The lower bound of the variable.
 */
/*ARGSUSED*/
Bound* xlp_getlower(
   UNUSED const Lps* lp,          /**< Pointer to storage */
   const Var*        var)         /**< Pointer to variable as returned by xlp_addvar() */
{
   assert(var != NULL);

   Bound* bound;
   
   if (!lps_haslower(var))
      bound = bound_new(BOUND_MINUS_INFTY, numb_zero());
   else
   {
      mpq_t  lower;
      
      mpq_init(lower);
      lps_getlower(var, lower);
      
      Numb* numb  = numb_new_mpq(lower);
      
      bound = bound_new(BOUND_VALUE, numb);

      numb_free(numb);
      mpq_clear(lower);
   }
   return bound;
}
     
/** Get the upper bound of the variable.
 *  @return The upper bound of the variable.
 */
/*ARGSUSED*/
Bound* xlp_getupper(
   UNUSED const Lps* lp,          /**< Pointer to storage */
   const Var*        var)         /**< Pointer to variable as returned by xlp_addvar() */

{
   assert(var != NULL);

   Bound* bound;
   
   if (!lps_hasupper(var))
      bound = bound_new(BOUND_INFTY, numb_zero());
   else
   {
      mpq_t  upper;
      
      mpq_init(upper);
      lps_getupper(var, upper);
      
      Numb* numb  = numb_new_mpq(upper);
      
      bound = bound_new(BOUND_VALUE, numb);
      
      numb_free(numb);
      mpq_clear(upper);
   }
   return bound;
}
     
/** Set the name and direction of the objective function, i.e. minimization or maximization
 *  Coefficents of the objective function will be set to all zero.
 */
Bool xlp_setobj(
   Lps*        lp,          /**< Pointer to storage */
   const char* name,        /**< The name of the objective function */
   Bool        minimize)    /**< True if the problem should be minimized, False if it should be maximized */
{
   assert(lp   != NULL);
   assert(name != NULL);

   lps_clearobj(lp);

   lps_setdir(lp, minimize ? LP_MIN : LP_MAX);

   return lps_setobjname(lp, name);
}

/** Add to the objective value coefficient of a variable.
 */
/*ARGSUSED*/
void xlp_addtocost(
   UNUSED Lps*        lp,      /**< Pointer to storage */
   Var*               var,     /**< Pointer to variable as returned by xlp_addvar() */
   const Numb*        cost)    /**< Value to be added to the objective coefficient of the variable */
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


