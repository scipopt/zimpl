#ident "@(#) $Id: stmt.c,v 1.2 2001/01/29 13:45:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: stmt.c                                                        */
/*   Name....: Statement Functions                                           */
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

int stmt_is_valid(const Stmt* stmt)
{
   return ((stmt != NULL) && SID_ok(stmt, STMT_SID));
}

const char* stmt_get_name(const Stmt* stmt)
{
   assert(stmt_is_valid(stmt));

   return stmt->name;
}

int stmt_parse(Stmt* stmt)
{
   assert(stmt_is_valid(stmt));

   if (verbose)
   {
      fprintf(stderr, "Parsing: ");
      stmt_print(stderr, stmt);
   }
   parse_string(stmt->text, stmt->lineno);

   stmt->node = code_get_root();

   return SUCCESS;
}

int stmt_execute(const Stmt* stmt)
{
   assert(stmt_is_valid(stmt));

   if (verbose)
   {
      fprintf(stderr, "Executing: ");
      stmt_print(stderr, stmt);
   }
   code_execute(stmt->node);

   return SUCCESS;
}

void stmt_print(FILE* fp, const Stmt* stmt)
{
   static const char* type_name[] =
   {
      "Unknown", "Set", "Param", "Var", "Min", "Max", "Cons", "Data"
   };
   assert(stmt_is_valid(stmt));
   assert(((int)stmt->type) < (sizeof(type_name) / sizeof(type_name[0])));

   fprintf(fp, "%04d %-7s %-10.10s [%s]\n",
      stmt->lineno,
      type_name[(int)stmt->type],
      stmt->name,
      stmt->text);
}
