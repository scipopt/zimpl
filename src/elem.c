#ident "@(#) $Id: elem.c,v 1.3 2001/01/30 08:23:46 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: elem.c                                                        */
/*   Name....: Element Functions                                             */
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

#define STORE_SIZE  1000
#define ELEM_SID    0x456c656d

typedef union element_value    ElemValue;
typedef struct element_storage ElemStore;

union element_value
{
   double      numb;
   const char* strg;
   const char* name;
};

struct element
{
   SID
   ElemType  type;
   ElemValue value;
};

struct element_storage
{
   int         size;
   int         used;
   Elem*       begin;
   ElemStore*  next;
};

static ElemStore* store_anchor = NULL;

static void extend_storage(void)
{
   ElemStore* store;
         
   store = calloc(1, sizeof(*store));
   
   assert(store != NULL);
   
   store->size  = STORE_SIZE;
   store->used  = 0;
   store->begin = calloc(STORE_SIZE, sizeof(*store->begin));
   store->next  = store_anchor;
   store_anchor = store;
   
   assert(store->begin != NULL);
}

static Elem* new_elem(void)
{
   assert(store_anchor != NULL);
   
   if (store_anchor->size == store_anchor->used)
      extend_storage();

   assert(store_anchor->size > store_anchor->used);

   return &store_anchor->begin[store_anchor->used++];
}

void elem_init(void)
{
   extend_storage();
}

void elem_exit(void)
{
   ElemStore* store;
   ElemStore* next;
   int        i;
   
   for(store = store_anchor; store != NULL; store = next)
   {
      next = store->next;

      for(i = 0; i < store->used; i++)
      {
         assert(elem_is_valid(&store->begin[i]));
         SID_del(((Elem*)&store->begin[i]));
      }
      free(store->begin);
      free(store);
   }
}

const Elem* elem_new_numb(double numb)
{
   Elem* elem = new_elem();
   
   assert(elem != NULL);

   elem->type       = ELEM_NUMB;
   elem->value.numb = numb;
   
   SID_set(elem, ELEM_SID);
   assert(elem_is_valid(elem));
   
   return elem;
}

const Elem* elem_new_strg(const char* strg)
{
   Elem* elem = new_elem();

   assert(strg != NULL);
   assert(elem != NULL);

   elem->type       = ELEM_STRG;
   elem->value.strg = strg;

   SID_set(elem, ELEM_SID);
   assert(elem_is_valid(elem));

   return elem;
}

const Elem* elem_new_name(const char* name)
{
   Elem* elem = new_elem();

   assert(name != NULL);
   assert(elem != NULL);

   elem->type       = ELEM_NAME;
   elem->value.strg = name;

   SID_set(elem, ELEM_SID);
   assert(elem_is_valid(elem));

   return elem;
}

Bool elem_is_valid(const Elem* elem)
{
   return ((elem != NULL) && SID_ok(elem, ELEM_SID));
}

/* 0 wenn gleich, sonst != 0
 */
Bool elem_cmp(const Elem* elem_a, const Elem* elem_b)
{
   assert(elem_is_valid(elem_a));
   assert(elem_is_valid(elem_b));
   assert(elem_a->type != ELEM_ERR);
   assert(elem_b->type != ELEM_ERR);

   /* Auf die schnelle vorweg.
    */
   if (elem_a == elem_b)
      return FALSE;
   
   /* Wenn einer ein Name ist, sind sie gleich.
    */
   if ((elem_a->type == ELEM_NAME) || (elem_b->type == ELEM_NAME))
      return FALSE;

   if (elem_a->type != elem_b->type)
   {
      fprintf(stderr, "Warning: Comparison of different type elements\n");
      return TRUE;
   }
   assert(elem_a->type == elem_b->type);
   
   if (elem_a->type == ELEM_STRG)
      return strcmp(elem_a->value.strg, elem_b->value.strg) != 0;

   assert(elem_a->type == ELEM_NUMB);

   return NE(elem_a->value.numb, elem_b->value.numb);
}

ElemType elem_get_type(const Elem* elem)
{
   assert(elem_is_valid(elem));
   
   return elem->type;
}

double elem_get_numb(const Elem* elem)
{
   assert(elem_is_valid(elem));
   assert(elem->type == ELEM_NUMB);
   
   return elem->value.numb;
}

const char* elem_get_strg(const Elem* elem)
{
   assert(elem_is_valid(elem));
   assert(elem->type       == ELEM_STRG);
   assert(elem->value.strg != NULL);
   
   return elem->value.strg;
}

const char* elem_get_name(const Elem* elem)
{
   assert(elem_is_valid(elem));
   assert(elem->type       == ELEM_NAME);
   assert(elem->value.name != NULL);
   
   return elem->value.name;
}

void elem_print(FILE* fp, const Elem* elem)
{
   assert(elem_is_valid(elem));

   switch(elem->type)
   {
   case ELEM_NUMB :
      fprintf(fp, "%.16g", elem->value.numb);
      break;
   case ELEM_STRG :
      fprintf(fp, "\"%s\"", elem->value.strg);
      break;
   case ELEM_NAME :
      fprintf(fp, "%s", elem->value.name);
      break;
   default :
      fprintf(fp, "Elem-ERR");
      break;
   }
}

unsigned int elem_hash(const Elem* elem)
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
   
   switch(elem->type)
   {
   case ELEM_NUMB :
      d2i.d = elem->value.numb;
      hcode = d2i.i.a ^ d2i.i.b;
      break;
   case ELEM_STRG :
      hcode = str_hash(elem->value.strg);
      break;
   case ELEM_NAME :
      hcode = str_hash(elem->value.name);
      break;
   default :
      assert(0);
      abort();
   }
   return hcode;
}
