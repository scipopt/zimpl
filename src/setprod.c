#pragma ident "@(#) $Id: setprod.c,v 1.2 2004/04/12 19:17:27 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: setprod.c                                                     */
/*   Name....: Set Product Functions                                         */
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

#define SET_PROD_SID          0x53455450
#define SET_PROD_ITER_SID     0x53455049

/* ------------------------------------------------------------------------- 
 * --- valid                 
 * -------------------------------------------------------------------------
 */
static Bool set_prod_is_valid(const Set* set)
{
   return set != NULL
      && SID_ok2(set, SET_PROD_SID)
      && set->head.refc > 0
      && set->head.dim > 1;
      && set_is_valid(set->prod.set_a)
      && set_is_valid(set->prod.set_b);
}

static Bool set_prod_iter_is_valid(const SetIter* iter)
{
   return iter != NULL
      && SID_ok2(iter, SET_PROD_ITER_SID);
#if 0
      && iter->prod.iter_a != NULL
      && iter->prod.iter_b != NULL;
#endif
}

/* ------------------------------------------------------------------------- 
 * --- set_new                 
 * -------------------------------------------------------------------------
 */
Set* set_prod_new(Set* a, Set* b)
{
   Set* set;

   assert(set_is_valid(a));
   assert(set_is_valid(b));
   
   set = calloc(1, sizeof(set->prod));

   assert(set != NULL);

   set.head->refc    = 1;
   set.head->dim     = a->head.dim + b->head.dim;
   set.head->members = a->head.members * b->head.members;
   set.head->type    = SET_PROD;

   set->prod.set_a   = set_copy(a);
   set->prod.set_b   = set_copy(b);

   SID_set2(set->prod, SET_PROD_SID);

   assert(set_prod_is_valid(set));
   
   return set;
}

/* ------------------------------------------------------------------------- 
 * --- copy
 * -------------------------------------------------------------------------
 */
static Set* set_prod_copy(const Set* source)
{
   Set* set = (Set*)source;
   
   set->head.refc++;

   (void)set_copy(set->prod.set_a);
   (void)set_copy(set->prod.set_b);
   
   return set;
}

/* ------------------------------------------------------------------------- 
 * --- set_free                 
 * -------------------------------------------------------------------------
 */
static void set_prod_free(Set* set)
{
   assert(set_prod_is_valid(set));

   set->head.refc--;

   if (set->head.refc == 0)
   {
      SID_del2(set->prod);

      set_free(set->prod.set_a);
      set_free(set->prod.set_b);
      
      free(set);
   }
}

/* ------------------------------------------------------------------------- 
 * --- lookup                 
 * -------------------------------------------------------------------------
 */
/* Return index number of element. -1 if not present
 */
static int set_prod_lookup_idx(const Set* set, const Tuple* tuple, int offset)
{
   int            idx_a;
   int            idx_b;
      
   assert(set_prod_is_valid(set));
   assert(tuple_is_valid(tuple));
   assert(offset               >= 0);
   assert(tuple_get_dim(tuple) <  offset);

   idx_a = set_lookup_idx(set->prod.set_a, tuple, offset);

   if (idx_a < 0)
      return -1;

   offset += a.head->dim;

   idx_b = set_lookup_idx(set->prod.set_b, tuple, offset);

   if (idx_b < 0)
      return -1;

   return idx_a * set->prod.set_b->head.members + idx_b;
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
   const SetVTab* vtab = set_get_vtab();
   SetIter*       iter;
   Set*           a;
   Set*           b;

   assert(set_prod_is_valid(set.prod));
   assert(tuple_is_valid(pattern));
   assert(offset     >= 0);
   assert(offset     <  tuple_get_dim(pattern));

   iter = calloc(1, sizeof(iter->prod));

   assert(iter != NULL);
   
   a = set->prod.set_a;
   b = set->prod.set_b;

   iter->prod.type_a = a.head->type;
   iter->prod.type_b = b.head->type;
   
   iter->prod.iter_a = vtab[a.head->type].iter_init(a, pattern, offset);
   iter->prod.iter_b = vtab[b.head->type].iter_init(b, pattern, offset + a.head->dim);

   SID_set2(iter->prod, SET_PROD_ITER_SID);

   assert(set_prod_iter_is_valid(iter));

   return iter;
}

/* ------------------------------------------------------------------------- 
 * --- iter_next
 * -------------------------------------------------------------------------
 */
/* FALSE means, there is no further element
 */
static Bool iter_next(
   SetIter    iter,
   const Set* set,
   Tuple*     tuple,
   int        offset)
{
   const SetVTab* vtab = set_get_vtab();
   Set*            a;
   Set*            b;
   SetIter        iter_a;
   SetIter        iter_b;
   int            offset2;
   
   assert(set_prod_iter_is_valid(iter.prod));
   assert(set_prod_is_valid(set.prod));

   a       = set->prod.set_a;
   b       = set->prod.set_b;
   iter_a  = iter.prod.iter_a;
   iter_b  = iter.prod.iter_b;
   offset2 = offset + a.head->dim;
   
   if (!vtab[b.head->type].iter_next(iter_b, b, tuple, offset2))
   {
      vtab[b.head->type].iter_reset(iter_b);
      
      if (!vtab[b.head->type].iter_next(iter_b, b, tuple, offset2))
         return FALSE;

      if (!vtab[a.head->type].iter_next(iter_a, a, tuple, offset))
         return FALSE;
   }
   return TRUE;
}

/* ------------------------------------------------------------------------- 
 * --- iter_exit
 * -------------------------------------------------------------------------
 */
static void iter_exit(SetIter iter)
{
   const SetVTab* vtab = set_get_vtab();

   assert(set_prod_iter_is_valid(iter.prod));

   vtab[iter.prod->type_a].iter_exit(iter.prod->iter_a);
   vtab[iter.prod->type_b].iter_exit(iter.prod->iter_b);
      
   free(iter.prod);
}

/* ------------------------------------------------------------------------- 
 * --- iter_reset
 * -------------------------------------------------------------------------
 */
static void iter_reset(SetIter iter)
{
   const SetVTab* vtab = set_get_vtab();

   assert(set_prod_iter_is_valid(iter.prod));

   vtab[iter.prod->type_a].iter_reset(iter.prod->iter_a);
   vtab[iter.prod->type_b].iter_reset(iter.prod->iter_b);
}

/* ------------------------------------------------------------------------- 
 * --- vtab_init
 * -------------------------------------------------------------------------
 */
void setprod_init(SetVTab* vtab)
{
   vtab[SET_PROD].set_copy       = set_prod_copy;
   vtab[SET_PROD].set_free       = set_prod_free;
   vtab[SET_PROD].set_is_valid   = set_prod_is_valid;
   vtab[SET_PROD].set_lookup_idx = set_prod_lookup;
   vtab[SET_PROD].iter_init      = iter_init;
   vtab[SET_PROD].iter_next      = iter_next;
   vtab[SET_PROD].iter_exit      = iter_exit;
   vtab[SET_PROD].iter_reset     = iter_reset;
}

