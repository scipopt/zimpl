#ident "@(#) $Id: hash.c,v 1.1 2001/01/26 07:11:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: hash.c                                                        */
/*   Name....: Hash Functions                                                */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"

#define HASH_BUCKETS  65521
#define HASH_SID      0x48617368

typedef struct hash_element HElem;

struct hash_element
{
   union
   {
      const Tuple* tuple;
      Entry*       entry;
   } value;
   HElem* next;
};

struct hash
{
   SID
   int      size;
   int      elems;
   HashType type;
   HElem**  bucket;
};

Hash* hash_new(HashType type)
{
   Hash* hash = calloc(1, sizeof(hash));

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
   assert(hash_is_valid(hash));

   SID_del(hash);
   
   free(hash->bucket);
   free(hash);
}

int hash_is_valid(const Hash* hash)
{
   return ((hash != NULL)
      && ((hash->type == HASH_TUPLE) || (hash->type == HASH_ENTRY))
      && SID_ok(hash, HASH_SID));
}

void hash_add_tuple(Hash* hash, const Tuple* tuple)
{
   HElem*       he = calloc(1, sizeof(he));
   unsigned int hcode;

   assert(hash_is_valid(hash));
   assert(tuple_is_valid(tuple));
   assert(hash->type == HASH_TUPLE);

   hcode               = tuple_hash(tuple) % hash->size;
   he->value.tuple     = tuple;
   he->next            = hash->bucket[hcode];
   hash->bucket[hcode] = he;
   hash->elems++;
}

void hash_add_entry(Hash* hash, Entry* entry)
{
   HElem*       he = calloc(1, sizeof(he));
   unsigned int hcode;

   assert(hash_is_valid(hash));
   assert(entry_is_valid(entry));
   assert(hash->type == HASH_ENTRY);

   hcode               = entry_hash(entry) % hash->size;
   he->value.entry     = entry;
   he->next            = hash->bucket[hcode];
   hash->bucket[hcode] = he;
   hash->elems++;
}

int hash_has_tuple(Hash* hash, const Tuple* tuple)
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

int hash_has_entry(Hash* hash, Entry* entry)
{
   unsigned int hcode = entry_hash(entry) % hash->size;
   Tuple*       tuple = entry_get_tuple(entry);
   HElem*       he    = NULL;
   
   assert(hash_is_valid(hash));
   assert(entry_is_valid(entry));
   
   for(he = hash->bucket[hcode]; he != NULL; he = he->next)
      if (!entry_cmp(he->value.entry, tuple))
         break;

   tuple_free(tuple);
   
   return he != NULL;
}

Entry* hash_lookup_entry(Hash* hash, Entry* entry)
{
   unsigned int hcode = entry_hash(entry) % hash->size;
   Tuple*       tuple = entry_get_tuple(entry);
   HElem*       he    = NULL;
   
   assert(hash_is_valid(hash));
   assert(entry_is_valid(entry));

   for(he = hash->bucket[hcode]; he != NULL; he = he->next)
      if (!entry_cmp(he->value.entry, tuple))
         break;

   tuple_free(tuple);
   
   return (he != NULL) ? entry_copy(he->value.entry) : NULL;
}



