#ident "@(#) $Id: load.c,v 1.4 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: load.c                                                        */
/*   Name....: Modell Loading Routines                                       */
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
#include <alloca.h>
#include <ctype.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"

#define BUF_EXT 65536

static char* get_line(char** buf, int* size, FILE* fp, int* lineno)
{
   int cnt = 0;
   int c;

   for(;;)
   {
      assert(cnt <= *size);
      
      if (cnt == *size - 1)
      {
         *size += BUF_EXT;
         *buf   = realloc(*buf, (size_t)*size);
      }
      assert(*buf != NULL);
      
      c = fgetc(fp);

      if (c == EOF)
         return NULL;

      if (c == '\n')
      {
         c = ' ';
         (*lineno)++;
      }

      if (c == '#')
      {
         do { c = fgetc(fp); } while((c != EOF) && (c != '\n'));
         (*lineno)++;
         continue;
      }
      (*buf)[cnt++] = (char)c;

      if (c == ';')
         break;      
   }
   (*buf)[cnt] = '\0';
   
   return *buf;
}

static void add_stmt(Prog* prog, int lineno, const char* text)
{
   const char* separ = " :;";
   StmtType type;
   char*    copy;
   char*    s;
   char*    name;
   
   assert(prog != NULL);
   assert(text != NULL);

   if (strlen(text) < 4)
      goto syntax_error;

   copy = strcpy(alloca(strlen(text) + 1), text);

   assert(copy != NULL);
   
   if (NULL == (s = strtok(copy, separ)))
      goto syntax_error;

   if (!strcmp(s, "set"))
      type = STMT_SET;
   else if (!strcmp(s, "param"))
      type = STMT_PARAM;
   else if (!strcmp(s, "var"))
      type = STMT_VAR;
   else if (!strcmp(s, "minimize"))
      type = STMT_MIN;
   else if (!strcmp(s, "maximize"))
      type = STMT_MAX;
   else if (!strcmp(s, "data"))
      type = STMT_DATA;
   else if (!strcmp(s, "subto"))
      type = STMT_CONS;
   else
      goto syntax_error;

   if (NULL == (name = strtok(NULL, separ)))
      goto syntax_error;

   prog_add_stmt(prog, stmt_new(type, lineno, name, text));
   
   return;
   
 syntax_error:
   
   fprintf(stderr, "*** Line %d: Syntax Error\n", lineno);
   exit(1);
}

void prog_load(Prog* prog)
{
   int   bufsize = BUF_EXT;
   char* buf     = malloc((size_t)bufsize);
   FILE* fp;
   char* s;
   int   lineno = 1;
   
   assert(prog != NULL);
   assert(buf  != NULL);

   if ((fp = fopen(prog_get_filename(prog), "r")) == NULL)
   {
      perror(prog_get_filename(prog));
      exit(1);
   }
   
   while((s = get_line(&buf, &bufsize, fp, &lineno)) != NULL)
   {
      while(isspace(*s))
         s++;

      if (*s == '\0')
         continue;

      add_stmt(prog, lineno, s);
   }   
   fclose(fp);
   free(buf);
}




