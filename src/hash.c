#ident "@(#) $Id: hash.c,v 1.8 2002/07/28 07:03:32 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: hash.c                                                        */
/*   Name....: Hash Functions                                                */
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

#define HASH_BUCKETS  65521U
#define HASH_SID      0x48617368

typedef struct hash_element HElem;

struct hash_element
{
   union
   {
      const Tuple* tuple;
      const Entry* entry;
   } value;
   HElem* next;
};

struct hash
{
   SID
   size_t   size;
   int      elems;
   HashType type;
   HElem**  bucket;
};

static void hash_statist(FILE* fp, const Hash* hash);

Hash* hash_new(HashType type)
{
   Hash* hash = calloc(1, sizeof(*hash));

   assert(hash != NULL);

   hash->size   = HASH_BUCKETS;
   hash->elems  = 0;
   hash->type   = type;
   hash->bucket = calloc(hash->size, sizeof(*hash->bucket));

   assert(hash->bucket != NULL);

   SID_set(hash, HASH_SID);
   assert(hash_is_valid(hash));
   
   return hash;
}

void hash_free(Hash* hash)
{
   HElem*       he;
   HElem*       hq;
   unsigned int i;
      
   assert(hash_is_valid(hash));

   if (verbose)
      hash_statist(stderr, hash);
   
   SID_del(hash);

   for(i = 0; i < hash->size; i++)
   {
      for(he = hash->bucket[i]; he != NULL; he = hq)
      {
         hq = he->next;
         free(he);
      }
   }
   free(hash->bucket);
   free(hash);
}

Bool hash_is_valid(const Hash* hash)
{
   return ((hash != NULL)
      && ((hash->type == HASH_TUPLE) || (hash->type == HASH_ENTRY))
      && SID_ok(hash, HASH_SID));
}

void hash_add_tuple(Hash* hash, const Tuple* tuple)
{
   HElem*       he = calloc(1, sizeof(*he));
   unsigned int hcode;

   assert(hash_is_valid(hash));
   assert(tuple_is_valid(tuple));
   assert(hash->type == HASH_TUPLE);
   assert(he != NULL);
   
   hcode               = tuple_hash(tuple) % hash->size;
   he->value.tuple     = tuple;
   he->next            = hash->bucket[hcode];
   hash->bucket[hcode] = he;
   hash->elems++;
}

void hash_add_entry(Hash* hash, const Entry* entry)
{
   HElem*       he = calloc(1, sizeof(*he));
   const Tuple* tuple;
   unsigned int hcode;

   assert(hash_is_valid(hash));
   assert(entry_is_valid(entry));
   assert(hash->type == HASH_ENTRY);
   assert(he != NULL);
   
   tuple               = entry_get_tuple(entry);
   hcode               = tuple_hash(tuple) % hash->size;
   he->value.entry     = entry;
   he->next            = hash->bucket[hcode];
   hash->bucket[hcode] = he;
   hash->elems++;
}

Bool hash_has_tuple(const Hash* hash, const Tuple* tuple)
{
   unsigned int hcode = tuple_hash(tuple) % hash->size;
   HElem*       he    = NULL;
   
   assert(hash_is_valid(hash));
   assert(tuple_is_valid(tuple));

   for(he = hash->bucket[hcode]; he != NULL; he = he->next)
      if (!tuple_cmp(he->value.tuple, tuple))
         break;

   return he != NULL;
}

Bool hash_has_entry(const Hash* hash, const Tuple* tuple)
{
   unsigned int hcode = tuple_hash(tuple) % hash->size;
   HElem*       he    = NULL;
   
   assert(hash_is_valid(hash));
   assert(tuple_is_valid(tuple));
   
   for(he = hash->bucket[hcode]; he != NULL; he = he->next)
      if (!entry_cmp(he->value.entry, tuple))
         break;

   return he != NULL;
}

/* Liefert NULL wenn nicht gefunden.
 */
const Entry* hash_lookup_entry(const Hash* hash, const Tuple* tuple)
{
   unsigned int hcode = tuple_hash(tuple) % hash->size;
   HElem*       he    = NULL;
   
   assert(hash_is_valid(hash));
   assert(tuple_is_valid(tuple));

   for(he = hash->bucket[hcode]; he != NULL; he = he->next)
      if (!entry_cmp(he->value.entry, tuple))
         break;

   if (he == NULL)
      return NULL;

   assert(he != NULL);

   assert(entry_is_valid(he->value.entry));

   return he->value.entry;
}

static void hash_statist(FILE* fp, const Hash* hash)
{
   HElem* he;
   int    min    = (int)hash->size;
   int    max    = 0;
   int    sum    = 0;
   int    zeros  = 0;
   int    filled = 0;
   int    count;
   double avg    = 0.0;
   unsigned int i;

   assert(fp != NULL);
   assert(hash_is_valid(hash));

   for(i = 0; i < hash->size; i++)
   {
      count = 0;
      
      for(he = hash->bucket[i]; he != NULL; he = he->next)
         count++;

      if (count == 0)
         zeros++;
      else
         filled++;

      if (count < min)
         min = count;
      if (count > max)
         max = count;
      sum += count;
   }
   assert(sum == hash->elems);

   if (filled > 0)
      avg = (double)sum / (double)filled;
   
   fprintf(fp,
      "HashStat: size=%u sum=%d min=%d max=%d avg=%.1f zero=%d filled=%d\n",
      hash->size, sum, min, max, avg, zeros, filled);
}


