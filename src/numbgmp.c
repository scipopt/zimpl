#pragma ident "@(#) $Id: numbgmp.c,v 1.1 2003/07/12 15:24:02 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: numbgmt.c                                                     */
/*   Name....: Number Functions using gmt                                    */
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

#include <gmp.h>

#include "bool.h"
#include "gmpmisc.h"

#include "lint.h"
#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"

#define STORE_SIZE  1000
#define NUMB_SID    0x4e756d62

typedef struct numb_storage NumbStore;

/* Note: In case of gmt use refc and infinity indicator.
 */
struct number
{
   SID
   union 
   {
      mpq_t  numb;
      Numb*  next;
   } value;
};

struct numb_storage
{
   Numb*       begin;
   NumbStore*  next;
};

static NumbStore* store_anchor = NULL;
static Numb*      store_free   = NULL;

/* constants
 */
static Numb* numb_const_zero     = NULL;
static Numb* numb_const_one      = NULL;
static Numb* numb_const_minusone = NULL;

static void extend_storage(void)
{
   NumbStore* store = calloc(1, sizeof(*store));
   Numb*      numb;
   int        i;
   
   assert(store != NULL);
   
   store->begin = malloc(STORE_SIZE * sizeof(*store->begin));
   store->next  = store_anchor;
   store_anchor = store;

   for(i = 0; i < STORE_SIZE - 1; i++)
   {
      numb             = &store->begin[i];
      numb->value.next = &store->begin[i + 1];
      SID_set(numb, NUMB_SID);
      assert(numb_is_valid(numb));
   }
   numb             = &store->begin[i];
   numb->value.next = store_free;  
   SID_set(numb, NUMB_SID);
   assert(numb_is_valid(numb));
   
   store_free       = &store->begin[0];

   assert(store->begin != NULL);
   assert(store_anchor != NULL);
   assert(store_free   != NULL);
}

void numb_init()
{
   store_anchor = NULL;
   store_free   = NULL;

   numb_const_zero     = numb_new();
   numb_const_one      = numb_new_integer(1);
   numb_const_minusone = numb_new_integer(-1);
}

void numb_exit()
{
   NumbStore* store;
   NumbStore* next;
   
   for(store = store_anchor; store != NULL; store = next)
   {
      next = store->next;

      /* ??? Hier werden die mpq_clears fuer die verwendeten nicht aufgerufen.
       */
      free(store->begin);
      free(store);
   }   
}

/* value is zero */
Numb* numb_new(void)
{
   Numb* numb;
   
   if (store_free == NULL)
      extend_storage();

   assert(store_free != NULL);

   numb             = store_free;
   store_free       = numb->value.next;

   mpq_init(numb->value.numb);

   return numb;
}

Numb* numb_new_ascii(const char* val)
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);

   gmp_str2mpq(numb->value.numb, val);
   
   return numb;
}

Numb* numb_new_integer(int val)
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);

   mpq_set_si(numb->value.numb, val, 1); 
   
   return numb;
}

#if 0
Numb* numb_new_posinfty()
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);

   numb->value.numb = INFINITY;
   
   return numb;
}

Numb* numb_new_neginfty()
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);

   numb->value.numb = INFINITY;
   
   return numb;
}
#endif

void numb_free(Numb* numb)
{
   assert(numb_is_valid(numb));

   mpq_clear(numb->value.numb);
   
   numb->value.next = store_free;
   store_free       = numb;
}

Bool numb_is_valid(const Numb* numb)
{
   return numb != NULL && SID_ok(numb, NUMB_SID);
}

#if 0
Bool numb_is_posinfty(const Numb* numb)
{
   return numb->value.numb >= INFINITY;   
}

Bool numb_is_neginfty(const Numb* numb)
{
   return numb->value.numb <= -INFINITY;   
}

Bool numb_is_number(const Numb* numb)
{
   return numb->value.numb > -INFINITY && numb->value.numb < INFINITY;   
}
#endif

Numb* numb_copy(const Numb* source)
{
   Numb* numb = numb_new();

   assert(numb_is_valid(source));
   assert(numb_is_valid(numb));

   mpq_set(numb->value.numb, source->value.numb);

   return numb;
}

/* TRUE wenn gleich, sonst FALSE
 */
Bool numb_equal(const Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   return mpq_equal(numb_a->value.numb, numb_b->value.numb) != 0;
}

/* Return a positive value if op1 > op2, zero if op1 = op2, and a negative value if op1 < op2
 */
int numb_cmp(const Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   return mpq_cmp(numb_a->value.numb, numb_b->value.numb);
}

void numb_set(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_set(numb_a->value.numb, numb_b->value.numb);
}

void numb_add(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_add(numb_a->value.numb, numb_a->value.numb, numb_b->value.numb);
}

Numb* numb_new_add(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_add(numb->value.numb, numb_a->value.numb, numb_b->value.numb);

   return numb;
}

void numb_sub(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_sub(numb_a->value.numb, numb_a->value.numb, numb_b->value.numb);
}

Numb* numb_new_sub(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_sub(numb->value.numb, numb_a->value.numb, numb_b->value.numb);

   return numb;
}

void numb_mul(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_mul(numb_a->value.numb, numb_a->value.numb, numb_b->value.numb);
}

Numb* numb_new_mul(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_mul(numb->value.numb, numb_a->value.numb, numb_b->value.numb);

   return numb;
}

void numb_div(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_div(numb_a->value.numb, numb_a->value.numb, numb_b->value.numb);
}

Numb* numb_new_div(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_div(numb->value.numb, numb_a->value.numb, numb_b->value.numb);

   return numb;
}

void numb_neg(Numb* numb)
{
   assert(numb_is_valid(numb));

   mpq_neg(numb->value.numb, numb->value.numb);
}

void numb_abs(Numb* numb)
{
   assert(numb_is_valid(numb));

   mpq_abs(numb->value.numb, numb->value.numb);
}

double numb_todbl(const Numb* numb)
{
   assert(numb_is_valid(numb));
   
   return mpq_get_d(numb->value.numb);
}

void numb_get_mpq(const Numb* numb, mpq_t value)
{
   assert(numb_is_valid(numb));
   
   mpq_set(value, numb->value.numb);
}

void numb_print(FILE* fp, const Numb* numb)
{
   assert(numb_is_valid(numb));

   fprintf(fp, "%.16g", mpq_get_d(numb->value.numb));
}

unsigned int numb_hash(const Numb* numb)
{
   union
   {
      struct
      {
         unsigned int a;
         unsigned int b;
      } i;
      double d;
   } d2i;
   
   unsigned int hcode;
   
   d2i.d = mpq_get_d(numb->value.numb);
   hcode = d2i.i.a ^ d2i.i.b;

   return hcode;
}

char* numb_tostr(const Numb* numb)
{
   char* str;
   
   assert(numb_is_valid(numb));

   str = malloc(32);
      
   assert(str != NULL);
      
   sprintf(str, "%.16g", mpq_get_d(numb->value.numb));

   return str;
}

const Numb* numb_zero()
{
   return numb_const_zero;
}

const Numb* numb_one()
{
   return numb_const_one;
}

const Numb* numb_minusone()
{
   return numb_const_minusone;
}










