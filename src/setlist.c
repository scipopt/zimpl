#pragma ident "@(#) $Id: setlist.c,v 1.1 2004/04/12 07:04:15 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: setlist.c                                                     */
/*   Name....: Set List Functions                                            */
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

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "set4.h"

#define SET_LIST_SID          0x5345544c
#define SET_LIST_ITER_SID     0x53454c49

struct set_list
{
   SetHead head;   /** head.dim == 1 */
   Elem* const *  member; /** head.members gives the number */
   Hash*   hash;
   SID
};

struct set_list_iter
{
   int first;
   int last;
   int now;
   SID
};

/* ------------------------------------------------------------------------- 
 * --- valid                 
 * -------------------------------------------------------------------------
 */
static Bool set_list_is_valid(const SetList* sl)
{
   return sl != NULL
      && SID_ok(sl, SET_LIST_SID)
      && sl->head.refc > 0
      && sl->head.dim == 1;
}

static Bool set_list_iter_is_valid(const SetListIter* sli)
{
   return sli != NULL
      && SID_ok(sli, SET_LIST_ITER_SID)
      && sli->first >= 0
      && sli->last  >= 0
      && sli->now   >= 0
      && sli->now   >= sli->first;
}

/* ------------------------------------------------------------------------- 
 * --- lookup                 
 * -------------------------------------------------------------------------
 */
/* Return index number of element. -1 if not present
 */
static int set_list_lookup(const Set set, const Tuple* tuple, int offset)
{
   const Elem* elem;
   int         i;
   
   assert(set_list_is_valid(set.list));
   assert(tuple_is_valid(tuple));
   assert(offset               >= 0);
   assert(tuple_get_dim(tuple) <  offset);
   
   elem = tuple_get_elem(tuple, offset);

   if (set.list->hash != NULL)
      return hash_lookup_elem_idx(set.list->hash, elem);

   for(i = 0; i < set.list->head.members; i++)
      if (!elem_cmp(elem, set.list->member[i]))
         return i;

   return -1;
}

/* ------------------------------------------------------------------------- 
 * --- iter_init                 
 * -------------------------------------------------------------------------
 */
/* Initialise Iterator. Write into iter
 */
static void iter_init(
   SetIter*     iter,
   const Set    set,
   const Tuple* pattern,
   int          offset)
{
   const Elem*    elem;
   int            i;

   assert(set_list_is_valid(set.list));
   assert(tuple_is_valid(pattern));
   assert(iter       != NULL);
   assert(iter->list == NULL);
   assert(offset     >= 0);
   assert(offset     <  tuple_get_dim(pattern));

   elem       = tuple_get_elem(pattern, offset);
   iter->list = calloc(1, sizeof(*iter->list));

   assert(iter->list != NULL);
   
   if (elem_get_type(elem) == ELEM_NAME)
   {
      iter->list->first = 0;
      iter->list->last  = set.head->members - 1;
   }
   else
   {
      for(i = 0; i < set.head->members; i++)
         if (!elem_cmp(elem, set.list->member[i]))
            break;

      iter->list->first = i;
      iter->list->last  = i;

      if (iter->list->last >= set.head->members)
         iter->list->last = set.head->members - 1;
   }
   iter->list->now = iter->list->first;

   SID_set(iter->list, SET_LIST_ITER_SID);

   assert(set_list_iter_is_valid(iter->list));
}

/* ------------------------------------------------------------------------- 
 * --- iter_next
 * -------------------------------------------------------------------------
 */
/* FALSE means, there is no further element
 */
static Bool iter_next(
   SetIter    iter,
   const Set  set,
   Tuple*     tuple,
   int        offset)
{
   assert(set_list_iter_is_valid(iter.list));
   assert(set_list_is_valid(set.list));
   
   if (iter.list->now > iter.list->last)
      return FALSE;

   tuple_set_elem(tuple, offset, elem_copy(set.list->member[iter.list->now]));
   
   iter.list->now++;

   return TRUE;
}

/* ------------------------------------------------------------------------- 
 * --- iter_exit
 * -------------------------------------------------------------------------
 */
static void iter_exit(SetIter iter)
{
   assert(set_list_iter_is_valid(iter.list));
   
   free(iter.list);
}

/* ------------------------------------------------------------------------- 
 * --- iter_reset
 * -------------------------------------------------------------------------
 */
static void iter_reset(SetIter iter)
{
   assert(set_list_iter_is_valid(iter.list));
   
   iter.list->now = iter.list->first;
}

/* ------------------------------------------------------------------------- 
 * --- set_free                 
 * -------------------------------------------------------------------------
 */
static void set_list_free(Set set)
{
   assert(set_list_is_valid(set.list));

   set.head->refc--;

   if (set.head->refc == 0)
   {
      free(set.list->member);
      free(set.list);
   }
}


#if 0
/* ------------------------------------------------------------------------- 
 * --- set_new                 
 * -------------------------------------------------------------------------
 */
static Set* set_new(SetType type, int dim, int members)
{
   Set* set;

   set = calloc(1, sizeof(set));

   assert(set != NULL);

   set->refc    = 1;
   set->dim     = dim;
   set->members = members;
   set->type    = type;

   return set;
}

Set* set_new_hash_from_elem_list(const List* list)
{
   Set* set_hash;
   Set* set_list;
   ListElem*   le    = NULL;
   int  n;

   n = list_get_elems(list);
   
   set_list                     = set_new(SUBSET_LIST, 1, 0);
   set_list->subset.list.size   = n;
   set_list->subset.list.extend = SET_EXTEND_SIZE;
   set_list->subset.list.member = calloc(n, sizeof(*set_list->subset.list.member));
   
   set_hash                     = set_new(SUBSET_HASH, 1, 0);
   set_hash->subset.hash.set    = set_list;
   set_hash->subset.hash.hash   = hash_new(n);

   for(i = 0; i < n; i++)
   {
      elem  = list_get_elem(list, &le);
      tuple = tuple_new(1);

      tuple_set_elem(tuple, 0, elem_copy(elem));

      if (check != SET_CHECK_NONE && set_lookup(set_hash, tuple))
      {
         if (check != SET_CHECK_QUIET)
         {
            assert(check == SET_CHECK_WARN);

            fprintf(stderr, "--- Warning 164: Duplicate element ");
            tuple_print(stderr, tuple);
            fprintf(stderr, " for set rejected\n");
         }
         tuple_free(tuple);
      }
      else
      {
         assert(!set_lookup(set_hash, tuple));
      
         idx = set_list->members;

         set_list->subset.list.member[idx] = tuple;

         hash_add_tuple(set_hash->subset.hash.hash, tuple, idx);
      
         set_list->members++;
         set_hash->members++;
      }
   }
   return set_hash;
}

Set* set_new_hash_from_tuple_list() 
{
   /* - fuer jede dimension eine hash.list.set erzeugen
    * - binaeren baum von cross_erzeugen, oder ueberlegen
    *   ob cross auf mehr als zwei erweitert wird.
    * - hash.oberster_cross.set erzeugen und alle elemente
    *   einhashen.
    */
}
#endif

void setlist_init(SetVTab* vtab)
{
   vtab[SET_LIST].set_free   = set_list_free;
   vtab[SET_LIST].set_lookup = set_list_lookup;
   vtab[SET_LIST].iter_init  = iter_init;
   vtab[SET_LIST].iter_next  = iter_next;
   vtab[SET_LIST].iter_exit  = iter_exit;
   vtab[SET_LIST].iter_reset = iter_reset;
}

const Elem* set_list_get_elem(SetList* sl, int idx)
{
   assert(set_list_is_valid(sl));
   assert(idx >= 0);
   assert(idx <  sl->head.members);

   return sl->member[idx];
}




