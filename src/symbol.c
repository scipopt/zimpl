#ident "@(#) $Id: symbol.c,v 1.1 2001/01/26 07:11:37 thor Exp $"
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
   Entry**      entry;
   Symbol*      next;
};

static int    symbols = 0;
#ifndef NDEBUG
static Symbol anchor  = { 0, "", 0, 0, 0, SYM_ERR, NULL, NULL, NULL };
#else
static Symbol anchor  = { "", 0, 0, 0, SYM_ERR, NULL, NULL, NULL };
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
      free(p);
   }
   anchor.next = NULL;
   symbols     = 0;
}

int symbol_is_valid(const Symbol* symbol)
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

int symbol_lookup_tuple(const Symbol* sym, const Tuple* tuple)
{
   int i;
   
   assert(symbol_is_valid(sym));
   assert(tuple_is_valid(tuple));

   for(i = 0; i < sym->used; i++)
      if (!entry_cmp(sym->entry[i], tuple))
         break;
   
   return i < sym->used ? i : -1;   
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

#if TEST_DUBLICATE   
   if (symbol_lookup_tuple(sym, tuple) >= 0)
      fprintf(stderr, "Dublicate entry\n");
   else
#endif
   {
      /* Falls noch nicht geschehen, legen wir hier den Typ des
       * Symbols fest.
       */
      if ((sym->type == SYM_ERR) && (sym->used == 0))
         sym->type = entry_get_type(entry);

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
   
   for(sym = anchor.next; sym != NULL; sym = sym->next)
      symbol_print(fp, sym);
}

