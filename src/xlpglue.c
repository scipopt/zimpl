#pragma ident "@(#) $Id: xlpglue.c,v 1.1 2003/07/12 15:24:02 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: numb2lp.c                                                     */
/*   Name....: Term Functions                                                */
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

#include <gmp.h>

#include "bool.h"
#include "mshell.h"
#include "ratlp.h"
#include "mme.h"
#include "gmpmisc.h"
#include "xlpglue.h"

static Lps* lp = NULL;

void xlp_alloc(const char* name)
{
   lp = lps_alloc(name);
}

void xlp_scale()
{
   lps_scale(lp);
}

void xlp_write(FILE* fp, LpFormat format)
{
   lps_write(lp, fp, format);
}

void xlp_transtable(FILE* fp)
{
   lps_transtable(lp, fp, 8, "zimpl");
}

void xlp_orderfile(FILE* fp)
{
   //   lps_orderfile(lp, fp);
}

void xlp_free()
{
   lps_free(lp);
   lp = NULL;
}

Bool xlp_conname_exists(const char* conname)
{
   return NULL != lps_getcon(lp, conname);
}

Con* xlp_addcon(
   const char*  name,
   ConType      contype,
   const Numb*  rhs,
   unsigned int flags)
{
   Con* con;
   mpq_t temp;
   
   assert(name != NULL);
   assert(rhs  != NULL);
   
   con = lps_addcon(lp, name);

   assert(con != NULL);

   mpq_init(temp);
   numb_get_mpq(rhs, temp);
   
   switch(contype)
   {
   case CON_RHS :
      lps_setrhs(con, temp);
      break;
   case CON_LHS :
      lps_setlhs(con, temp);
      break;
   case CON_EQUAL :
      lps_setrhs(con, temp);
      lps_setlhs(con, temp);
      break;
   default :
      abort();
   }
   lps_addflags(con, flags);

   mpq_clear(temp);
   
   return con;
}

Var* xlp_addvar(
   const char* name,
   VarClass    varclass,
   const Numb* lower,
   const Numb* upper,
   const Numb* priority,
   const Numb* startval)
{
   Var*  var;
   mpq_t temp;
   
   assert(name  != NULL);
   assert(lower != NULL);
   assert(upper != NULL);
   
   var = lps_addvar(lp, name);

   mpq_init(temp);

   numb_get_mpq(lower, temp);
   lps_setlower(var, temp);

   numb_get_mpq(upper, temp);
   lps_setupper(var, temp);

   mpq_clear(temp);
   
   return var;
}

void xlp_objname(const char* name)
{
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

   mpq_init(val1);
   mpq_init(val2);

   lps_getcost(var, val1);
   numb_get_mpq(cost, val2);
   mpq_add(val1, val1, val2);
   lps_setcost(var, val1);

   mpq_clear(val1);
   mpq_clear(val2);
}



     
