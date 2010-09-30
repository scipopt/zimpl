/* $Id: numbdbl.c,v 1.8 2010/09/30 09:27:36 bzfkocht Exp $ */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: numbdbl.c                                                     */
/*   Name....: Number Functions using double                                 */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2010 by Thorsten Koch <koch@zib.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
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

/* #define TRACE 1 */

#include "bool.h"
#include "lint.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "numb.h"
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
      double numb;
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

void numb_init(Bool with_management)
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

   numb_const_zero     = NULL;
   numb_const_one      = NULL;
   numb_const_minusone = NULL;

   if (store_count != 0)
      printf("Numb store count %d\n", store_count);
   
   for(store = store_anchor; store != NULL; store = next)
   {
      next = store->next;

      free(store->begin);
      free(store);
   }   
   store_anchor = NULL;
   store_free   = NULL;
   store_count  = 0;
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

   numb->value.numb = 0.0;

   return numb;
}

Numb* numb_new_ascii(const char* val)
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);

   numb->value.numb = atof(val);
   
   return numb;
}

Numb* numb_new_integer(int val)
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);

   numb->value.numb = val;
   
   return numb;
}

void numb_free(Numb* numb)
{
   Trace("numb_free");

   assert(numb_is_valid(numb));

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

   numb->value.numb = source->value.numb;

   return numb;
}

/* TRUE wenn gleich, sonst FALSE
 */
Bool numb_equal(const Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   return numb_a->value.numb == numb_b->value.numb;
}

/* Return a positive value if op1 > op2, zero if op1 = op2, and a negative value if op1 < op2
 */
int numb_cmp(const Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   if (numb_a->value.numb > numb_b->value.numb)
      return 1;
   if (numb_a->value.numb < numb_b->value.numb)
      return -1;
   return 0;
}

void numb_set(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb_a->value.numb = numb_b->value.numb;
}

void numb_add(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb_a->value.numb += numb_b->value.numb;
}

Numb* numb_new_add(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb->value.numb = numb_a->value.numb + numb_b->value.numb;

   return numb;
}

void numb_sub(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb_a->value.numb -= numb_b->value.numb;
}

Numb* numb_new_sub(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb->value.numb = numb_a->value.numb - numb_b->value.numb;

   return numb;
}

void numb_mul(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb_a->value.numb *= numb_b->value.numb;
}

Numb* numb_new_mul(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb->value.numb = numb_a->value.numb * numb_b->value.numb;

   return numb;
}

void numb_div(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb_a->value.numb /= numb_b->value.numb;
}

Numb* numb_new_div(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb->value.numb = numb_a->value.numb / numb_b->value.numb;

   return numb;
}

void numb_intdiv(Numb* numb_a, const Numb* numb_b)
{
   double q;

   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb_a->value.numb = trunc(numb_a->value.numb / numb_b->value.numb);
}

Numb* numb_new_intdiv(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb->value.numb = trunc(numb_a->value.numb / numb_b->value.numb);

   return numb;
}

void numb_mod(Numb* numb_a, const Numb* numb_b)
{
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb_a->value.numb = fmod(numb_a->value.numb, numb_b->value.numb);
}

Numb* numb_new_mod(const Numb* numb_a, const Numb* numb_b)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));
   assert(numb_is_valid(numb_b));

   numb->value.numb = fmod(numb_a->value.numb, numb_b->value.numb);

   return numb;
}

Numb* numb_new_pow(const Numb* base, int expo)
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);
   assert(numb_is_valid(base));

   numb->value.numb = pow(base->value.numb, expo);
   
   return numb;
}

Numb* numb_new_fac(int n)
{
   Numb* numb = numb_new();
   int   i;

   assert(numb != NULL);
   assert(n    >= 0);

   numb->value.numb = 1;
   
   for(i = 2; i <= n; i++)
      numb->value.numb *= i;
   
   return numb;
}

void numb_neg(Numb* numb)
{
   assert(numb_is_valid(numb));

   numb->value.numb *= -1;
}

void numb_abs(Numb* numb)
{
   assert(numb_is_valid(numb));

   numb->value.numb = fabs(numb->value.numb);
}

void numb_sgn(Numb* numb)
{
   assert(numb_is_valid(numb));

   if (numb->value.numb < 0)
      numb->value.numb = -1;
   else if (numb->value.numb > 0)
      numb->value.numb =  1;
   else
      numb->value.numb =  0;
}

int numb_get_sgn(const Numb* numb)
{
   assert(numb_is_valid(numb));

   if (numb->value.numb < 0)
      return -1;

   if (numb->value.numb > 0)
      return  1;

   return 0;
}

void numb_round(Numb* numb)
{
   assert(numb_is_valid(numb));

   numb->value.numb = (double)(int)(numb->value.numb + 0.5);
}

void numb_ceil(Numb* numb)
{
   assert(numb_is_valid(numb));

   numb->value.numb = ceil(numb->value.numb);
}

void numb_floor(Numb* numb)
{
   assert(numb_is_valid(numb));

   numb->value.numb = floor(numb->value.numb);
}

Numb* numb_new_log(const Numb* numb_a)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(numb_a));

   numb->value.numb = log10(numb_a->value.numb);

   if (!finite(numb->value.numb))
   {
      char temp[256];

      sprintf(temp, "*** Error 700: log(%f)", numb_a->value.numb);
      perror(temp);
      return NULL;
   }
   return numb;
}

Numb* numb_new_sqrt(const Numb* numb_a)
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);
   assert(numb_is_valid(numb_a));

   numb->value.numb = sqrt(numb_a->value.numb);

   if (!finite(numb->value.numb))
   {
      char temp[256];

      sprintf(temp, "*** Error 701: sqrt(%f)", numb_a->value.numb);
      perror(temp);
      return NULL;
   }
   return numb;
}

Numb* numb_new_exp(const Numb* numb_a)
{
   char temp[32];
   Numb* numb = numb_new();
   
   assert(numb != NULL);
   assert(numb_is_valid(numb_a));

   numb->value.numb = exp(numb_a->value.numb);

   return numb;
}

Numb* numb_new_ln(const Numb* numb_a)
{
   Numb* numb = numb_new();
   
   assert(numb != NULL);
   assert(numb_is_valid(numb_a));

   numb->value.numb = log(numb_a->value.numb);

   if (!finite(numb->value.numb))
   {
      char temp[256];
      
      sprintf(temp, "*** Error 702: ln(%f)", numb->value.numb);
      perror(temp);
      return NULL;
   }
   return numb;
}

Numb* numb_new_rand(const Numb* mini, const Numb* maxi)
{
   Numb* numb = numb_new();

   assert(numb != NULL);
   assert(numb_is_valid(mini));
   assert(numb_is_valid(maxi));
   assert(numb_cmp(mini, maxi) <= 0);
   
   numb->value.numb  = genrand_int32();
   numb->value.numb /= 4294967295.0; /* MAXINT */
   numb->value.numb *= maxi->value.numb - mini->value.numb;
   numb->value.numb += mini->value.numb;
      
   return numb;
}

double numb_todbl(const Numb* numb)
{
   assert(numb_is_valid(numb));
   
   return numb->value.numb;
}

void numb_print(FILE* fp, const Numb* numb)
{
   assert(numb_is_valid(numb));

   fprintf(fp, "%.16g", numb->value.numb);
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
   
   d2i.d = numb->value.numb;
   hcode = d2i.i.a ^ d2i.i.b;

   return hcode;
}

char* numb_tostr(const Numb* numb)
{
   char* str;
   
   assert(numb_is_valid(numb));

   str = malloc(32);
      
   assert(str != NULL);
      
   sprintf(str, "%.16g", numb->value.numb);

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
   return numb->value.numb == (double)((int)numb->value.numb);
}

int numb_toint(const Numb* numb)
{
   assert(numb_is_valid(numb));
   assert(numb_is_int(numb));
   
   return (int)numb->value.numb; 
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




