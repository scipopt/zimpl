/* $Id: stkchk.c,v 1.4 2014/03/03 16:44:25 bzfkocht Exp $ */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: stkchk.c                                                      */
/*   Name....: Stack usage checker                                           */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2011-2014 by Thorsten Koch <koch@zib.de>
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
#ifndef NDEBUG

#include <stdio.h>
#include "stkchk.h"

const void*  stkchk_start = 0;
size_t       stkchk_maxi  = 0;

void stkchk_init_x()
{
   int a;

   stkchk_start = &a; /*lint !e789 Assigning address of auto variable 'a' to static */
   stkchk_maxi  = 0;
}

size_t stkchk_used_x()
{
   size_t used = (size_t)((const char*)stkchk_start - (const char*)&used);

   if (used > stkchk_maxi)
      stkchk_maxi = used;

   return used;
}

void stkchk_maximum_x(FILE* fp)
{
   (void)fprintf(fp, "Maximum amount of stack used = %lu bytes\n",
      (unsigned long)stkchk_maxi);
}

void stkchk_display_x(FILE* fp)
{
   (void)fprintf(fp, "Current amount of stack used = %lu bytes\n",
      (unsigned long)stkchk_used());
}

#endif /* NDEBUG */
