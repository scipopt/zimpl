#ident "@(#) $Id: tuple.c,v 1.3 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: tuple.c                                                       */
/*   Name....: Tuple Functions                                               */
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

#define TUPLE_SID  0x5475706c

struct tuple
{
   SID
   int          dim;
   int          refc;
   const Elem** element;
};

Tuple* tuple_new(int dim)
{
   Tuple* tuple;
   int    count;
   
   assert(dim >= 0);
   
   tuple = malloc(sizeof(*tuple));

   assert(tuple != NULL);

   /* Machen wir hier nur, weil einige Compiler bei malloc(0)
    * als Rueckgabe NULL liefern. Das mag das free() nicht.
    */
   count          = dim < 1 ? 1 : dim;
   tuple->dim     = dim;
   tuple->refc    = 1;
   tuple->element = calloc((size_t)count, sizeof(*tuple->element));

   assert(tuple->element != NULL);

   SID_set(tuple, TUPLE_SID);
   assert(tuple_is_valid(tuple));
   
   return tuple;
}

void tuple_free(Tuple* tuple)
{
   assert(tuple_is_valid(tuple));
   assert(tuple->element != NULL);

   tuple->refc--;

   if (tuple->refc == 0)
   {
      SID_del(tuple);

      free(tuple->element);   
      free(tuple);
   }
}

Bool tuple_is_valid(const Tuple* tuple)
{
   return ((tuple != NULL) && SID_ok(tuple, TUPLE_SID) && (tuple->refc > 0));
}

Tuple* tuple_copy(Tuple* tuple)
{
   assert(tuple_is_valid(tuple));

   tuple->refc++;

   return tuple;
}

/* 1 = verschieden,
 * 0 = gleich.
 */
Bool tuple_cmp(const Tuple* tuple_a, const Tuple* tuple_b)
{   
   int i;

   assert(tuple_is_valid(tuple_a));
   assert(tuple_is_valid(tuple_b));

   if (tuple_a == tuple_b)
      return FALSE;
   
   if (tuple_a->dim != tuple_b->dim)
   {
      fprintf(stderr, "Warning: Comparison of different dimension tuples\n");

      return FALSE;
   }
   assert(tuple_a->dim == tuple_b->dim);
   
   for(i = 0; i < tuple_a->dim; i++)
      if (elem_cmp(tuple_a->element[i], tuple_b->element[i]))
         break;

   return i < tuple_a->dim;
}

int tuple_get_dim(const Tuple* tuple)
{
   assert(tuple_is_valid(tuple));

   return tuple->dim;
}

void tuple_set_elem(Tuple* tuple, int idx, const Elem* elem)
{
   assert(tuple_is_valid(tuple));
   assert(idx         >= 0);
   assert(idx         <  tuple->dim);
   assert(tuple->refc == 1);
   
   assert(tuple->element[idx] == NULL);
   
   tuple->element[idx] = elem;
}

const Elem* tuple_get_elem(const Tuple* tuple, int idx)
{
   assert(tuple_is_valid(tuple));
   assert(idx   >= 0);
   assert(idx   <  tuple->dim);

   /* Abfrage eines noch nicht gesetzten Elements ist illegal.
    */
   assert(tuple->element[idx] != NULL);
   
   return tuple->element[idx];
}

Tuple* tuple_combine(const Tuple* ta, const Tuple* tb)
{
   Tuple* tuple;
   int    i;
   
   assert(tuple_is_valid(ta));
   assert(tuple_is_valid(tb));

   tuple = tuple_new(ta->dim + tb->dim);

   for(i = 0; i < ta->dim; i++)
      tuple->element[i] = ta->element[i];

   for(i = 0; i < tb->dim; i++)
      tuple->element[ta->dim + i] = tb->element[i];
   
   return tuple;
}

void tuple_print(FILE* fp, const Tuple* tuple)
{
   int i;

   assert(tuple_is_valid(tuple));
   
   fprintf(fp, "<");
      
   for(i = 0; i < tuple->dim; i++)
   {
      elem_print(fp, tuple->element[i]);
      
      fprintf(fp, "%s", (i < tuple->dim - 1) ? "," : "");
   }
   fprintf(fp, ">");
}

unsigned int tuple_hash(const Tuple* tuple)
{
   unsigned int hcode = 0;
   int          i;
   
   for(i = 0; i < tuple->dim; i++)
      hcode = DISPERSE(hcode + elem_hash(tuple->element[i]));

   return hcode;
}

