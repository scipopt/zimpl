#ident "@(#) $Id: local.c,v 1.5 2001/03/09 16:12:36 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: local.c                                                       */
/*   Name....: Local Parameter Functions                                     */
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

struct local
{
   const char*  name;
   const Elem*  element;
   Local*       next;
};

static Local anchor  = { "", NULL, NULL };

static void local_new(const char* name, const Elem* element)
{
   Local* local;

   assert(name    != NULL);
   
   local = calloc(1, sizeof(*local));

   assert(local != NULL);
   
   local->name    = name;
   local->element = element;
   local->next    = anchor.next;
   anchor.next    = local;
}

static void local_new_frame(void)
{
   local_new("", NULL);
}

void local_drop_frame(void)
{
   Bool   frame = FALSE;
   Local* q     = NULL;
   Local* p;
   
   for(p = anchor.next; (p != NULL) && !frame; p = q)
   {
      q = p->next;

      if (p->element == NULL)
         frame = TRUE;
      
      free(p);
   }
   anchor.next = q;
}

const Elem* local_lookup(const char* name)
{
   Local* local;

   assert(name != NULL);

   for(local = anchor.next; local != NULL; local = local->next)
      if (!strcmp(local->name, name))
         break;

   return local == NULL ? ELEM_NULL : local->element;
}

void local_install_tuple(const Tuple* pattern, const Tuple* values)
{
   const Elem* elem;
   const char* name;
   int         i;
   
   assert(tuple_is_valid(pattern));
   assert(tuple_is_valid(values));
   
   assert(tuple_get_dim(pattern) == tuple_get_dim(values));

   local_new_frame();
   
   for(i = 0; i < tuple_get_dim(pattern); i++)
   {
      elem = tuple_get_elem(pattern, i);

      if (elem_get_type(elem) == ELEM_NAME)
      {
         name = elem_get_name(elem);
         elem = tuple_get_elem(values, i);

         assert(elem_get_type(elem) != ELEM_NAME);

         local_new(name, elem);
      }
   }
}

void local_print_all(FILE* fp)
{
   const Local* local;

   for(local = anchor.next; local != NULL; local = local->next)
   {
      fprintf(fp, "[%s] ", local->name);
      if (local->element == NULL)
         fprintf(fp, "New Frame");
      else
         elem_print(fp, local->element);
      fprintf(fp, "\n");
   }
}
