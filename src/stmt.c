#ident "@(#) $Id: stmt.c,v 1.4 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: stmt.c                                                        */
/*   Name....: Statement Functions                                           */
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
#include "code.h"
#include "inst.h"

#define STMT_SID 0x53746d74

struct statement
{
   SID
   StmtType    type;
   int         lineno;
   const char* name;
   const char* text;
   CodeNode*   node;
};

Stmt* stmt_new(StmtType type, int lineno, const char* name, const char* text)
{
   Stmt* stmt = calloc(1, sizeof(*stmt));;

   assert(name  != NULL);
   assert(text  != NULL);
   assert(stmt  != NULL);
   assert(lineno > 0);
   
   stmt->type   = type;
   stmt->lineno = lineno;
   stmt->name   = strdup(name);
   stmt->text   = strdup(text);
   stmt->node   = NULL;
   
   SID_set(stmt, STMT_SID);
   assert(stmt_is_valid(stmt));

   return stmt;
}

void stmt_free(Stmt* stmt)
{
   assert(stmt_is_valid(stmt));
   assert(stmt->name != NULL);
   assert(stmt->text != NULL);

   SID_del(stmt);
   
   if (stmt->node != NULL)
      code_free(stmt->node);
   
   free((void*)stmt->name);
   free((void*)stmt->text);
   free(stmt);
}

Bool stmt_is_valid(const Stmt* stmt)
{
   return ((stmt != NULL) && SID_ok(stmt, STMT_SID));
}

const char* stmt_get_name(const Stmt* stmt)
{
   assert(stmt_is_valid(stmt));

   return stmt->name;
}

void stmt_parse(Stmt* stmt)
{
   assert(stmt_is_valid(stmt));

   if (verbose)
   {
      fprintf(stderr, "Parsing: ");
      stmt_print(stderr, stmt);
   }
   parse_string(stmt->text, stmt->lineno);

   stmt->node = code_get_root();
}

void stmt_execute(const Stmt* stmt)
{
   assert(stmt_is_valid(stmt));

   if (verbose)
   {
      fprintf(stderr, "Executing: ");
      stmt_print(stderr, stmt);
   }
   if (code_get_type(code_eval(stmt->node)) != CODE_VOID)
   {
      fprintf(stderr, "*** Error: Execute must return void element\n");
      exit(1);
   }
}

void stmt_print(FILE* fp, const Stmt* stmt)
{
   static const char* type_name[] =
   {
      "Unknown", "Set", "Param", "Var", "Min", "Max", "Cons", "Data"
   };
   assert(stmt_is_valid(stmt));

   /* Lint weiss hier dass das assert immer erfuellt sein muss.
    * aber wir wollen es trotzdem.
    */
   assert((unsigned int)stmt->type
      < (sizeof(type_name) / sizeof(type_name[0]))); /*lint !e650 */

   fprintf(fp, "%04d %-7s %-10.10s [%s]\n",
      stmt->lineno,
      type_name[(int)stmt->type],
      stmt->name,
      stmt->text);
}
