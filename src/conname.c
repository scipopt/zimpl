#ident "@(#) $Id: conname.c,v 1.1 2002/07/29 07:51:44 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: conname.c                                                     */
/*   Name....: Instructions                                                  */
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

#include "portab.h"
#include "mshell.h"
#include "mme.h"

static const char* cpfix = NULL;
static char*       cname = NULL;
static int         count = 0;

void conname_free()
{
   if (cname != NULL)
      free(cname);

   cname = NULL;
   count = 0;
}
      
void conname_set(const char* prefix)
{
   assert(prefix != NULL);

   cpfix = prefix;

   if (cname != NULL)
      free(cname);

   cname = malloc(strlen(cpfix) + 16);
   count = 1;
   
   assert(cname != NULL);
}

const char* conname_get()
{
   assert(cpfix != NULL);
   assert(cname != NULL);

   sprintf(cname, "%s_%d", cpfix, count);

   count++;

   return cname;
}



