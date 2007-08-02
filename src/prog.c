#pragma ident "@(#) $Id: prog.c,v 1.15 2007/08/02 08:36:55 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: prog.c                                                        */
/*   Name....: Program Functions                                             */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2007 by Thorsten Koch <koch@zib.de>
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

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lint.h"
#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "code.h"

#define PROG_SID         0x50726f67
#define PROG_EXTEND_SIZE 100

struct program
{
   SID
   int         size;
   int         used;
   int         extend;
   Stmt**      stmt;
};

Prog* prog_new()
{
   Prog* prog = calloc(1, sizeof(*prog));

   assert(prog != NULL);

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
   assert(prog->stmt     != NULL);
   
   SID_del(prog);

   for(i = 0; i < prog->used; i++)
      stmt_free(prog->stmt[i]);

   free(prog->stmt);
   free(prog);
}

Bool prog_is_valid(const Prog* prog)
{
   return ((prog != NULL) && SID_ok(prog, PROG_SID));
}

Bool prog_is_empty(const Prog* prog)
{
   return prog->used == 0;
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

void prog_print(FILE* fp, const Prog* prog)
{
   int i;

   assert(prog_is_valid(prog));
   
   fprintf(fp, "Statements: %d\n", prog->used);

   for(i = 0; i < prog->used; i++)
      stmt_print(fp, prog->stmt[i]);
}

void prog_execute(const Prog* prog)
{
   int i;

   assert(prog_is_valid(prog));

   code_clear_inst_count();

   for(i = 0; i < prog->used; i++)
   {
      stmt_parse(prog->stmt[i]);
      stmt_execute(prog->stmt[i]);

      /* These calls should make sure, that all output is really
       * flushed out, even in a Batch environment.
       */
      fflush(stdout);
      fflush(stderr);

#ifdef USE_FSYNC
      /* This is to force the output do disk. It is to my knowledge
       * only needed on AIX batch systems that seem not to flush
       * the output buffer. If then the job is killed for some reason
       * no output is generated.
       */
      (void)fsync(fileno(stdout));
      (void)fsync(fileno(stderr));
#endif
   }
   if (verbose >= VERB_NORMAL)
      printf("Instructions evaluated: %u\n", code_get_inst_count());
}

char* prog_tostr(const Prog* prog, const char* prefix, const char* title)
{
   int   len;
   char* text;
   int   pos = 0;
   int   i;

   assert(prog_is_valid(prog));
   assert(prefix != NULL);

   /* prefix + title + \n
    * prog->used * (prefix + stmt + \n
    * \0
    */
   len = (prog->used + 1) * (strlen(prefix) + 1) + strlen(title) + 1;

   for(i = 0; i < prog->used; i++)
      len += strlen(stmt_get_text(prog->stmt[i]));

   text = calloc(len, sizeof(*text));

   pos = sprintf(&text[pos], "%s%s\n", prefix, title);
   
   for(i = 0; i < prog->used; i++)
      pos += sprintf(&text[pos], "%s%s\n", prefix, stmt_get_text(prog->stmt[i]));

   assert(pos + 1 == len);
   
   return text;
}
