#pragma ident "@(#) $Id: setmulti.c,v 1.3 2004/04/13 13:59:57 bzfkocht Exp $"
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

/* This is a bloody hack. But there seems to be no easy way to give
 * additional information to the compare routine needed for qsort().
 */
static int subset_cmp_dim = 0;

/* ------------------------------------------------------------------------- 
 * --- valid                 
 * -------------------------------------------------------------------------
 */
static Bool set_multi_is_valid(const Set* set)
{
   return set != NULL
      && SID_ok2(set->multi, SET_MULTI_SID)
      && set->head.refc    > 0
      && set->head.dim     > 1
      && set->multi.subset != NULL
      && set->multi.set    != NULL;
}

static Bool set_multi_iter_is_valid(const SetIter* iter)
{
   return iter != NULL
      && SID_ok2(iter->multi, SET_MULTI_ITER_SID)
      && iter->multi.dim     >= 1 /* might be zero if subset is empty */
      && iter->multi.members >= 0
      && iter->multi.now     >= 0
      && (  (iter->multi.members == 0 && iter->multi.dim == 0 && iter->multi.subset == NULL)
         || (iter->multi.members >= 0 && iter->multi.dim >  0 && iter->multi.subset != NULL));
}

/* ------------------------------------------------------------------------- 
 * --- internal                 
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

/* ------------------------------------------------------------------------- 
 * --- new                 
 * -------------------------------------------------------------------------
 */
Set* set_multi_new_from_list(const List* list)
{
   const Tuple* tuple;
   ListElem*    le = NULL;
   Set*         set;
   int          n;
   int          i;
   int          k;
   int          dim;
   Bool         is_entrylist;
   
   assert(list_is_valid(list));

   is_entrylist = list_is_entrylist(list);
   
   n     = list_get_elems(list);
   tuple = is_entrylist
      ? entry_get_tuple(list_get_entry(list, &le))
      : list_get_tuple(list, &le);
   dim   = tuple_get_dim(tuple);

   assert(n   > 1);
   assert(dim > 1);
   
   set = calloc(1, sizeof(*set));
   
   assert(set != NULL);

   set->head.refc    = 1;
   set->head.dim     = dim;
   set->head.members = 0;
   set->head.type    = SET_MULTI;
   set->multi.set    = calloc((size_t)dim, sizeof(*set->multi.set));
   set->multi.subset = calloc((size_t)(n * dim), sizeof(*set->multi.subset));

   assert(set->multi.set    != NULL);
   assert(set->multi.subset != NULL);

   for(k = 0; k < dim; k++)
      set->multi.set[k] = set_list_new(n, SET_DEFAULT);
   
   le  = NULL;

   for(i = 0; i < n; i++)
   {
      tuple = is_entrylist
         ? entry_get_tuple(list_get_entry(list, &le))
         : list_get_tuple(list, &le);

      for(k = 0; k < dim; k++)
         set->multi.subset[i * dim + k] =
            set_list_add_elem(set->multi.set[k], tuple_get_elem(tuple, k), SET_CHECK_QUIET);

      set->head.members++;
   }
   /* Sort entries
    */
   /* Bloody hack!
    */
   subset_cmp_dim = dim;
   
   qsort(set->multi.subset, (size_t)set->head.members,
      (size_t)dim * sizeof(*set->multi.subset), subset_cmp);
   
   /* Remove doubles
    */
   i = 1;
   
   while(i < set->head.members)
   {
      if (subset_cmp(
         &set->multi.subset[i       * dim],
         &set->multi.subset[(i - 1) * dim]) == 0)
      {
         for(k = i * dim; k < (set->head.members - 1) * dim; k++)
            set->multi.subset[k] = set->multi.subset[k + dim];

         fprintf(stderr, "Doublicate found!!\n");
         set->head.members--;
      }
      else
         i++;
   }
   SID_set2(set->multi, SET_MULTI_SID);

   assert(set_multi_is_valid(set));
   
   return set;
}

/* ------------------------------------------------------------------------- 
 * --- copy
 * -------------------------------------------------------------------------
 */
static Set* set_multi_copy(const Set* source)
{
   Set* set = (Set*)source;
   int  i;
   
   assert(set_multi_is_valid(source));

   set->head.refc++;

   for(i = 0; i < set->head.dim; i++)
      (void)set_copy(set->multi.set[i]);
   
   return set;
}

/* ------------------------------------------------------------------------- 
 * --- free                 
 * -------------------------------------------------------------------------
 */
static void set_multi_free(Set* set)
{
   int i;
   
   assert(set_multi_is_valid(set));

   set->head.refc--;

   if (set->head.refc == 0)
   {
      SID_del2(set->multi);

      for(i = 0; i < set->head.dim; i++)
         set_free(set->multi.set[i]);

      free(set->multi.subset);
      free(set);
   }
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
static int set_multi_lookup_idx(const Set* set, const Tuple* tuple, int offset)
{
   int*       idx;
   ptrdiff_t  result;
   int        i;
   
   assert(set_multi_is_valid(set));
   assert(tuple_is_valid(tuple));
   assert(offset >= 0);
   assert(offset <  tuple_get_dim(tuple));

   idx = malloc(((size_t)set->head.dim + 1) * sizeof(*idx));

   assert(idx != NULL);

   idx[0] = set->head.dim;
   
   for(i = 0; i < set->head.dim; i++)
   {
      idx[i + 1] = set_lookup_idx(set->multi.set[i], tuple, offset + i);

      if (idx[i] < 0)
      {
         free(idx);
         return -1;
      }
   }
   result = (ptrdiff_t)bsearch(idx, set->multi.subset, (size_t)set->head.members,
      (size_t)set->head.dim * sizeof(set->multi.subset[0]), subset_idx_cmp);
   
   free(idx);

   if (result == 0)
      return -1;

   assert((result - (ptrdiff_t)set->multi.subset) % set->head.dim == 0);
   
   return (result - (ptrdiff_t)set->multi.subset) / set->head.dim;
}

/* ------------------------------------------------------------------------- 
 * --- iter_init                 
 * -------------------------------------------------------------------------
 */
static SetIter* iter_init(
   const Set*   set,
   const Tuple* pattern,
   int          offset)
{
   SetIter*     iter;
   int*         idx;
   int          i;
   int          j;
   int          k;
   
   assert(set_multi_is_valid(set));
   assert(pattern == NULL || tuple_is_valid(pattern));
   assert(offset      >= 0);
   assert(pattern == NULL || offset < tuple_get_dim(pattern));

   iter = calloc(1, sizeof(*iter));

   assert(iter != NULL);

   idx = malloc((size_t)set->head.dim * sizeof(*idx));

   assert(idx != NULL);

   for(i = 0; i < set->head.dim; i++)
   {
      if (pattern == NULL
         || elem_get_type(tuple_get_elem(pattern, offset + i)) == ELEM_NAME)
         idx[i] = -1;
      else
      {
         idx[i] = set_lookup_idx(set->multi.set[i], pattern, offset + i);
         
         if (idx[i] < 0)
            break;
      }
   }
   if (i < set->head.dim)
   {
      iter->multi.members = 0;
      iter->multi.now     = 1;
   }
   else
   {
      iter->multi.dim = set->head.dim;
   
      iter->multi.subset = malloc(
         (size_t)set->head.dim * (size_t)set->head.members * sizeof(*iter->multi.subset));

      assert(iter->multi.subset);
      
      j = 0;
      
      for(k = 0; k < set->head.members; k++)
      {
         for(i = 0; i < set->head.dim; i++)
         {
            /* can entry match ?
             */
            if (idx[i] > 0 && idx[i] != set->multi.subset[k * set->head.dim + i])
               break;

            iter->multi.subset[j * set->head.dim + i] = set->multi.subset[k * set->head.dim + i];
         }
         if (i == set->head.dim)
            j++;
      }
      iter->multi.members = j;
      iter->multi.now     = 0;
   }
   free(idx);
   
   SID_set2(iter->multi, SET_MULTI_ITER_SID);

   assert(set_multi_iter_is_valid(iter));

   return iter;
}

/* ------------------------------------------------------------------------- 
 * --- iter_next
 * -------------------------------------------------------------------------
 */
/* FALSE means, there is no further element
 */
static Bool iter_next(
   SetIter*    iter,
   const Set*  set,
   Tuple*     tuple,
   int        offset)
{
   int i;
   
   if (iter->multi.now >= iter->multi.members)
      return FALSE;

   for(i = 0; i < iter->multi.dim; i++)
   {
      tuple_set_elem(tuple, offset + i,
         elem_copy(set_list_get_elem(set->multi.set[i],
            iter->multi.subset[iter->multi.now * iter->multi.dim + i])));
   }
   iter->multi.now++;
   
   return TRUE;
}

/* ------------------------------------------------------------------------- 
 * --- iter_exit
 * -------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void iter_exit(SetIter* iter, const Set* set)
{
   assert(set_multi_iter_is_valid(iter));

   SID_del2(iter->multi);
   
   if (iter->multi.subset != NULL)
      free(iter->multi.subset);

   free(iter);
}

/* ------------------------------------------------------------------------- 
 * --- iter_reset
 * -------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void iter_reset(SetIter* iter, const Set* set)
{
   assert(set_multi_iter_is_valid(iter));
   
   iter->multi.now = 0;
}

/* ------------------------------------------------------------------------- 
 * --- vtab_init
 * -------------------------------------------------------------------------
 */
void set_multi_init(SetVTab* vtab)
{
   vtab[SET_MULTI].set_copy       = set_multi_copy;
   vtab[SET_MULTI].set_free       = set_multi_free;
   vtab[SET_MULTI].set_is_valid   = set_multi_is_valid;
   vtab[SET_MULTI].set_lookup_idx = set_multi_lookup_idx;
   vtab[SET_MULTI].iter_init      = iter_init;
   vtab[SET_MULTI].iter_next      = iter_next;
   vtab[SET_MULTI].iter_exit      = iter_exit;
   vtab[SET_MULTI].iter_reset     = iter_reset;
}







