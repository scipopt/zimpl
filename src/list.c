#ident "@(#) $Id: list.c,v 1.4 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: list.c                                                        */
/*   Name....: List Functions                                                */
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

#define LIST_SID  0x4c697374

typedef enum list_type      ListType;
typedef union list_data     ListData; 

enum list_type
{
   LIST_ERR = 0, LIST_ELEM, LIST_TUPLE, LIST_ENTRY, LIST_IDXELEM
};

union list_data
{
   Entry*      entry;
   Tuple*      tuple;
   const Elem* elem;
};

struct list_element
{
   ListData  data;
   ListElem* prev;
   ListElem* next;
};

struct list
{
   SID
   int      refc;
   int      elems;
   ListType type;
   ListElem anchor;
};

static void list_add_data(List* list, ListData* data)
{
   ListElem* elem = calloc(1, sizeof(*elem));

   assert(list_is_valid(list));
   assert(elem != NULL);
   
   elem->data = *data;

   elem->next              = &list->anchor;
   elem->prev              = list->anchor.prev;
   list->anchor.prev->next = elem;
   list->anchor.prev       = elem;
   list->elems++;
}

static List* list_new(ListType type, ListData* data)
{
   List* list = calloc(1, sizeof(*list));
   
   assert(list != NULL);
   assert(data != NULL);

   list->refc        = 1;
   list->elems       = 0;
   list->type        = type;
   list->anchor.prev = &list->anchor;
   list->anchor.next = &list->anchor;
   
   SID_set(list, LIST_SID);
   assert(list_is_valid(list));

   list_add_data(list, data);

   return list;
}

List* list_new_elem(const Elem* elem)
{
   ListData data;
   
   assert(elem_is_valid(elem));

   data.elem = elem;

   return list_new(LIST_ELEM, &data);
}

List* list_new_tuple(Tuple* tuple)
{
   ListData data;
   
   assert(tuple_is_valid(tuple));

   data.tuple = tuple_copy(tuple);

   return list_new(LIST_TUPLE, &data);
}

List* list_new_entry(Entry* entry)
{
   ListData data;
   
   assert(entry_is_valid(entry));

   data.entry = entry_copy(entry);

   return list_new(LIST_ENTRY, &data);
}

void list_free(List* list)
{   
   ListElem* p;
   ListElem* q;
   
   assert(list_is_valid(list));

   list->refc--;

   if (list->refc == 0)
   {
      SID_del(list);

      for(p = list->anchor.next; p != &list->anchor; p = q)
      {
         q = p->next;
         
         switch(list->type)
         {
         case LIST_ELEM :
            break;
         case LIST_TUPLE :
            tuple_free(p->data.tuple);
            break;
         case LIST_ENTRY :
            entry_free(p->data.entry);
            break;
         case LIST_IDXELEM :
            break;
         default :
            abort();
         }
         free(p);
      }
      free(list);
   }
}

Bool list_is_valid(const List* list)
{
   return ((list != NULL) && SID_ok(list, LIST_SID) && (list->refc > 0));
}

List* list_copy(List* list)
{
   assert(list_is_valid(list));

   list->refc++;

   return list;
}

void list_add_elem(List* list, const Elem* elem)
{
   ListData data;

   assert(list_is_valid(list));
   assert(elem_is_valid(elem));
   assert(list->type == LIST_ELEM);
   
   data.elem = elem;

   list_add_data(list, &data);
}

void list_add_tuple(List* list, Tuple* tuple)
{
   ListData data;

   assert(list_is_valid(list));
   assert(tuple_is_valid(tuple));
   assert(list->type == LIST_TUPLE);
   
   data.tuple = tuple_copy(tuple);

   list_add_data(list, &data);
}

void list_add_entry(List* list, Entry* entry)
{
   ListData data;

   assert(list_is_valid(list));
   assert(entry_is_valid(entry));
   assert(list->type == LIST_ENTRY);

   data.entry = entry_copy(entry);

   list_add_data(list, &data);
}

int list_get_elems(const List* list)
{
   assert(list_is_valid(list));

   return list->elems;
}

static ListData* list_get_data(const List* list, ListElem** idxp)
{
   assert(list_is_valid(list));

   if (*idxp == NULL)
      *idxp = list->anchor.next;

   if (*idxp == &list->anchor)
      return NULL;

   *idxp = (*idxp)->next;

   return &((*idxp)->prev->data);
}

const Elem* list_get_elem(const List* list, ListElem** idxp)
{
   ListData* data;
   
   assert(list_is_valid(list));
   assert(list->type == LIST_ELEM);

   data = list_get_data(list, idxp);

   return (data == NULL) ? ELEM_NULL : data->elem;
}

Tuple* list_get_tuple(const List* list, ListElem** idxp)
{
   ListData* data;
   
   assert(list_is_valid(list));
   assert(list->type == LIST_TUPLE);

   data = list_get_data(list, idxp);

   return (data == NULL) ? TUPLE_NULL : tuple_copy(data->tuple);
}

Entry* list_get_entry(const List* list, ListElem** idxp)
{
   ListData* data;
   
   assert(list_is_valid(list));
   assert(list->type == LIST_ENTRY);

   data = list_get_data(list, idxp);

   return (data == NULL) ? ENTRY_NULL : entry_copy(data->entry);
}

void list_print(FILE* fp, const List* list)
{
   ListElem* le;
   
   fprintf(fp, "List Type: %d Elems: %d\n", list->type, list->elems);

   for(le = list->anchor.next; le != &list->anchor; le = le->next)
   {
      switch(list->type)
      {
      case LIST_ELEM :
         elem_print(fp, le->data.elem);
         break;
      case LIST_TUPLE :
         tuple_print(fp, le->data.tuple);
         break;
      case LIST_ENTRY :
         entry_print(fp, le->data.entry);
         break;
      default :
         abort();
      }
      fprintf(fp, "\n");
   }
   fprintf(fp, "---------------------\n");
}



