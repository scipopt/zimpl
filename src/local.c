#ident "@(#) $Id: local.c,v 1.1 2001/01/26 07:11:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: local.c                                                       */
/*   Name....: Local Parameter Functions                                     */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
   int    frame = FALSE;
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

   return local == NULL ? NULL : local->element;
}

void local_install_tuple(const Tuple* pattern, const Tuple* values)
{
   const Elem* elem;
   const char* name;
   int         i;
   
   assert(pattern != NULL);
   assert(values  != NULL);
   
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
