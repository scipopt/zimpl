#ident "@(#) $Id: prog.c,v 1.1 2001/01/26 07:11:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: prog.c                                                        */
/*   Name....: Program Functions                                             */
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

#define PROG_EXTEND_SIZE 100

struct program
{
   const char* filename;
   int         size;
   int         used;
   int         extend;
   Stmt**      stmt;
};

Prog* prog_new(const char* filename)
{
   Prog* prog = calloc(1, sizeof(*prog));

   assert(filename != NULL);
   assert(prog     != NULL);

   prog->filename = strdup(filename);
   prog->size     = PROG_EXTEND_SIZE;
   prog->used     = 0;
   prog->extend   = PROG_EXTEND_SIZE;
   prog->stmt     = calloc(prog->size, sizeof(*prog->stmt));

   assert(prog->stmt != NULL);
   
   return prog;
}

void prog_free(Prog* prog)
{
   int i;
   
   assert(prog           != NULL);
   assert(prog->filename != NULL);
   assert(prog->stmt     != NULL);
   
   for(i = 0; i < prog->used; i++)
      stmt_free(prog->stmt[i]);

   free(prog->stmt);
   free((void*)prog->filename);
   free(prog);
}

void prog_add_stmt(Prog* prog, Stmt* stmt)
{
   assert(prog != NULL);
   assert(stmt != NULL);

   assert(prog->used <= prog->size);
   
   if (prog->used == prog->size)
   {
      prog->size   += prog->extend;
      prog->extend += prog->extend;
      prog->stmt    = realloc(
         prog->stmt, (size_t)prog->size * sizeof(*prog->stmt));
      
      assert(prog->stmt != NULL);
   }
   assert(prog->used < prog->size);

   prog->stmt[prog->used] = stmt;
   prog->used++;   
}

const char* prog_get_filename(const Prog* prog)
{
   assert(prog           != NULL);
   assert(prog->filename != NULL);

   return prog->filename;
}

void prog_print(FILE* fp, const Prog* prog)
{
   int i;

   assert(prog != NULL);
   
   fprintf(fp, "Programm  : %s\n", prog->filename);
   fprintf(fp, "Statements: %d\n", prog->used);

   for(i = 0; i < prog->used; i++)
      stmt_print(fp, prog->stmt[i]);
}

int prog_execute(const Prog* prog)
{
   int i;

   printf("\\Problem name: %s\n", prog->filename);
   
   for(i = 0; i < prog->used; i++)
   {
      if (stmt_parse(prog->stmt[i]))
         break;

      if (stmt_execute(prog->stmt[i]))
         break;
   }
   printf("End\n");
   
   return (i == prog->used) ? SUCCESS : FAILURE;
}

