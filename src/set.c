#ident "@(#) $Id: set.c,v 1.6 2001/05/06 11:43:21 thor Exp $"
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

#include "portab.h"
#include "mshell.h"
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

Set* set_new(int dim)
{
   Set* set;

   assert(dim >= 0);
   
   set = calloc(1, sizeof(*set));

   assert(set != NULL);

   set->size    = 1;
   set->used    = 0;
   set->extend  = SET_EXTEND_SIZE;
   set->refc    = 1;
   set->dim     = dim;
   set->hash    = hash_new(HASH_TUPLE);
   
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

Set* set_copy(Set* set)
{
   assert(set_is_valid(set));

   set->refc++;

   return set;
}

void set_add_member(Set* set, Tuple* tuple, int where)
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

   if (hash_has_tuple(set->hash, tuple))
   {
      fprintf(stderr, "*** Warning: Dublicate element ");
      tuple_print(stderr, tuple);
      fprintf(stderr, " for set rejected\n");
   }
   else
   {
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

Tuple* set_match_next(const Set* set, const Tuple* pattern, int* idx)
{
   Tuple* tuple;
   
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

   return tuple_copy(tuple);
}
     
int set_get_dim(const Set* set)
{
   assert(set_is_valid(set));

   return set->dim;   
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
   fprintf(fp, "}\n");
}

Set* set_range(double start, double end, double step)
{
   Set*      set;
   Tuple*    tuple;
   double    x;
   
   set = set_new(1);

   for(x = start; LE(x, end); x += step)
   {
      tuple = tuple_new(1);
      tuple_set_elem(tuple, 0, elem_new_numb(x));
      set_add_member(set, tuple, SET_ADD_END);
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
   set = set_new(dim);

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

         set_add_member(set, tuple, SET_ADD_END);
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
   
   set = set_new(set_a->dim);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      set_add_member(set, set_a->member[i], SET_ADD_END);         

   for(i = 0; i < set_b->used; i++)
      set_add_member(set, set_b->member[i], SET_ADD_END);         

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
   
   set = set_new(set_a->dim);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (set_lookup(set_b, set_a->member[i]))
         set_add_member(set, set_a->member[i], SET_ADD_END);         

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
   
   set = set_new(set_a->dim);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (!set_lookup(set_b, set_a->member[i]))
         set_add_member(set, set_a->member[i], SET_ADD_END);         

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
   
   set = set_new(set_a->dim);

   assert(set != NULL);
   
   for(i = 0; i < set_a->used; i++)
      if (!set_lookup(set_b, set_a->member[i]))
         set_add_member(set, set_a->member[i], SET_ADD_END);         

   for(i = 0; i < set_b->used; i++)
      if (!set_lookup(set_a, set_b->member[i]))
         set_add_member(set, set_b->member[i], SET_ADD_END);         

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
