#ident "@(#) $Id: prog.c,v 1.4 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: prog.c                                                        */
/*   Name....: Program Functions                                             */
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

#define PROG_SID         0x50726f67
#define PROG_EXTEND_SIZE 100

struct program
{
   SID
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

   SID_set(prog, PROG_SID);
   assert(prog_is_valid(prog));

   return prog;
}

void prog_free(Prog* prog)
{
   int i;
   
   assert(prog_is_valid(prog));
   assert(prog->filename != NULL);
   assert(prog->stmt     != NULL);
   
   SID_del(prog);

   for(i = 0; i < prog->used; i++)
      stmt_free(prog->stmt[i]);

   free(prog->stmt);
   free((void*)prog->filename);
   free(prog);
}

Bool prog_is_valid(const Prog* prog)
{
   return ((prog != NULL) && SID_ok(prog, PROG_SID));
}

void prog_add_stmt(Prog* prog, Stmt* stmt)
{
   assert(prog_is_valid(prog));
   assert(stmt_is_valid(stmt));

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
   assert(prog_is_valid(prog));
   assert(prog->filename != NULL);

   return prog->filename;
}

void prog_print(FILE* fp, const Prog* prog)
{
   int i;

   assert(prog_is_valid(prog));
   
   fprintf(fp, "Programm  : %s\n", prog->filename);
   fprintf(fp, "Statements: %d\n", prog->used);

   for(i = 0; i < prog->used; i++)
      stmt_print(fp, prog->stmt[i]);
}

void prog_execute(const Prog* prog)
{
   int i;

   assert(prog_is_valid(prog));

   printf("\\Problem name: %s\n", prog->filename);
   
   for(i = 0; i < prog->used; i++)
   {
      stmt_parse(prog->stmt[i]);
      stmt_execute(prog->stmt[i]);
   }
   symbol_print_bounds(stdout);
   
   printf("End\n");
}

