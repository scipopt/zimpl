#pragma ident "@(#) $Id: setlist.c,v 1.2 2004/04/12 19:17:27 bzfkocht Exp $"
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

#define HASH_THRESHOLD        12

/* ------------------------------------------------------------------------- 
 * --- valid                 
 * -------------------------------------------------------------------------
 */
static Bool set_list_is_valid(const Set* set)
{
   return set != NULL
      && SID_ok2(set->list, SET_LIST_SID)
      && set->head.refc    >  0
      && set->head.dim     == 1
      && set->head.members >= 0
      && set->list.size    >= set->head.members
      && set->list.member  != NULL;
}

static Bool set_list_iter_is_valid(const SetIter*iter)
{
   return iter != NULL
      && SID_ok2(iter->list, SET_LIST_ITER_SID)
      && iter->list.first >= 0
      && iter->list.last  >= 0
      && iter->list.now   >= 0
      && iter->list.now   >= iter->list.first;
}

/* ------------------------------------------------------------------------- 
 * --- internal                 
 * -------------------------------------------------------------------------
 */
/* Return index number of element. -1 if not present
 */
static int lookup_elem_idx(const Set* set, const Elem* elem)
{
   int i;
   
   assert(set_list_is_valid(set));
   assert(elem_is_valid(elem));

   if (set->list.hash != NULL)
      return hash_lookup_elem_idx(set->list.hash, elem);

   for(i = 0; i < set->list.head.members; i++)
      if (!elem_cmp(elem, set->list.member[i]))
         return i;

   return -1;
}

/* ------------------------------------------------------------------------- 
 * --- new                 
 * -------------------------------------------------------------------------
 */
Set* set_list_new(int size, int flags)
{
   Set* set;

   assert(size > 0);
   
   set = calloc(1, sizeof(set->list));

   assert(set != NULL);

   set->head.refc    = 1;
   set->head.dim     = 1;
   set->head.members = 0;
   set->head.type    = SET_LIST;
   set->list.size    = size;
   set->list.member  = calloc((size_t)size, sizeof(*set->list.member));

   assert(set->list.member != NULL);

   if ((flags & SET_NO_HASH) == 0 && size > HASH_THRESHOLD)
      set->list.hash = hash_new(HASH_ELEM_IDX, size);

   SID_set2(set->list, SET_LIST_SID);

   assert(set_list_is_valid(set));
   
   return set;
}

int set_list_add_elem(Set* set, const Elem* elem, SetCheckType check)
{
   int idx = -1;
   
   assert(set_list_is_valid(set));
   assert(elem_is_valid(elem));
   
   if (check != SET_CHECK_NONE && (idx = lookup_elem_idx(set, elem)) >= 0)
   {
      if (check != SET_CHECK_QUIET)
      {
         assert(check == SET_CHECK_WARN);

         fprintf(stderr, "--- Warning 164: Duplicate element ");
         elem_print(stderr, elem);
         fprintf(stderr, " for set rejected\n");
      }
   }
   else
   {
      idx = set->head.members;

      set->list.member[idx] = elem_copy(elem); 

      if (set->list.hash != NULL)
         hash_add_elem_idx(set->list.hash, elem, idx);
      
      set->head.members++;

      assert(set->head.members <= set->list.size);
   }
   assert(idx >= 0);
   
   return idx;
}

Set* set_list_new_from_elems(const List* list)
{
   ListElem*   le = NULL;
   Set*         set;
   int         n;

   assert(list_is_valid(list));

   n   = list_get_elems(list);
   set = set_list_new(n, SET_DEFAULT);

   while(n-- > 0)
      (void)set_list_add_elem(set, list_get_elem(list, &le), SET_CHECK_WARN);

   assert(set_list_is_valid(set));

   return set;
}

Set* set_list_new_from_tuples(const List* list)
{
   ListElem*    le = NULL;
   const Tuple* tuple;
   Set*         set;
   int          n;

   assert(list_is_valid(list));

   n   = list_get_elems(list);
   set = set_list_new(n, SET_DEFAULT);

   while(n-- > 0)
   {
      tuple = list_get_tuple(list, &le);

      assert(tuple_get_dim(tuple) == 1);
      
      (void)set_list_add_elem(set, tuple_get_elem(tuple, 0), SET_CHECK_WARN);
   }
   assert(set_list_is_valid(set));

   return set;
}

/* ------------------------------------------------------------------------- 
 * --- copy
 * -------------------------------------------------------------------------
 */
static Set* set_list_copy(const Set* source)
{
   Set* set = (Set*)source;
   
   set->head.refc++;

   return set;
}

/* ------------------------------------------------------------------------- 
 * --- free                 
 * -------------------------------------------------------------------------
 */
static void set_list_free(Set* set)
{
   int i;
   
   assert(set_list_is_valid(set));

   set->head.refc--;

   if (set->head.refc == 0)
   {
      SID_del2(set->list);

      for(i = 0; i < set->head.members; i++)
         elem_free(set->list.member[i]);

      if (set->list.hash != NULL)
         hash_free(set->list.hash);
      
      free(set->list.member);
      free(set);
   }
}

/* ------------------------------------------------------------------------- 
 * --- lookup                 
 * -------------------------------------------------------------------------
 */
/* Return index number of element. -1 if not present
 */
static int set_list_lookup_idx(const Set* set, const Tuple* tuple, int offset)
{
   assert(set_list_is_valid(set));
   assert(tuple_is_valid(tuple));
   assert(offset               >= 0);
   assert(tuple_get_dim(tuple) <  offset);
   
   return lookup_elem_idx(set, tuple_get_elem(tuple, offset));
}

/* ------------------------------------------------------------------------- 
 * --- iter_init                 
 * -------------------------------------------------------------------------
 */
/* Initialise Iterator. Write into iter
 */
static SetIter* iter_init(
   const Set*   set,
   const Tuple* pattern,
   int          offset)
{
   const Elem*  elem;
   SetIter*     iter;
   int          i;

   assert(set_list_is_valid(set));
   assert(tuple_is_valid(pattern));
   assert(offset        >= 0);
   assert(offset        <  tuple_get_dim(pattern));

   elem = tuple_get_elem(pattern, offset);
   iter = calloc(1, sizeof(iter->list));
   
   assert(iter != NULL);
   
   if (elem_get_type(elem) == ELEM_NAME)
   {
      iter->list.first = 0;
      iter->list.last  = set->head.members - 1;
   }
   else
   {
      for(i = 0; i < set->head.members; i++)
         if (!elem_cmp(elem, set->list.member[i]))
            break;

      iter->list.first = i;
      iter->list.last  = i;

      if (iter->list.last >= set->head.members)
         iter->list.last = set->head.members - 1;
   }
   iter->list.now = iter->list.first;

   SID_set2(iter->list, SET_LIST_ITER_SID);

   assert(set_list_iter_is_valid(iter));

   return iter;
}

/* ------------------------------------------------------------------------- 
 * --- iter_next
 * -------------------------------------------------------------------------
 */
/* FALSE means, there is no further element
 */
static Bool iter_next(
   SetIter*   iter,
   const Set* set,
   Tuple*     tuple,
   int        offset)
{
   assert(set_list_iter_is_valid(iter));
   assert(set_list_is_valid(set));
   
   if (iter->list.now > iter->list.last)
      return FALSE;

   tuple_set_elem(tuple, offset, elem_copy(set->list.member[iter->list.now]));
   
   iter->list.now++;

   return TRUE;
}

/* ------------------------------------------------------------------------- 
 * --- iter_exit
 * -------------------------------------------------------------------------
 */
static void iter_exit(SetIter* iter)
{
   assert(set_list_iter_is_valid(iter));
   
   free(iter);
}

/* ------------------------------------------------------------------------- 
 * --- iter_reset
 * -------------------------------------------------------------------------
 */
static void iter_reset(SetIter* iter)
{
   assert(set_list_iter_is_valid(iter));
   
   iter->list.now = iter->list.first;
}

/* ------------------------------------------------------------------------- 
 * --- vtab_init
 * -------------------------------------------------------------------------
 */
void setlist_init(SetVTab* vtab)
{
   vtab[SET_LIST].set_copy       = set_list_copy;
   vtab[SET_LIST].set_free       = set_list_free;
   vtab[SET_LIST].set_is_valid   = set_list_is_valid;
   vtab[SET_LIST].set_lookup_idx = set_list_lookup_idx;
   vtab[SET_LIST].iter_init      = iter_init;
   vtab[SET_LIST].iter_next      = iter_next;
   vtab[SET_LIST].iter_exit      = iter_exit;
   vtab[SET_LIST].iter_reset     = iter_reset;
}

/* ------------------------------------------------------------------------- 
 * --- extras
 * -------------------------------------------------------------------------
 */
const Elem* set_list_get_elem(const Set* set, int idx)
{
   assert(set_list_is_valid(set));
   assert(idx >= 0);
   assert(idx <  set->head.members);

   return set->list.member[idx];
}




