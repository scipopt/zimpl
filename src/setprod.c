#pragma ident "@(#) $Id: setprod.c,v 1.1 2004/04/12 07:04:16 bzfkocht Exp $"
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

struct set_prod
{
   SetHead head;   /** head.dim > 1 */
   Set     set_a;
   Set     set_b;
   SID
};

struct set_prod_iter
{
   SetType type_a;
   SetType type_b;
   SetIter iter_a;
   SetIter iter_b;
   SID
};

/* ------------------------------------------------------------------------- 
 * --- valid                 
 * -------------------------------------------------------------------------
 */
static Bool set_prod_is_valid(const SetProd* sp)
{
   return sp != NULL
      && SID_ok(sp, SET_PROD_SID)
      && sp->head.refc > 0
      && sp->head.dim > 1;
#if 0
      && set_is_valid(sp->set_a)
      && set_is_valid(sp->set_b);
#endif
}

static Bool set_prod_iter_is_valid(const SetProdIter* spi)
{
   return spi != NULL
      && SID_ok(spi, SET_PROD_ITER_SID);
#if 0
      && spi->iter_a != NULL
      && spi->iter_b != NULL;
#endif
}


/* ------------------------------------------------------------------------- 
 * --- lookup                 
 * -------------------------------------------------------------------------
 */
/* Return index number of element. -1 if not present
 */
static int set_prod_lookup(const Set set, const Tuple* tuple, int offset)
{
   const SetVTab* vtab  = set_get_vtab();
   int            idx_a;
   int            idx_b;
   Set            a;
   Set            b;
      
   assert(set_prod_is_valid(set.prod));
   assert(tuple_is_valid(tuple));
   assert(offset               >= 0);
   assert(tuple_get_dim(tuple) <  offset);

   a = set.prod->set_a;
   b = set.prod->set_b;
   
   idx_a = vtab[a.head->type].set_lookup(a, tuple, offset);

   if (idx_a < 0)
      return -1;

   offset += a.head->dim;

   idx_b = vtab[b.head->type].set_lookup(b, tuple, offset);

   if (idx_b < 0)
      return -1;

   return idx_a * b.head->members + idx_b;
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
   Set            a;
   Set            b;

   assert(set_prod_is_valid(set.prod));
   assert(tuple_is_valid(pattern));
   assert(iter       != NULL);
   assert(iter->list == NULL);
   assert(offset     >= 0);
   assert(offset     <  tuple_get_dim(pattern));
   assert(iter       != NULL);

   iter->prod = calloc(1, sizeof(*iter->prod));

   assert(iter->prod != NULL);
   
   a = set.prod->set_a;
   b = set.prod->set_b;

   iter->prod->type_a = a.head->type;
   iter->prod->type_b = b.head->type;
   
   vtab[a.head->type].iter_init(&iter->prod->iter_a, a, pattern, offset);
   vtab[b.head->type].iter_init(&iter->prod->iter_b, b, pattern, offset + a.head->dim);

   SID_set(iter->prod, SET_PROD_ITER_SID);

   assert(set_prod_iter_is_valid(iter->prod));
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
   const SetVTab* vtab = set_get_vtab();
   Set            a;
   Set            b;
   SetIter        iter_a;
   SetIter        iter_b;
   int            offset2;
   
   assert(set_prod_iter_is_valid(iter.prod));
   assert(set_prod_is_valid(set.prod));

   a       = set.prod->set_a;
   b       = set.prod->set_b;
   iter_a  = iter.prod->iter_a;
   iter_b  = iter.prod->iter_b;
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
 * --- set_free                 
 * -------------------------------------------------------------------------
 */
static void set_prod_free(Set set)
{
   const SetVTab* vtab = set_get_vtab();
   Set            a;
   Set            b;

   assert(set_prod_is_valid(set.prod));

   set.head->refc--;

   if (set.head->refc == 0)
   {
      a = set.prod->set_a;
      b = set.prod->set_b;

      vtab[a.head->type].set_free(a);
      vtab[b.head->type].set_free(b);
      
      free(set.prod);
   }
}


#if 0
/* ------------------------------------------------------------------------- 
 * --- set_new                 
 * -------------------------------------------------------------------------
 */
Set* set_new_range(int beg, int end, int step)
{
   Set* set;

   set = set_new(SUBSET_RANGE, 1, /* anzahl ausrechnen */);

   set->subset.range.beg  = beg;
   set->subset.range.end  = end;
   set->subset.range.step = step;

   return set;
}
#endif

void setprod_init(SetVTab* vtab)
{
   vtab[SET_PROD].set_free   = set_prod_free;
   vtab[SET_PROD].set_lookup = set_prod_lookup;
   vtab[SET_PROD].iter_init  = iter_init;
   vtab[SET_PROD].iter_next  = iter_next;
   vtab[SET_PROD].iter_exit  = iter_exit;
   vtab[SET_PROD].iter_reset = iter_reset;
}





