#ident "@(#) $Id: entry.c,v 1.1 2001/01/26 07:11:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: entry.c                                                       */
/*   Name....: Symbol Table Entry Functions                                  */
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

#define ENTRY_SID  0x456e7472

typedef union entry_value EntryValue;

union entry_value
{
   double      numb;
   const char* strg;
   Set*        set;
   Var*        var;
};

struct entry
{
   SID
   int        refc;
   Tuple*     tuple;
   SymbolType type;
   EntryValue value;
};

Entry* entry_new_numb(Tuple* tuple, double numb)
{
   Entry* entry = calloc(1, sizeof(*entry));

   assert(entry != NULL);
   assert(tuple != NULL);

   entry->refc       = 1;
   entry->tuple      = tuple_copy(tuple);
   entry->type       = SYM_NUMB;
   entry->value.numb = numb;

   SID_set(entry, ENTRY_SID);
   assert(entry_is_valid(entry));
   
   return entry;
}

Entry* entry_new_strg(Tuple* tuple, const char* strg)
{
   Entry* entry = calloc(1, sizeof(*entry));

   assert(entry != NULL);
   assert(tuple != NULL);
   assert(strg  != NULL);
   
   entry->refc       = 1;
   entry->tuple      = tuple_copy(tuple);
   entry->type       = SYM_STRG;
   entry->value.strg = strg;

   SID_set(entry, ENTRY_SID);
   assert(entry_is_valid(entry));

   return entry;
}

Entry* entry_new_set(Tuple* tuple, Set* set)
{
   Entry* entry = calloc(1, sizeof(*entry));

   assert(entry != NULL);
   assert(tuple != NULL);
   assert(set   != NULL);
   
   entry->refc      = 1;
   entry->tuple     = tuple_copy(tuple);
   entry->type      = SYM_SET;
   entry->value.set = set_copy(set);

   SID_set(entry, ENTRY_SID);
   assert(entry_is_valid(entry));

   return entry;
}

Entry* entry_new_var(Tuple* tuple, Var* var)
{
   Entry* entry = calloc(1, sizeof(*entry));

   assert(entry != NULL);
   assert(tuple != NULL);
   assert(var   != NULL);
   
   entry->refc      = 1;
   entry->tuple     = tuple_copy(tuple);
   entry->type      = SYM_VAR;
   entry->value.var = var_copy(var);

   SID_set(entry, ENTRY_SID);
   assert(entry_is_valid(entry));

   return entry;
}

void entry_free(Entry* entry)
{
   assert(entry_is_valid(entry));

   entry->refc--;

   if (entry->refc == 0)
   {
      SID_del(entry);

      switch(entry->type)
      {
      case SYM_NUMB :
      case SYM_STRG :
         break;
      case SYM_SET :
         set_free(entry->value.set);
         break;
      case SYM_VAR :
         var_free(entry->value.var);
         break;
      default :
         assert(0);
      }
      tuple_free(entry->tuple);
   
      free(entry);
   }
}

int entry_is_valid(const Entry* entry)
{
   return ((entry != NULL) && SID_ok(entry, ENTRY_SID));
}

Entry* entry_copy(Entry* entry)
{
   assert(entry_is_valid(entry));

   entry->refc++;

   return entry;
}

int entry_cmp(const Entry* entry, const Tuple* tuple)
{
   assert(entry_is_valid(entry));
   assert(tuple_is_valid(tuple));
   
   return tuple_cmp(entry->tuple, tuple);
}

SymbolType entry_get_type(const Entry* entry)
{
   assert(entry_is_valid(entry));

   return entry->type;
}

Tuple* entry_get_tuple(const Entry* entry)
{
   assert(entry_is_valid(entry));
   assert(tuple_is_valid(entry->tuple));
   
   return tuple_copy(entry->tuple);
}

double entry_get_numb(const Entry* entry)
{
   assert(entry_is_valid(entry));
   assert(entry->type == SYM_NUMB);
   
   return entry->value.numb;
}

const char* entry_get_strg(const Entry* entry)
{
   assert(entry_is_valid(entry));
   assert(entry->type == SYM_STRG);
   
   return entry->value.strg;
}

Set* entry_get_set(Entry* entry)
{
   assert(entry_is_valid(entry));
   assert(entry->type == SYM_SET);
   
   return set_copy(entry->value.set);
}

Var* entry_get_var(Entry* entry)
{
   assert(entry_is_valid(entry));
   assert(entry->type == SYM_VAR);
   
   return var_copy(entry->value.var);
}

void entry_print(FILE* fp, const Entry* entry)
{
   assert(entry_is_valid(entry));

   tuple_print(fp, entry->tuple);
   fprintf(fp, " -> ");
   
   switch(entry->type)
   {
   case SYM_NUMB :
      fprintf(fp, "%g", entry->value.numb);
      break;
   case SYM_STRG :
      fprintf(fp, "\"%s\"", entry->value.strg);
      break;
   case SYM_SET :
      set_print(fp, entry->value.set);
      break;
   case SYM_VAR :
      var_print(fp, entry->value.var);
      break;
   default :
      fprintf(fp, "Entry-ERR");
      break;
   }
}

unsigned int entry_hash(const Entry* entry)
{
   return tuple_hash(entry->tuple);
}
