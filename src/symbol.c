#ident "@(#) $Id: symbol.c,v 1.6 2001/01/30 08:23:46 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: symbol.c                                                      */
/*   Name....: Symbol Table Functions                                        */
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

#define TEST_DUBLICATE   0

#define SYMBOL_SID  0x53796d62
#define SYMBOL_EXTEND_SIZE 100

struct symbol
{
   SID
   const char*  name;
   int          size;
   int          used;
   int          extend;
   SymbolType   type;
   Set*         set;
   Hash*        hash;
   Entry**      entry;
   Symbol*      next;
};

static int    symbols = 0;
#ifndef NDEBUG
static Symbol anchor  = { 0, "", 0, 0, 0, SYM_ERR, NULL, NULL, NULL, NULL };
#else
static Symbol anchor  = { "", 0, 0, 0, SYM_ERR, NULL, NULL, NULL, NULL };
#endif

Symbol* symbol_new(const char* name, SymbolType type, Set* set)
{
   Symbol* sym;

   assert(name         != NULL);
   assert(strlen(name) >  0);
   assert(set          != NULL);
   assert((type > SYM_ERR) && (type <= SYM_VAR));

   sym = calloc(1, sizeof(*sym));

   assert(sym != NULL);
   
   sym->name   = name;
   sym->type   = type;
   sym->size   = 1;
   sym->used   = 0;
   sym->extend = SYMBOL_EXTEND_SIZE;
   sym->set    = set_copy(set);
   sym->hash   = hash_new(HASH_ENTRY);
   sym->entry  = calloc(1, sizeof(*sym->entry));
   sym->next   = anchor.next;
   anchor.next = sym;
   symbols++;

   assert(sym->entry != NULL);

   SID_set(sym, SYMBOL_SID);
   assert(symbol_is_valid(sym));

   return sym;
}

void symbol_exit(void)
{
   Symbol* q;
   Symbol* p;
   int     i;
   
   for(p = anchor.next; p != NULL; p = q)
   {
      assert(symbol_is_valid(p));

      SID_del(p);

      q = p->next;
      
      for(i = 0; i < p->used; i++)
         entry_free(p->entry[i]);

      free(p->entry);
      set_free(p->set);
      hash_free(p->hash);
      free(p);
   }
   anchor.next = NULL;
   symbols     = 0;
}

Bool symbol_is_valid(const Symbol* symbol)
{
   return ((symbol != NULL) && SID_ok(symbol, SYMBOL_SID));
}

Symbol* symbol_lookup(const char* name)
{
   Symbol* sym;

   assert(name != NULL);

   for(sym = anchor.next; sym != NULL; sym = sym->next)
      if (!strcmp(sym->name, name))
         break;

   return sym;
}

Bool symbol_has_entry(const Symbol* sym, Tuple* tuple)
{
   assert(symbol_is_valid(sym));
   assert(tuple_is_valid(tuple));

   return hash_has_entry(sym->hash, tuple);
}

Entry* symbol_lookup_entry(const Symbol* sym, Tuple* tuple)
{
   assert(symbol_is_valid(sym));
   assert(tuple_is_valid(tuple));

   return hash_lookup_entry(sym->hash, tuple);
}

/* Entry wird gefressen.
 */
void symbol_add_entry(Symbol* sym, Entry* entry)
{
   Tuple* tuple;
   
   assert(symbol_is_valid(sym));
   assert(entry_is_valid(entry));
   
   assert(sym->used <= sym->size);
   
   if (sym->used == sym->size)
   {
      sym->size   += sym->extend;
      sym->extend += sym->extend;
      sym->entry   = realloc(
         sym->entry, (size_t)sym->size * sizeof(*sym->entry));
      
      assert(sym->entry != NULL);
   }
   assert(sym->used < sym->size);

   tuple = entry_get_tuple(entry);

   if (hash_has_entry(sym->hash, tuple))
      fprintf(stderr, "Dublicate entry\n");
   else
   {
      /* Falls noch nicht geschehen, legen wir hier den Typ des
       * Symbols fest.
       */
      if ((sym->type == SYM_ERR) && (sym->used == 0))
         sym->type = entry_get_type(entry);

      entry_set_index(entry, sym->used);      
      hash_add_entry(sym->hash, entry);
      
      sym->entry[sym->used] = entry_copy(entry);      
      sym->used++;
   }
   tuple_free(tuple);
}

int symbol_get_dim(const Symbol* sym)
{
   assert(symbol_is_valid(sym));

   return set_get_dim(sym->set);
}

const Set* symbol_get_iset(const Symbol* sym)
{
   assert(symbol_is_valid(sym));

   return sym->set;
}

const char* symbol_get_name(const Symbol* sym)
{
   assert(symbol_is_valid(sym));

   return sym->name;
}

SymbolType symbol_get_type(const Symbol* sym)
{
   assert(symbol_is_valid(sym));

   return sym->type;
}

double symbol_get_numb(const Symbol* sym, int idx)
{
   assert(symbol_is_valid(sym));
   assert(idx >= 0);
   assert(idx <  sym->used);
   
   return entry_get_numb(sym->entry[idx]);
}

const char* symbol_get_strg(const Symbol* sym, int idx)
{
   assert(symbol_is_valid(sym));
   assert(idx >= 0);
   assert(idx <  sym->used);
   
   return entry_get_strg(sym->entry[idx]);
}

Set* symbol_get_set(Symbol* sym, int idx)
{
   assert(symbol_is_valid(sym));
   assert(idx >= 0);
   assert(idx <  sym->used);
   
   return entry_get_set(sym->entry[idx]);
}

Var* symbol_get_var(Symbol* sym, int idx)
{
   assert(symbol_is_valid(sym));
   assert(idx >= 0);
   assert(idx <  sym->used);
   
   return entry_get_var(sym->entry[idx]);
}

void symbol_print(FILE* fp, const Symbol* sym)
{
   static const char* type_name[] = { "Error", "Numb", "Strg", "Set", "Var" };
   
   int i;
   
   assert(symbol_is_valid(sym));

   fprintf(fp, "Name  : %s\n", sym->name);
   fprintf(fp, "Type  : %s\n", type_name[sym->type]);

   fprintf(fp, "Index : ");
   set_print(fp, sym->set);
   fprintf(fp, "Entries:\n");
   
   for(i = 0; i < sym->used; i++)
   {
      fprintf(fp, "\t%3d: ", i);
      entry_print(fp, sym->entry[i]);
      fprintf(fp, "\n");
   }
   fprintf(fp, "\n");
}

void symbol_print_all(FILE* fp)
{
   Symbol* sym;
   
   assert(fp != NULL);

   for(sym = anchor.next; sym != NULL; sym = sym->next)
      symbol_print(fp, sym);
}

void symbol_print_bounds(FILE* fp)
{
   Symbol* sym;
   Var*    var;
   int     i;
   Bool    flag;
   Bool    first;
   
   assert(fp != NULL);
   
   fprintf(fp, "Bounds\n");
   
   for(sym = anchor.next; sym != NULL; sym = sym->next)
   {
      if (sym->type != SYM_VAR)
         continue;
      
      for(i = 0; i < sym->used; i++)
      {
         var = entry_get_var(sym->entry[i]);
         
         fprintf(fp, " %.16g <= ", var_get_lower(var));
         fprintf(fp, "%s", sym->name);
         fprintf(fp, "_%d", entry_get_index(sym->entry[i]));      
         fprintf(fp, " <= %.16g\n", var_get_upper(var));
         
         var_free(var);
      }
   }
   first = TRUE;
   
   for(sym = anchor.next; sym != NULL; sym = sym->next)
   {
      if (sym->type != SYM_VAR)
         continue;

      flag = FALSE;
      
      for(i = 0; i < sym->used; i++)
      {
         var = entry_get_var(sym->entry[i]);

         if (var_get_type(var) == VAR_INT)
         {
            if (first)
            {
               fprintf(fp, "Integer\n");
               first = FALSE;
            }
            fprintf(fp, " %s", sym->name);
            fprintf(fp, "_%d", entry_get_index(sym->entry[i]));      

            flag = TRUE;

            if (i % 8 == 7)
               fputc('\n', fp);
         }
         var_free(var);
      }
      if (flag)
         fputc('\n', fp);
   }
   first = TRUE;
   
   for(sym = anchor.next; sym != NULL; sym = sym->next)
   {
      if (sym->type != SYM_VAR)
         continue;

      flag = FALSE;
      
      for(i = 0; i < sym->used; i++)
      {
         var = entry_get_var(sym->entry[i]);

         if (var_get_type(var) == VAR_BIN)
         {
            if (first)
            {
               fprintf(fp, "Binary\n");
               first = FALSE;
            }
            fprintf(fp, " %s", sym->name);
            fprintf(fp, "_%d", entry_get_index(sym->entry[i]));      

            flag = TRUE;
            
            if (i % 8 == 7)
               fputc('\n', fp);
         }
         var_free(var);
      }
      if (flag)
         fputc('\n', fp);
   }
}

