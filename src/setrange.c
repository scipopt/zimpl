#pragma ident "@(#) $Id: setrange.c,v 1.1 2004/04/12 07:04:16 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: setrange.c                                                    */
/*   Name....: Set Range Functions                                           */
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

#define SET_RANGE_SID          0x53455452
#define SET_RANGE_ITER_SID     0x53455249

struct set_range
{
   SetHead head;   /** head.dim == 1 */
   int     begin;
   int     end;
   int     step;
   SID
};

struct set_range_iter
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
static Bool set_range_is_valid(const SetRange* sr)
{
   return sr != NULL
      && SID_ok(sr, SET_RANGE_SID)
      && sr->head.refc > 0
      && sr->head.dim == 1;
}

static Bool set_range_iter_is_valid(const SetRangeIter* sri)
{
   return sri != NULL
      && SID_ok(sri, SET_RANGE_ITER_SID)
      && sri->first >= 0
      && sri->last  >= 0
      && sri->now   >= 0
      && sri->now   >= sri->first;
}

/* ------------------------------------------------------------------------- 
 * --- lookup                 
 * -------------------------------------------------------------------------
 */
/* Return index number of element. -1 if not present
 */
static int set_range_lookup(const Set set, const Tuple* tuple, int offset)
{
   const Elem*     elem;
   const Numb*     numb;
   int             val;
   
   assert(set_range_is_valid(set.range));
   assert(tuple_is_valid(tuple));
   assert(offset               >= 0);
   assert(tuple_get_dim(tuple) <  offset);

   elem = tuple_get_elem(tuple, offset);

   if (elem_get_type(elem) != ELEM_NUMB)
   {
      fprintf(stderr, "Shit! %d\n", offset);
      return -1;
   }
   numb = elem_get_numb(elem);

   if (!numb_is_int(numb))
   {
      fprintf(stderr, "Minor shit! %d\n", offset);
      return -1;
   }
   val = numb_toint(numb);

   if (  val >= set.range->begin 
      || val <= set.range->end
      || ((val - set.range->begin) % set.range->step) != 0)
      return -1;

   return (val - set.range->begin) / set.range->step;
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
   const Elem*     elem;
   const Numb*     numb;
   int             val;
   
   assert(set_range_is_valid(set.range));
   assert(tuple_is_valid(pattern));
   assert(iter        != NULL);
   assert(iter->range == NULL);
   assert(offset      >= 0);
   assert(offset      <  tuple_get_dim(pattern));

   elem        = tuple_get_elem(pattern, offset);
   iter->range = calloc(1, sizeof(*iter->range));

   assert(iter->range != NULL);
   
   switch(elem_get_type(elem))
   {
   case ELEM_NAME :
      iter->range->first = set.range->begin;
      iter->range->last  = set.range->end;
      break;
   case ELEM_NUMB :
      numb = elem_get_numb(elem);

      if (!numb_is_int(numb))
      {
         fprintf(stderr, "Minor shit! %d\n", offset);
         iter->range->first = 1;
         iter->range->last  = 0;
      }
      else
      {
         val = numb_toint(numb);

         if (  val < set.range->begin
            || val > set.range->end
            || ((val - set.range->begin) % set.range->step) != 0)
         {
            iter->range->first = 1;
            iter->range->last  = 0;
         }
         else
         {
            iter->range->first = (val - set.range->begin) / set.range->step;
            iter->range->last  = iter->range->first;
         }
      }
      break;
   case ELEM_STRG :
      fprintf(stderr, "Shit! %d\n", offset);
      iter->range->first = 1;
      iter->range->last  = 0;
      break;
   default :
      abort();
   }
   iter->range->now = iter->range->first;

   SID_set(iter->range, SET_RANGE_ITER_SID);

   assert(set_range_iter_is_valid(iter->range));
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
   int   val;
   Numb* numb;

   assert(set_range_iter_is_valid(iter.range));
   assert(set_range_is_valid(set.range));
   
   if (iter.range->now > iter.range->last)
      return FALSE;

   val  = iter.range->now;
   numb = numb_new_integer(val);

   tuple_set_elem(tuple, offset, elem_new_numb(numb));

   numb_free(numb);

   iter.range->now += set.range->step;

   return TRUE;
}

/* ------------------------------------------------------------------------- 
 * --- iter_exit
 * -------------------------------------------------------------------------
 */
static void iter_exit(SetIter iter)
{
   assert(set_range_iter_is_valid(iter.range));
   
   free(iter.range);
}

/* ------------------------------------------------------------------------- 
 * --- iter_reset
 * -------------------------------------------------------------------------
 */
static void iter_reset(SetIter iter)
{
   assert(set_range_iter_is_valid(iter.range));
   
   iter.range->now = iter.range->first;
}

/* ------------------------------------------------------------------------- 
 * --- set_free                 
 * -------------------------------------------------------------------------
 */
static void set_range_free(Set set)
{
   assert(set_range_is_valid(set.range));

   set.head->refc--;

   if (set.head->refc == 0)
      free(set.range);
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

void setrange_init(SetVTab* vtab)
{
   vtab[SET_RANGE].set_free   = set_range_free;
   vtab[SET_RANGE].set_lookup = set_range_lookup;
   vtab[SET_RANGE].iter_init  = iter_init;
   vtab[SET_RANGE].iter_next  = iter_next;
   vtab[SET_RANGE].iter_exit  = iter_exit;
   vtab[SET_RANGE].iter_reset = iter_reset;
}













