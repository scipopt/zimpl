#pragma ident "@(#) $Id: strstore.c,v 1.8 2006/09/09 10:00:22 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: strstore.c                                                    */
/*   Name....: String Storage Functions                                      */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2006 by Thorsten Koch <koch@zib.de>
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

#include "lint.h"
#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"

typedef struct string Str;

struct string
{
   char* value;
   Str*  next;
};

static Str* anchor = NULL;

const char* str_new(const char* s)
{
   Str* str = malloc(sizeof(*str));

   assert(s   != NULL);
   assert(str != NULL);

   str->value = strdup(s);
   str->next  = anchor;
   anchor     = str;

   assert(str->value != NULL);
   
   return str->value;
}

void str_init(void)
{
}

void str_exit(void)
{
   Str* p;
   Str* q;

   for(p = anchor; p != NULL; p = q)
   {
      q = p->next;
      free(p->value);
      free(p);
   }
   anchor = NULL;
}

unsigned int str_hash(const char* s)
{
   unsigned int sum;
   int          i;
   
   for(sum = 0, i = 0; s[i] != '\0'; i++)
      sum = sum * 31 + s[i];

   return sum;
}

