#pragma ident "@(#) $Id: ratordwrite.c,v 1.7 2004/04/27 09:56:02 bzfkocht Exp $"
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
   char*       vtmp;
   int         namelen;
   
   assert(lp     != NULL);
   assert(fp     != NULL);
   assert(format == LP_FORM_LPF || format == LP_FORM_MPS);
   
   namelen = (format == LP_FORM_MPS) ? MPS_NAME_LEN : LPF_NAME_LEN;
   vtmp    = malloc((size_t)namelen + 1);

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
      
      lps_makename(vtmp, namelen + 1, var->name, var->number);

      /* if it is within the allowed MPS name length, we try to conform
       * to the MPS specification. This is not really important, since
       * CPLEX does not care.
       */
      if (namelen <= MPS_NAME_LEN)
         fprintf(fp, "    %-8.8s  %8d", vtmp, var->priority);
      else
         fprintf(fp, "    %s  %8d", vtmp, var->priority);
         
      fprintf(fp, "  %.10e", mpq_get_d(var->startval));

      fputc('\n', fp);
   }
   fprintf(fp, "ENDATA\n");

   free(vtmp);
}   


