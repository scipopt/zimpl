#pragma ident "@(#) $Id: set4.c,v 1.1 2004/04/12 19:17:27 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: set.c                                                         */
/*   Name....: Set Functions                                                 */
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

#define TEST_DUBLICATE   0

#define SET_SID          0x5365745f

#ifndef NDEBUG
static
#endif
SetVTab* set_vtab_global = NULL;

SetVTab* set_get_vtab(void)
{
   return set_vtab_global;
}

Set set_new_from_list(const List* list)
{
   ListElem* le = NULL;

   assert(list_get_elems(list) > 0);
   
   if (list_is_elemlist(list))
      return set_list_new_from_elems(list);

   assert(list_is_tuplelist(list));

   dim = tuple_get_dim(list_get_tuple(list, &le));

   if (dim == 1)
      return set_list_new_from_tuples(list);

   return set_multi_new_from_tuples(list);
}

void set_free(Set* set)
{
   set_vtab_gobal[set.head->type].set_free(set);
}

Bool set_is_valid(const Set* set)
{
   set_vtab_gobal[set.head->type].set_is_valid(set);
}

Set* set_copy(const Set* set)
{
   return set_vtab_gobal[set.head->type].set_copy(set);
}

int set_lookup_idx(const Set* set, const Tuple* tuple, int offset)
{
   return set_vtab_gobal[set.head->type].set_lookup_idx(set, tuple, offset);
}

Bool set_lookup(const Set* set, const Tuple* tuple)
{
   return set_vtab_gobal[set.head->type].set_lookup_idx(set, tuple, 0) >= 0;
}

SetIter* set_iter_init(const Set* set, const Tuple* pattern)
{
   return set_vtab_gobal[set.head->type].set_iter_init(set, pattern, 0);
}

Bool set_iter_next(SetIter* iter, const Set* set, Tuple* tuple)
{
   return set_vtab_gobal[set.head->type].set_iter_next(iter, set, tuple, 0);
}
     
void set_iter_exit(SetIter* iter)
{
   set_vtab_gobal[set.head->type].set_iter_next(iter);
}
     
int set_get_dim(const Set* set)
{
   assert(set_is_valid(set));

   return set.head->dim;   
}

int set_get_used(const Set* set)
{
   assert(set_is_valid(set));

   return set.head->members;   
}

void set_print(FILE* fp, const Set* set)
{
   int i;

   assert(fp != NULL);
   assert(set_is_valid(set));

   fprintf(fp, "|%d|", set->dim);
   fprintf(fp, "{");

   for(i = 0; i < set->used; i++)
   {
      tuple_print(fp, set->member[i]);
      fprintf(fp, "%s", (i < set->used - 1) ? "," : "");
   }
   fprintf(fp, "}");
}

#if 0
Set* set_union(const Set* set_a, const Set* set_b)
{
   Set* set;
   int  i;
   
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));
   assert(set_a->dim == set_b->dim);
   
   set = set_new(set_a->dim, set_a->used + set_b->used, SET_DEFAULT);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      set_add_member(set, tuple_copy(set_a->member[i]), SET_ADD_END, SET_CHECK_NONE);    

   for(i = 0; i < set_b->used; i++)
      set_add_member(set, tuple_copy(set_b->member[i]), SET_ADD_END, SET_CHECK_QUIET);    

   assert(set_is_valid(set));

   return set;
}

/* In A and in B */
Set* set_inter(const Set* set_a, const Set* set_b)
{
   Set* set;
   int  i;
   
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));
   assert(set_a->dim == set_b->dim);
   
   set = set_new(set_a->dim, Min(set_a->used, set_b->used), SET_DEFAULT);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (set_lookup(set_b, set_a->member[i]))
         set_add_member(set, tuple_copy(set_a->member[i]), SET_ADD_END, SET_CHECK_NONE);   

   assert(set_is_valid(set));

   return set;
}

/* In A but not in B */
Set* set_minus(const Set* set_a, const Set* set_b)
{
   Set* set;
   int  i;
   
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));
   assert(set_a->dim == set_b->dim);
   
   set = set_new(set_a->dim, set_a->used, SET_DEFAULT);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (!set_lookup(set_b, set_a->member[i]))
         set_add_member(set, tuple_copy(set_a->member[i]), SET_ADD_END, SET_CHECK_NONE); 

   assert(set_is_valid(set));

   return set;
}

/* In A and not in B or in B and not in A  (Symetric difference) */
Set* set_sdiff(const Set* set_a, const Set* set_b)
{
   Set* set;
   int  i;
   
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));
   assert(set_a->dim == set_b->dim);
   
   set = set_new(set_a->dim, set_a->used + set_b->used, SET_DEFAULT);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (!set_lookup(set_b, set_a->member[i]))
         set_add_member(set, tuple_copy(set_a->member[i]), SET_ADD_END, SET_CHECK_NONE);  

   for(i = 0; i < set_b->used; i++)
      if (!set_lookup(set_a, set_b->member[i]))
         set_add_member(set, tuple_copy(set_b->member[i]), SET_ADD_END, SET_CHECK_NONE); 

   assert(set_is_valid(set));

   return set;
}

/* project set_a to a new set, using the elements index in the tuple.
 */
Set* set_proj(const Set* set_a, const Tuple* pattern)
{
   Set*   set;
   Tuple* tuple;
   int    i;
   int    k;
   int    dim;
   int*   idx;
   
   assert(set_is_valid(set_a));
   assert(tuple_is_valid(pattern));

   dim = tuple_get_dim(pattern);
   set = set_new(dim, set_a->used, SET_DEFAULT);
   idx = malloc(sizeof(*idx) * dim);

   assert(set != NULL);
   assert(idx != NULL);
   
   for(i = 0; i < dim; i++)
   {
      assert(numb_is_int(elem_get_numb(tuple_get_elem(pattern, i))));
      
      idx[i] = numb_toint(elem_get_numb(tuple_get_elem(pattern, i))) - 1;
   }
      
   for(k = 0; k < set_a->used; k++)
   {
      tuple = tuple_new(dim);

      for(i = 0; i < dim; i++)
         tuple_set_elem(tuple, i, elem_copy(tuple_get_elem(set_a->member[k], idx[i]))); 

      set_add_member(set, tuple, SET_ADD_END, SET_CHECK_QUIET);
   }
   free(idx);
   
   assert(set_is_valid(set));

   return set;
}

/* Is A subset (or equal) of B */
Bool set_is_subseteq(const Set* set_a, const Set* set_b)
{
   int  i;
   
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));

   /* If A is the empty set and B is non empty,
    * A is a subset of B
    */
   if (set_a->used == 0)
      return TRUE;
   
   if (set_a->dim != set_b->dim)
   {
      fprintf(stderr, "--- Warning 165: Comparison of different dimension sets.\n");
      set_print(stderr, set_a);
      fputc('\n', stderr);
      set_print(stderr, set_b);
      fputc('\n', stderr);
      return FALSE;
   }
   if (set_a->used > set_b->used)
      return FALSE;
   
   for(i = 0; i < set_a->used; i++)
      if (!set_lookup(set_b, set_a->member[i]))
         return FALSE;
   
   return TRUE;
}

/* A is real subset of B */
Bool set_is_subset(const Set* set_a, const Set* set_b)
{
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));

   if (set_a->used >= set_b->used)
      return FALSE;

   return set_is_subseteq(set_a, set_b);
}

/* A has the same elements as B */
Bool set_is_equal(const Set* set_a, const Set* set_b)
{
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));

   if (set_a->used != set_b->used)
      return FALSE;

   return set_is_subseteq(set_a, set_b);
}

/* n elements in set
 * k elements in subset
 * i index for counter
 */
/*lint -sem(counter_inc, 1p == 3n && 2n > 0 && 3n > 0 && 4n >= 0, @n > 0) */
static int counter_inc(int* counter, int n, int k, int i)
{
   int ret = 0;
   
   counter[i]++;

   if (counter[i] == n - i)
   {
      if (i == k - 1)
         return 1;
      
      ret = counter_inc(counter, n, k, i + 1);

      counter[i] = counter[i + 1] + 1;
   }
   return ret;
}

List* set_subsets_list(
   const Set* set,
   int        subset_size,
   List*      list,
   int*       idx)
{
   int*   counter;
   int    i;
   Set*   subset;
   Tuple* tuple;
   Entry* entry;
   Numb*  numb;
   
   assert(set_is_valid(set));
   assert(subset_size <= set->used);
   assert(idx         != NULL);
   
   counter = malloc(sizeof(*counter) * subset_size);

   assert(counter != NULL);
   
   for(i = 0; i < subset_size; i++)
      counter[i] = subset_size - 1 - i;

   do
   {
      subset = set_new(set->dim, subset_size, SET_NO_HASH);

      for(i = 0; i < subset_size; i++)
         set_add_member(subset, tuple_copy(set->member[counter[i]]),
            SET_ADD_END, SET_CHECK_NONE);

      numb  = numb_new_integer(*idx);
      *idx += 1;
      tuple = tuple_new(1);
      tuple_set_elem(tuple, 0, elem_new_numb(numb));
      entry = entry_new_set(tuple, subset);

      if (list == NULL)
         list = list_new_entry(entry);
      else
         list_add_entry(list, entry);

      numb_free(numb);
      entry_free(entry);
      tuple_free(tuple);
      set_free(subset);
   }
   while(!counter_inc(counter, set->used, subset_size, 0));

   free(counter);

   assert(list != NULL);

   return list;
}

#endif









