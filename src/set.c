#pragma ident "@(#) $Id: set.c,v 1.15 2003/07/16 13:32:08 bzfkocht Exp $"
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

#define TEST_DUBLICATE   0

#define SET_EXTEND_SIZE  100
#define SET_SID          0x5365745f

struct set
{
   SID
   int     dim;
   int     size;
   int     used;
   int     extend;
   int     refc;
   Hash*   hash;
   Tuple** member;
};

Set* set_new(int dim, int estimated_size)
{
   Set* set;

   assert(dim            >= 0);
   assert(estimated_size >= 0);
   
   set = calloc(1, sizeof(*set));

   assert(set != NULL);
   
   set->size    = 1;
   set->used    = 0;
   set->extend  = SET_EXTEND_SIZE;
   set->refc    = 1;
   set->dim     = dim;
   set->hash    = hash_new(HASH_TUPLE, estimated_size);
   
   /* Falls wir dim == 0 haben, liefert das nachfolgende calloc je
    * nach Implementierung einen pointer oder NULL zurueck.
    * Um die sonderfaelle mit der NULL auszuschliessen, also...
    */
   set->member = calloc((size_t)set->size, sizeof(*set->member));

   assert(set->member != NULL);

   SID_set(set, SET_SID);

   assert(set_is_valid(set));
      
   return set;
}

void set_free(Set* set)
{
   int i;
   
   assert(set_is_valid(set));
   assert(set->member != NULL);

   set->refc--;
   
   if (set->refc == 0)
   {
      SID_del(set);
      
      for(i = 0; i < set->used; i++)
         tuple_free(set->member[i]);

      hash_free(set->hash);
      free(set->member);
      free(set);
   }
}

Bool set_is_valid(const Set* set)
{
   return ((set != NULL) && SID_ok(set, SET_SID) && (set->refc > 0));
}

Set* set_copy(const Set* source)
{
   Set* set = (Set*)source;
   
   assert(set_is_valid(set));

   set->refc++;

   return set;
}

void set_add_member(
   Set*         set,
   const Tuple* tuple,
   SetAddType   where,
   SetCheckType check)
{
   int idx;
   int i;
   
   assert(set_is_valid(set));
   assert(tuple     != NULL);
   assert(set->used <= set->size);
   assert(set->refc == 1); /* ??? ewtl. ok */
   
   if (set->used == set->size)
   {
      set->size   += set->extend;
      set->extend += set->extend;
      set->member  = realloc(
         set->member, (size_t)set->size * sizeof(*set->member));

      assert(set->member != NULL);
   }
   assert(set->used < set->size);

   assert(set->dim == tuple_get_dim(tuple));

   if (check != SET_CHECK_NONE && hash_has_tuple(set->hash, tuple))
   {
      if (check != SET_CHECK_QUIET)
      {
         assert(check == SET_CHECK_WARN);

         fprintf(stderr, "*** Warning: Dublicate element ");
         tuple_print(stderr, tuple);
         fprintf(stderr, " for set rejected\n");
      }
   }
   else
   {
      assert(!hash_has_tuple(set->hash, tuple));
      
      if (where == SET_ADD_END)
         idx = set->used;
      else
      {
         assert(where == SET_ADD_BEGIN);
         
         for(i = set->used; i > 0; i--)
            set->member[i] = set->member[i - 1];
         idx = 0;
      }
      set->member[idx] = tuple_copy(tuple);
      
      hash_add_tuple(set->hash, set->member[idx]);
      
      set->used++;
   }
}

Bool set_lookup(const Set* set, const Tuple* tuple)
{
   assert(set_is_valid(set));
   assert(tuple_is_valid(tuple));

   return hash_has_tuple(set->hash, tuple);
}

const Tuple* set_match_next(const Set* set, const Tuple* pattern, int* idx)
{
   const Tuple* tuple;
   
   assert(set_is_valid(set));
   assert(tuple_is_valid(pattern));
   assert(*idx    >= 0);

   while((*idx < set->used) && tuple_cmp(pattern, set->member[*idx]))
      (*idx)++;

   if (*idx >= set->used)
      return NULL;

   assert(!tuple_cmp(pattern, set->member[*idx]));

   tuple = set->member[*idx];
   
   (*idx)++;

   return tuple;
}
     
int set_get_dim(const Set* set)
{
   assert(set_is_valid(set));

   return set->dim;   
}

int set_get_used(const Set* set)
{
   assert(set_is_valid(set));

   return set->used;   
}

void set_print(FILE* fp, const Set* set)
{
   int i;

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

Set* set_range(int start, int end, int step)
{
   Set*      set;
   Tuple*    tuple;
   Numb*     numb;
   int       x;
   
   set = set_new(1, (abs(end - start) / step) + 1);

   for(x = start; x <= end; x += step)
   {
      tuple = tuple_new(1);
      numb  = numb_new_integer(x);
      tuple_set_elem(tuple, 0, elem_new_numb(numb));
      set_add_member(set, tuple, SET_ADD_END, SET_CHECK_NONE);
      numb_free(numb);
      tuple_free(tuple);
   }
   assert(set_is_valid(set));

   return set;   
}

Set* set_cross(const Set* set_a, const Set* set_b)
{
   Set*   set;
   Tuple* tuple;
   int    dim;
   int    i;
   int    j;
   int    k;
   int    idx;
   
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));

   dim = set_a->dim + set_b->dim;
   set = set_new(dim, set_a->used * set_b->used);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
   {
      for(j = 0; j < set_b->used; j++)
      {
         tuple = tuple_new(dim);
         idx   = 0;
         
         for(k = 0; k < set_a->dim; k++)
            tuple_set_elem(tuple, idx++, tuple_get_elem(set_a->member[i], k));

         for(k = 0; k < set_b->dim; k++)            
            tuple_set_elem(tuple, idx++, tuple_get_elem(set_b->member[j], k));

         set_add_member(set, tuple, SET_ADD_END, SET_CHECK_NONE);
         
         tuple_free(tuple);
      }
   }
   assert(set_is_valid(set));

   return set;
}

Set* set_union(const Set* set_a, const Set* set_b)
{
   Set* set;
   int  i;
   
   assert(set_is_valid(set_a));
   assert(set_is_valid(set_b));
   assert(set_a->dim == set_b->dim);
   
   set = set_new(set_a->dim, set_a->used + set_b->used);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      set_add_member(set, set_a->member[i], SET_ADD_END, SET_CHECK_NONE);    

   for(i = 0; i < set_b->used; i++)
      set_add_member(set, set_b->member[i], SET_ADD_END, SET_CHECK_QUIET);    

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
   
   set = set_new(set_a->dim, Min(set_a->used, set_b->used));

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (set_lookup(set_b, set_a->member[i]))
         set_add_member(set, set_a->member[i], SET_ADD_END, SET_CHECK_NONE);   

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
   
   set = set_new(set_a->dim, set_a->used);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (!set_lookup(set_b, set_a->member[i]))
         set_add_member(set, set_a->member[i], SET_ADD_END, SET_CHECK_NONE); 

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
   
   set = set_new(set_a->dim, set_a->used + set_b->used);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (!set_lookup(set_b, set_a->member[i]))
         set_add_member(set, set_a->member[i], SET_ADD_END, SET_CHECK_NONE);  

   for(i = 0; i < set_b->used; i++)
      if (!set_lookup(set_a, set_b->member[i]))
         set_add_member(set, set_b->member[i], SET_ADD_END, SET_CHECK_NONE); 

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
   set = set_new(dim, set_a->used);
   idx = malloc(sizeof(*idx) * dim);

   assert(set != NULL);
   assert(idx != NULL);
   
   for(i = 0; i < dim; i++)
      idx[i] = (int)numb_todbl(elem_get_numb(tuple_get_elem(pattern, i))) - 1; /* ??? */
      
   for(k = 0; k < set_a->used; k++)
   {
      tuple = tuple_new(dim);

      for(i = 0; i < dim; i++)
         tuple_set_elem(tuple, i, tuple_get_elem(set_a->member[k], idx[i])); 

      set_add_member(set, tuple, SET_ADD_END, SET_CHECK_QUIET);

      tuple_free(tuple);
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

   if (set_a->dim != set_b->dim)
   {
      fprintf(stderr, "*** Warning: Comparison of differen dimension sets.\n");
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
   Elem*  elem;
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
      subset = set_new(set->dim, subset_size);

      for(i = 0; i < subset_size; i++)
         set_add_member(subset, set->member[counter[i]],
            SET_ADD_END, SET_CHECK_NONE);

      numb  = numb_new_integer(*idx);
      elem  = elem_new_numb(numb);
      *idx += 1;
      tuple = tuple_new(1);
      tuple_set_elem(tuple, 0, elem);
      entry = entry_new_set(tuple, subset);

      if (list == NULL)
         list = list_new_entry(entry);
      else
         list_add_entry(list, entry);

      numb_free(numb);
      entry_free(entry);
      tuple_free(tuple);
      elem_free(elem);
      set_free(subset);
   }
   while(!counter_inc(counter, set->used, subset_size, 0));

   free(counter);

   assert(list != NULL);

   return list;
}











