#pragma ident "@(#) $Id: setmulti.c,v 1.2 2004/04/12 19:17:27 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: setmulti.c                                                    */
/*   Name....: Set Multi Functions                                           */
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "set4.h"

#define SET_MULTI_SID          0x5345544d
#define SET_MULTI_ITER_SID     0x53454d49

struct set_multi
{
   SetHead  head;   /* head.dim > 1  */
   Set*     set;    /* dim times, type == SET_LIST */
   int*     subset; /* members * dim */
   SID
};

struct set_multi_iter
{
   int  dim;
   int  members;
   int  now;
   int* subset;
   SID
};

/* This is a bloody hack. But there seems to be no easy way to give
 * additional information to the compare routine needed for qsort().
 */
static subset_cmp_dim = 0;

/* ------------------------------------------------------------------------- 
 * --- valid                 
 * -------------------------------------------------------------------------
 */
static Bool set_multi_is_valid(const SetMulti* sm)
{
   return sm != NULL
      && SID_ok(sm, SET_MULTI_SID)
      && sm->head.refc > 0
      && sm->head.dim > 1
      && sm->subset != NULL
      && sm->set != NULL;
}

static Bool set_multi_iter_is_valid(const SetMultiIter* smi)
{
   return smi != NULL
      && SID_ok(smi, SET_MULTI_ITER_SID)
      && smi->dim     >= 1 /* might be zero if subset is empty */
      && smi->members >= 0
      && smi->now     >= 0
      && (  (smi->members == 0 && smi->dim == 0 && smi->subset == NULL)
         || (smi->members >= 0 && smi->dim >  0 && smi->subset != NULL));
}

/* ------------------------------------------------------------------------- 
 * --- lookup                 
 * -------------------------------------------------------------------------
 */
static int subset_idx_cmp(const void* a, const void* b)
{
   const int* key    = (const int*)a;
   const int* subset = (const int*)b;
   int        dim;
   int        i;
   int        d;
   
   assert(key    != NULL);
   assert(subset != NULL);
   
   dim = *key;

   key++;

   assert(dim > 0);

   for(i = 0; i < dim; i++)
   {
      d = key[i] - subset[i];

      if (d != 0)
         return d;
   }
   return 0;
}

/* return the index of the element, -1 if not found
 */
static int set_multi_lookup(const Set set, const Tuple* tuple, int offset)
{
   const SetVTab*  vtab = set_get_vtab();
   int*            idx;
   ptrdiff_t       result;
   int             i;
   
   assert(set_multi_is_valid(set.multi));
   assert(tuple_is_valid(tuple));
   assert(offset               >= 0);
   assert(tuple_get_dim(tuple) <  offset);

   idx = malloc(((size_t)set.head->dim + 1) * sizeof(*idx));

   assert(idx != NULL);

   idx[0] = set.head->dim;
   
   for(i = 0; i < set.head->dim; i++)
   {
      idx[i + 1] = vtab[SET_LIST].set_lookup(set.multi->set[i], tuple, offset + i);

      if (idx[i] < 0)
      {
         free(idx);
         return -1;
      }
   }
   result = (ptrdiff_t)bsearch(idx, set.multi->subset, (size_t)set.head->members,
      (size_t)set.head->dim * sizeof(set.multi->subset[0]), subset_idx_cmp);
   
   free(idx);

   if (result == 0)
      return -1;

   assert((result - (ptrdiff_t)set.multi->subset) % set.head->dim == 0);
   
   return (result - (ptrdiff_t)set.multi->subset) / set.head->dim;
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
   const SetVTab* vtab = set_get_vtab();
   const Elem*    elem;
   int*           idx;
   int            i;
   int            j;
   int            k;
   
   assert(set_multi_is_valid(set.multi));
   assert(tuple_is_valid(pattern));
   assert(iter        != NULL);
   assert(iter->multi == NULL);
   assert(offset      >= 0);
   assert(offset      <  tuple_get_dim(pattern));

   iter->multi = calloc(1, sizeof(*iter->multi));

   assert(iter->multi != NULL);

   idx = malloc((size_t)set.head->dim * sizeof(*idx));

   assert(idx != NULL);

   for(i = 0; i < set.head->dim; i++)
   {
      elem = tuple_get_elem(pattern, offset + i);
   
      if (elem_get_type(elem) == ELEM_NAME)
         idx[i] = -1;
      else
      {
         idx[i] = vtab[SET_LIST].set_lookup(set.multi->set[i], pattern, offset + i);

         if (idx[i] < 0)
            break;
      }
   }
   if (i < set.head->dim)
   {
      iter->multi->members = 0;
      iter->multi->now     = 1;
   }
   else
   {
      iter->multi->dim = set.head->dim;
   
      iter->multi->subset = malloc(
         (size_t)set.head->dim * (size_t)set.head->members * sizeof(*iter->multi->subset));

      assert(iter->multi->subset);
      
      j = 0;
      
      for(k = 0; k < set.head->members; k++)
      {
         for(i = 0; i < set.head->dim; i++)
         {
            /* can entry match ?
             */
            if (idx[i] > 0 && idx[i] != set.multi->subset[k * set.head->dim + i])
               break;

            iter->multi->subset[j * set.head->dim + i] = set.multi->subset[k * set.head->dim + i];
         }
         if (i == set.head->dim)
            j++;
      }
      iter->multi->members = j;
      iter->multi->now     = 0;
   }
   free(idx);
   
   SID_set(iter->multi, SET_MULTI_ITER_SID);

   assert(set_multi_iter_is_valid(iter->multi));
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
   int i;
   
   if (iter.multi->now > iter.multi->members)
      return FALSE;

   for(i = 0; i < iter.multi->dim; i++)
   {
      tuple_set_elem(tuple, offset + i,
         elem_copy(set_list_get_elem(set.multi->set[i].list,
            iter.multi->subset[iter.multi->now * iter.multi->dim + i])));
   }
   iter.multi->now++;
   
   return TRUE;
}

/* ------------------------------------------------------------------------- 
 * --- iter_exit
 * -------------------------------------------------------------------------
 */
static void iter_exit(SetIter iter)
{
   assert(set_multi_iter_is_valid(iter.multi));

   if (iter.multi->subset != NULL)
      free(iter.multi->subset);
   
   free(iter.multi);
}

/* ------------------------------------------------------------------------- 
 * --- iter_reset
 * -------------------------------------------------------------------------
 */
static void iter_reset(SetIter iter)
{
   assert(set_multi_iter_is_valid(iter.multi));
   
   iter.multi->now = 0;
}

/* ------------------------------------------------------------------------- 
 * --- set_free                 
 * -------------------------------------------------------------------------
 */
static void set_multi_free(Set set)
{
   const SetVTab* vtab = set_get_vtab();
   int            i;
   
   assert(set_multi_is_valid(set.multi));

   set.head->refc--;

   if (set.head->refc == 0)
   {
      SID_del(set.prod);

      for(i = 0; i < set.head->dim; i++)
         vtab[SET_LIST].set_free(set.multi->set[i]);

      free(set.multi->subset);
      free(set.multi);
   }
}


/* ------------------------------------------------------------------------- 
 * --- set_new                 
 * -------------------------------------------------------------------------
 */
static int subset_cmp(const void* a, const void* b)
{
   const int* aa = (const int*)a;
   const int* bb = (const int*)b;

   int i;
   int d;

   for(i = 0; i < subset_cmp_dim; i++)
   {
      d = aa[i] - bb[i];

      if (d != 0)
         return d;
   }
   return 0;
}

Set set_multi_new_from_tuples(const List* list)
{
   const Elem*  elem;
   const Tuple* tuple;
   ListElem*    le = NULL;
   Set          set;
   int          n;
   int          i;
   int          k;
   
   assert(list_is_valid(list));

   n         = list_get_elems(list);
   tuple     = list_get_tuple(list, &le);
   tuple_dim = tuple_get_dim(tuple);

   assert(tuple_dim > 1);

   set.multi = calloc(1, sizeof(*set.multi));

   assert(set.multi != NULL);

   set.head->refc    = 1;
   set.head->dim     = tuple_dim;
   set.head->members = 0;
   set.head->type    = SET_MULTI;
   set.multi->set    = calloc((size_t)tuple_dim, sizeof(*set.multi->set));
   set.multi->subset = calloc((size_t)(n * tuple_dim), sizeof(*set.multi->subset));

   assert(set.multi->set    != NULL);
   assert(set.multi->subset != NULL);

   for(k = 0; k < tuple_dim; k++)
      set.multi->set[i] = set_list_new(n, SET_DEFAULTS);
   
   le  = NULL;

   for(i = 0; i < n; i++)
   {
      tuple = list_get_tuple(list, &le);

      for(k = 0; k < tuple_dim; k++)
      {
         set.multi->subset[i * tuple_dim + k] =
            set_list_add_elem(set.multi->set[i], tuple_get_elem(tuple, k), SET_CHECK_QUIET);
         set.head->members++;
      }
   }
   /* Sort entries
    */
   /* Bloody hack!
    */
   subset_cmp_dim = tuple_dim;
   
   qsort(set.multi->subset, set.head->members,
      tuple_dim * sizeof(*set.multi->subset), subset_cmp);
   
   /* Remove doubles
    */
   i = 1;
   
   while(i < set.head->members)
   {
      if (subset_cmp(
         &set.multi->subset[i       * tuple_dim],
         &set.multi->subset[(i - 1) * tuple_dim]) == 0)
      {
         for(k = i * tuple_dim; k < (set.head->members - 1) * tuple_dim; k++)
            set.multi->subset[k] = set.multi->subset[k + tuple_dim];

         fprintf(stderr, "Doublicate found!!\n");
         set.head->members--;
      }
      else
         i++;
   }
   return set;
}


void setmulti_init(SetVTab* vtab)
{
   vtab[SET_MULTI].set_free   = set_multi_free;
   vtab[SET_MULTI].set_lookup = set_multi_lookup;
   vtab[SET_MULTI].iter_init  = iter_init;
   vtab[SET_MULTI].iter_next  = iter_next;
   vtab[SET_MULTI].iter_exit  = iter_exit;
   vtab[SET_MULTI].iter_reset = iter_reset;
}





