#pragma ident "@(#) $Id: numbgmp.c,v 1.8 2003/09/16 14:24:29 bzfkocht Exp $"
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

/* #define TRACE 1 */

#include "bool.h"
#include "lint.h"
#include "mshell.h"
#include "gmpmisc.h"
#include "ratlptypes.h"
#include "mme.h"

#define NUMB_STORE_SIZE  1000
#define NUMB_SID         0x4e756d62

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
static int        store_count  = 0;

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
   
   store->begin = malloc(NUMB_STORE_SIZE * sizeof(*store->begin));
   store->next  = store_anchor;
   store_anchor = store;

   for(i = 0; i < NUMB_STORE_SIZE - 1; i++)
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

   numb_free(numb_const_zero);
   numb_free(numb_const_one);
   numb_free(numb_const_minusone);

   if (store_count != 0)
      printf("Numb store count %d\n", store_count);
   
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

   Trace("numb_new");
   
   if (store_free == NULL)
      extend_storage();

   assert(store_free != NULL);

   numb             = store_free;
   store_free       = numb->value.next;
   store_count++;

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

void numb_free(Numb* numb)
{
   Trace("numb_free");

   assert(numb_is_valid(numb));

   mpq_clear(numb->value.numb);
   
   numb->value.next = store_free;
   store_free       = numb;

   store_count--;   
}

Bool numb_is_valid(const Numb* numb)
{
   return numb != NULL && SID_ok(numb, NUMB_SID);
}

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

void numb_intdiv(Numb* numb_a, const Numb* numb_b)
{
   mpz_t q;

   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_div(numb_a->value.numb, numb_a->value.numb, numb_b->value.numb);

   mpz_init(q);
   mpz_tdiv_q(q, mpq_numref(numb_a->value.numb), mpq_denref(numb_a->value.numb));
   mpq_set_z(numb_a->value.numb, q);
   mpz_clear(q);
}

Numb* numb_new_intdiv(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();
   mpz_t q;

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_div(numb->value.numb, numb_a->value.numb, numb_b->value.numb);

   mpz_init(q);
   mpz_tdiv_q(q, mpq_numref(numb->value.numb), mpq_denref(numb->value.numb));
   mpq_set_z(numb->value.numb, q);
   mpz_clear(q);

   return numb;
}

void numb_mod(Numb* numb_a, const Numb* numb_b)
{
   mpz_t r;

   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_div(numb_a->value.numb, numb_a->value.numb, numb_b->value.numb);

   mpz_init(r);
   mpz_tdiv_r(r, mpq_numref(numb_a->value.numb), mpq_denref(numb_a->value.numb));
   mpq_set_z(numb_a->value.numb, r);
   mpz_clear(r);
}

Numb* numb_new_mod(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();
   mpz_t        r;

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpq_div(numb->value.numb, numb_a->value.numb, numb_b->value.numb);

   mpz_init(r);
   mpz_tdiv_r(r, mpq_numref(numb->value.numb), mpq_denref(numb->value.numb));
   mpq_set_z(numb->value.numb, r);
   mpz_clear(r);

   return numb;
}

Numb* numb_new_pow(const Numb* base, int expo)
{
   Numb* numb = numb_new();
   int   i;
   Bool  is_negative = FALSE;
   
   assert(numb != NULL);
   assert(numb_is_valid(base));

   if (expo < 0)
   {
      is_negative = TRUE;
      expo        = -expo;
   }
   mpq_set_si(numb->value.numb, 1, 1);  /* set to 1 */

   for(i = 1; i <= expo; i++)
      mpq_mul(numb->value.numb, numb->value.numb, base->value.numb);

   if (is_negative)
      mpq_inv(numb->value.numb, numb->value.numb);
   
   return numb;
}

Numb* numb_new_fac(int n)
{
   Numb* numb = numb_new();
   mpz_t z;
   
   assert(numb != NULL);
   assert(n    >= 0);

   mpz_init(z);
   mpz_fac_ui(z, n);
   mpq_set_z(numb->value.numb, z);
   mpz_clear(z);
   
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

void numb_sgn(Numb* numb)
{
   assert(numb_is_valid(numb));

   switch(mpq_sgn(numb->value.numb))
   {
   case -1 :
      mpq_set(numb->value.numb, const_minus_one);
      break;
   case 0 :
      mpq_set(numb->value.numb, const_zero);
      break;
   case 1 :
      mpq_set(numb->value.numb, const_one);
      break;
   default :
      abort();
   }
}

void numb_ceil(Numb* numb)
{
   mpz_t q;
   
   assert(numb_is_valid(numb));

   mpz_init(q);
   mpz_cdiv_q(q, mpq_numref(numb->value.numb), mpq_denref(numb->value.numb));
   mpq_set_z(numb->value.numb, q);
   mpz_clear(q);
}

void numb_floor(Numb* numb)
{
   mpz_t q;
   
   assert(numb_is_valid(numb));

   mpz_init(q);
   mpz_fdiv_q(q, mpq_numref(numb->value.numb), mpq_denref(numb->value.numb));
   mpq_set_z(numb->value.numb, q);
   mpz_clear(q);
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

Bool numb_is_int(const Numb* numb)
{
   /* Do we have an integer ?
    */
   if (mpz_get_si(mpq_denref(numb->value.numb)) == 1)
   {
      /* And is it small enough ?
       */
      if (mpz_fits_sint_p(mpq_numref(numb->value.numb)) == 1)
         return TRUE;
   }
   return FALSE;
}

int numb_toint(const Numb* numb)
{
   assert(numb_is_valid(numb));
   assert(numb_is_int(numb));
   
   return mpz_get_si(mpq_numref(numb->value.numb)); 
}








