#pragma ident "@(#) $Id: numbgmp.c,v 1.24 2006/06/16 08:24:56 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: numbgmp.c                                                     */
/*   Name....: Number Functions using gmp                                    */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001,2006 by Thorsten Koch <koch@zib.de>
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
#include <math.h>
#include <ctype.h>
#include <errno.h>
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

static void          genrand_init(unsigned long s);
static unsigned long genrand_int32(void);

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

void numb_init(unsigned long seed)
{
   store_anchor = NULL;
   store_free   = NULL;

   gmp_init(verbose >= VERB_VERBOSE);

   numb_const_zero     = numb_new();
   numb_const_one      = numb_new_integer(1);
   numb_const_minusone = numb_new_integer(-1);

   genrand_init(seed);
}

void numb_exit()
{
   NumbStore* store;
   NumbStore* next;

   numb_free(numb_const_zero);
   numb_free(numb_const_one);
   numb_free(numb_const_minusone);

   numb_const_zero     = NULL;
   numb_const_one      = NULL;
   numb_const_minusone = NULL;

   if (store_count != 0)
      printf("Numb store count %d\n", store_count);
   
   for(store = store_anchor; store != NULL; store = next)
   {
      next = store->next;

      /* ??? mpq_clear() is not called for the used ones.
       * This would be faster then doing it in numb_free.
       */
      free(store->begin);
      free(store);
   }   
   store_anchor = NULL;
   store_free   = NULL;
   store_count  = 0;

   gmp_exit();
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

Numb* numb_new_mpq(const mpq_t val)
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);

   mpq_set(numb->value.numb, val); 
   
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
   mpz_t a;
   mpz_t b;
   mpz_t r;

   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpz_init(a);
   mpz_init(b);
   mpz_init(r);

   mpz_mul(a, mpq_numref(numb_a->value.numb), mpq_denref(numb_b->value.numb));
   mpz_mul(b, mpq_numref(numb_b->value.numb), mpq_denref(numb_a->value.numb));
   mpz_mod(r, a, b);
   mpq_set_z(numb_a->value.numb, r);

   mpz_clear(r);
   mpz_clear(b);
   mpz_clear(a);
}

Numb* numb_new_mod(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();
   mpz_t a;
   mpz_t b;
   mpz_t r;
   

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   mpz_init(a);
   mpz_init(b);
   mpz_init(r);

   mpz_mul(a, mpq_numref(numb_a->value.numb), mpq_denref(numb_b->value.numb));
   mpz_mul(b, mpq_numref(numb_b->value.numb), mpq_denref(numb_a->value.numb));
   mpz_mod(r, a, b);
   mpq_set_z(numb->value.numb, r);

   mpz_clear(r);
   mpz_clear(b);
   mpz_clear(a);

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

   /*lint -e(634) Strong type mismatch (type 'Bool') in equality or conditional */
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

int numb_get_sgn(const Numb* numb)
{
   assert(numb_is_valid(numb));

   /*lint -e(634) Strong type mismatch (type 'Bool') in equality or conditional */
   return mpq_sgn(numb->value.numb);
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

Numb* numb_new_log(const Numb* numb)
{
   char   temp[256];
   double d;
   
   assert(numb_is_valid(numb));

   d = log10(mpq_get_d(numb->value.numb));

   if (!finite(d))
   {
      sprintf(temp, "*** Error 700: log(%f)", mpq_get_d(numb->value.numb));
      perror(temp);
      return NULL;
   }
   sprintf(temp, "%.16e", d);

   return numb_new_ascii(temp);
}

Numb* numb_new_sqrt(const Numb* numb)
{
   char   temp[256];
   double d;
   
   assert(numb_is_valid(numb));

   d = sqrt(mpq_get_d(numb->value.numb));

   if (!finite(d))
   {
      sprintf(temp, "*** Error 701: sqrt(%f)", mpq_get_d(numb->value.numb));
      perror(temp);
      return NULL;
   }
   sprintf(temp, "%.16e", d);

   return numb_new_ascii(temp);
}

Numb* numb_new_exp(const Numb* numb)
{
   char temp[32];
   
   assert(numb_is_valid(numb));

   sprintf(temp, "%.16e", exp(mpq_get_d(numb->value.numb)));

   return numb_new_ascii(temp);
}

Numb* numb_new_ln(const Numb* numb)
{
   char   temp[256];
   double d;
   
   assert(numb_is_valid(numb));

   d = log(mpq_get_d(numb->value.numb));

   if (!finite(d))
   {
      sprintf(temp, "*** Error 702: ln(%f)", mpq_get_d(numb->value.numb));
      perror(temp);
      return NULL;
   }
   sprintf(temp, "%.16e", d);

   return numb_new_ascii(temp);
}

Numb* numb_new_rand(const Numb* mini, const Numb* maxi)
{
   Numb* numb = numb_new();
   mpq_t maxint;
   mpq_t factor;

   assert(numb != NULL);
   assert(numb_is_valid(mini));
   assert(numb_is_valid(maxi));
   assert(numb_cmp(mini, maxi) <= 0);
   
   mpq_init(factor);
   mpq_init(maxint);
   
   mpq_set_ui(numb->value.numb, genrand_int32(), 1); 
   mpq_set_ui(maxint, 4294967295UL, 1);
   
   mpq_div(numb->value.numb, numb->value.numb, maxint);
   mpq_sub(factor, maxi->value.numb, mini->value.numb);
   mpq_mul(numb->value.numb, numb->value.numb, factor);
   mpq_add(numb->value.numb, numb->value.numb, mini->value.numb);
      
   mpq_clear(factor);
   mpq_clear(maxint);

   return numb;
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

Bool numb_is_number(const char *s)
{
   /* 5 !*/
   if (isdigit(*s))
      return TRUE;

   /* maybe -5 or .6 or -.7 ? */
   if (*s != '+' && *s != '-' && *s != '.')
      return FALSE;

   if (*s == '\0')
      return FALSE;

   s++;

   /* -5 or .6 ! */
   if (isdigit(*s))
      return TRUE;

   /* maybe -.7 ? */
   if (*s != '.')
      return FALSE;
   
   if (*s == '\0')
      return FALSE;

   s++;
   
   return isdigit(*s);
}

/* 
   A C-program for MT19937, with initialization improved 2002/2/10.
   Coded by Takuji Nishimura and Makoto Matsumoto.
   This is a faster version by taking Shawn Cokus's optimization,
   Matthe Bellew's simplification, Isaku Wada's real version.

   Before using, initialize the state by using init_genrand(seed) .

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

/* Period parameters */  
#define N            624
#define M            397
#define MATRIX_A     0x9908b0dfUL   /* constant vector a */
#define UMASK        0x80000000UL /* most significant w-r bits */
#define LMASK        0x7fffffffUL /* least significant r bits */
#define MIXBITS(u,v) (((u) & UMASK) | ((v) & LMASK))
#define TWIST(u,v)   ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

static unsigned long  state[N]; /* the array for the state vector  */
static int            left  = 1;
static unsigned long* next;

/* initializes state[N] with a seed */
static void genrand_init(unsigned long s)
{
   int j;
    
   state[0] = s & 0xffffffffUL;

   for(j = 1; j < N; j++)
   {
      state[j] = (1812433253UL * (state[j-1] ^ (state[j-1] >> 30)) + j); 

      /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
      /* In the previous versions, MSBs of the seed affect   */
      /* only MSBs of the array state[].                     */
      /* 2002/01/09 modified by Makoto Matsumoto             */
      state[j] &= 0xffffffffUL;  /* for >32 bit machines */
   }
   left = 1;
}

/* generates a random number on [0,0xffffffff]-interval */
static unsigned long genrand_int32(void)
{
   unsigned long y;

   if (--left == 0)
   {
      unsigned long* p = state;
      int            j;

      left = N;
      next = state;
    
      for(j = N - M + 1; --j; p++) 
         *p = p[M] ^ TWIST(p[0], p[1]);

      for(j = M; --j; p++) 
         *p = p[M-N] ^ TWIST(p[0], p[1]);

      *p = p[M-N] ^ TWIST(p[0], state[0]);
   }
   y = *next++;

   /* Tempering */
   y ^= (y >> 11);
   y ^= (y << 7) & 0x9d2c5680UL;
   y ^= (y << 15) & 0xefc60000UL;
   y ^= (y >> 18);

   return y;
}

/* ----------------------------------------------------------------------------- */




