#ident "@(#) $Id: idxset.c,v 1.2 2001/01/29 13:45:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: idxset.c                                                      */
/*   Name....: Index Set Functions                                           */
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

#define IDXSET_SID  0x49647853

typedef struct index_set_element ISElem;

struct index_set_element
{
   Tuple*        tuple;
   Set*          set;
   const ISElem* next;
};

struct index_set
{
   SID
   int           elements;
   int           dim;
   CodeNode*     lexpr;
   const ISElem* anchor;
};

IdxSet* idxset_new(void)
{
   IdxSet* idxset = calloc(1, sizeof(*idxset));
   
   assert(idxset != NULL);

   idxset->elements = 0;
   idxset->dim      = 0;
   idxset->lexpr    = NULL;
   idxset->anchor   = NULL;

   SID_set(idxset, IDXSET_SID);
   assert(idxset_is_valid(idxset));

   return idxset;
}

void idxset_free(IdxSet* idxset)
{
   const ISElem* p;
   const ISElem* q;

   assert(idxset_is_valid(idxset));

   SID_del(idxset);

   for(p = idxset->anchor; p != NULL; p = q)
   {
      q = p->next;

      if (p->tuple != NULL)
         tuple_free(p->tuple);
      
      set_free(p->set);

      free((void*)p);
   }
   free(idxset);
}

void idxset_add_set(IdxSet* idxset, Tuple* tuple, Set* set)
{
   ISElem* iselem = calloc(1, sizeof(*iselem));
   int     dim;
   
   assert(idxset_is_valid(idxset));
   assert(tuple_is_valid(tuple));
   assert(set_is_valid(set));
   
   assert(iselem != NULL);

   /* Bin mir noch nicht ganz im klaren, was mit sets der dimension 0
    * passieren soll.
    */
   dim = set_get_dim(set);
   
   assert(set_get_dim(set) > 0);
   
   iselem->tuple  = tuple_copy(tuple);
   iselem->set    = set_copy(set);
   iselem->next   = idxset->anchor;
   idxset->anchor = iselem;
   idxset->dim   += dim;
   idxset->elements++;
}

int idxset_is_valid(const IdxSet* idxset)
{
   return ((idxset != NULL) && SID_ok(idxset, IDXSET_SID));
}

IdxSet* idxset_copy(const IdxSet* source)
{
   IdxSet* idxset = idxset_new();
   const ISElem* iselem;
   
   assert(idxset_is_valid(idxset));
   assert(idxset_is_valid(source));

   for(iselem = source->anchor; iselem != NULL; iselem = iselem->next)
      idxset_add_set(idxset, iselem->tuple, iselem->set);

   idxset->lexpr = source->lexpr;

   assert(idxset_is_valid(idxset));

   return idxset;   
}

void idxset_set_lexpr(IdxSet* idxset, CodeNode* lexpr)
{
   assert(idxset_is_valid(idxset));
   assert(idxset->lexpr == NULL);

   idxset->lexpr = lexpr;
}

CodeNode* idxset_get_lexpr(const IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));

   return idxset->lexpr;
}

int idxset_get_sets(const IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));

   return idxset->elements;
}

int idxset_get_dim(const IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));

   return idxset->dim;
}

Tuple* idxset_get_tuple(const IdxSet* idxset, int elem_nr)
{
   const ISElem* iselem;
   int           i;
   
   assert(idxset_is_valid(idxset));
   assert(elem_nr >= 0);

   for(i = 0, iselem = idxset->anchor;
      (iselem != NULL) && (elem_nr != i);
       iselem = iselem->next, i++)
      ;
         
   assert(iselem != NULL);

   return tuple_copy(iselem->tuple);
}

Set* idxset_get_set(const IdxSet* idxset, int elem_nr)
{
   const ISElem* iselem;
   int           i;
   
   assert(idxset_is_valid(idxset));
   assert(elem_nr >= 0);
   
   for(i = 0, iselem = idxset->anchor;
      (iselem != NULL) && (elem_nr != i);
       iselem = iselem->next, i++)
      ;

   assert(iselem != NULL);
   
   return set_copy(iselem->set);
}

void idxset_print(FILE* fp, const IdxSet* idxset)
{
   const ISElem* iselem;
   
   assert(idxset_is_valid(idxset));

   fprintf(fp, "Elements: %d, Addr-Lexpr: %ld\n",
      idxset->elements,
      (long)idxset->lexpr);

   for(iselem = idxset->anchor; iselem != NULL; iselem = iselem->next)
   {
      fprintf(fp, "Tuple: ");
      if (iselem->tuple != NULL)
         tuple_print(fp, iselem->tuple);
      else
         fprintf(fp, "N/A ");

      set_print(fp, iselem->set);
   }
}

