#pragma ident "@(#) $Id: ratordwrite.c,v 1.8 2006/08/22 20:11:09 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: ratordwrite.c                                                 */
/*   Name....: ORD File Write                                                */
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

#include "mshell.h"
#include "bool.h"
#include "ratlp.h"
#include "ratlpstore.h"
#include "mme.h"

/* A specification for the ORD file format can be found in the
 * ILOG CPLEX 7.0 Reference Manual page 545.
 */
void lps_orderfile(
   const Lps*  lp,
   FILE*       fp,
   LpFormat    format,
   const char* text)
{
   const Var*  var;
   int         name_size;
   char*       vtmp;
   
   assert(lp     != NULL);
   assert(fp     != NULL);
   assert(format == LP_FORM_LPF || format == LP_FORM_MPS);

   name_size = lps_getnamesize(lp, format);
   vtmp      = malloc((size_t)name_size);

   assert(vtmp != NULL);
   
   if (text != NULL)
      fprintf(fp, "* %s\n", text);
   
   fprintf(fp, "NAME        %8.8s\n", lp->name);

   for(var = lp->var_root; var != NULL; var = var->next)
   {
      if (var->class == VAR_CON)
         continue;

      if (var->size == 0)
          continue;

      if (var->type == VAR_FIXED)
         continue;
      
      lps_makename(vtmp, name_size, var->name, var->number);

      fprintf(fp, "    %-8s  %8d", vtmp, var->priority);
      fprintf(fp, "  %.10e\n", mpq_get_d(var->startval));
   }
   fprintf(fp, "ENDATA\n");

   free(vtmp);
}   


