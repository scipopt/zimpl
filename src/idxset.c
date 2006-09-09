#pragma ident "@(#) $Id: idxset.c,v 1.10 2006/09/09 10:00:21 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: idxset.c                                                      */
/*   Name....: IndexSet Functions                                            */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2006 by Thorsten Koch <koch@zib.de>
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

#define IDXSET_SID  0x49647853

struct index_set
{
   SID
   Tuple*        tuple;
   Set*          set;
   CodeNode*     lexpr;
   Bool          is_unrestricted;
};

IdxSet* idxset_new(
   const Tuple* tuple,
   const Set*   set,
   CodeNode*    lexpr,
   Bool         is_unrestricted)
{
   IdxSet* idxset = calloc(1, sizeof(*idxset));
   
   assert(tuple_is_valid(tuple));
   assert(set_is_valid(set));
   assert(lexpr  != NULL);
   assert(idxset != NULL);

   idxset->tuple           = tuple_copy(tuple);
   idxset->set             = set_copy(set);
   idxset->lexpr           = lexpr;
   idxset->is_unrestricted = is_unrestricted;
   
   SID_set(idxset, IDXSET_SID);
   assert(idxset_is_valid(idxset));

   return idxset;
}

void idxset_free(IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));

   SID_del(idxset);

   tuple_free(idxset->tuple);
   set_free(idxset->set);
   free(idxset);
}

Bool idxset_is_valid(const IdxSet* idxset)
{
   return ((idxset != NULL) && SID_ok(idxset, IDXSET_SID));
}

IdxSet* idxset_copy(const IdxSet* source)
{
   assert(idxset_is_valid(source));

   return idxset_new(source->tuple, source->set, source->lexpr, source->is_unrestricted);   
}

CodeNode* idxset_get_lexpr(const IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));

   return idxset->lexpr;
}

const Tuple* idxset_get_tuple(const IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));

   return idxset->tuple;
}

const Set* idxset_get_set(const IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));
   
   return idxset->set;
}

Bool idxset_is_unrestricted(const IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));
   
   return idxset->is_unrestricted;
}

void idxset_print(FILE* fp, const IdxSet* idxset)
{
   assert(idxset_is_valid(idxset));

   fprintf(fp, "IdxSet\n");
   fprintf(fp, "Tuple: ");
   tuple_print(fp, idxset->tuple);
   fputc('\n', fp);
   set_print(fp, idxset->set);
   fprintf(fp, "\nAddr-Lexpr: %ld\n", (long)idxset->lexpr);
}




