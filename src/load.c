#pragma ident "@(#) $Id: load.c,v 1.16 2003/09/08 15:41:31 bzfkocht Exp $"
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

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>

#include "lint.h"
#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"

#define BUF_EXT 65536

static char* get_line(char** buf, int* size, FILE* fp, int* lineno)
{
   Bool in_string = FALSE;
   int  cnt = 0;
   int  c;

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

      if (in_string && ((c == EOF) || c == '\n'))
      {
         fprintf(stderr, "*** Error 161: Line %d: Unterminated string\n", *lineno);
         abort();
      }
      if (c == EOF)
      {
         if (cnt > 0)
         {
            (*buf)[cnt] = '\0';
            fprintf(stderr, "*** Error 162: Line %d: Trailing \"%s\" ignored\n",
               *lineno, *buf);
         }
         return NULL;
      }
      if (c == '\n')
      {
         c = ' ';
         (*lineno)++;
      }
      /* Skip leading white space
       */
      if (cnt == 0 && isspace(c))
         continue;
      
      if (c == '"')
         in_string = !in_string;

      /* Remove comments
       */
      if (!in_string && (c == '#'))
      {
         do { c = fgetc(fp); } while((c != EOF) && (c != '\n'));
         (*lineno)++;
         continue;
      }
      (*buf)[cnt++] = (char)c;

      if (!in_string && (c == ';'))
         break;      
   }
   (*buf)[cnt] = '\0';
   
   return *buf;
}

static const char* make_pathname(
   char*       target,
   const char* pathname,
   const char* filename)
{
   char* s;

   assert(target   != NULL);
   assert(pathname != NULL);
   assert(filename != NULL);

   /* Absolute Name ? */
   if (*filename == DIRSEP)
      strcpy(target, filename);
   else
   {
      strcpy(target, pathname);
   
      if (NULL == (s = strrchr(target, DIRSEP)))
         strcpy(target, filename);
      else
         strcpy(s + 1, filename);
   }
   return target;
}

static void add_stmt(
   Prog*       prog,
   const char* filename,
   const int   lineno,
   const char* text)
{
   const char* separ = " :;\t";

   StmtType type;
   char*    copy;
   char*    s;
   char*    name;

   assert(prog     != NULL);
   assert(filename != NULL);
   assert(text     != NULL);

   if (strlen(text) < 4)
      goto syntax_error;

   copy = strdup(text);

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
   else if (!strcmp(s, "subto"))
      type = STMT_CONS;
   else if (!strcmp(s, "print"))
      type = STMT_PRINT;
   else
      goto syntax_error;

   if (NULL == (name = strtok(NULL, separ)))
      goto syntax_error;

   prog_add_stmt(prog, stmt_new(type, filename, lineno, name, text));

   free(copy);
   return;
   
 syntax_error:
   
   fprintf(stderr, "*** Error 163: Line %d: Syntax Error\n", lineno);
   abort();
}

void prog_load(Prog* prog, const char* filename)
{
   int   bufsize = BUF_EXT;
   char* buf     = malloc((size_t)bufsize);
   FILE* fp;
   char* s;
   int   lineno  = 1;
   char  newname [1024];
   char* temp;
   char* myfilename;
   
   assert(prog     != NULL);
   assert(filename != NULL);
   assert(buf      != NULL);
   assert(filename != NULL);

   /* Allow for omission of the .zpl extension
    */
   myfilename = malloc(strlen(filename) + 5); /* ".zpl" + '\0' */
   assert(myfilename != NULL);
   strcpy(myfilename, filename);
   
   if (access(myfilename, R_OK) != 0)
   {
      strcat(myfilename, ".zpl");

      /* If .zpl also does not work, revert to the old name
       * to get a better error message.
       */
      if (access(myfilename, R_OK) != 0)
         strcpy(myfilename, filename);
   }
   if (verbose)
      printf("Reading %s\n", myfilename);

   if ((fp = fopen(myfilename, "r")) == NULL)
   {
      perror(myfilename);
      abort();
   }
   
   while((s = get_line(&buf, &bufsize, fp, &lineno)) != NULL)
   {
      assert(!isspace(*s));

      /* This could happen if we have a ;; somewhere.
       */
      if (*s == '\0')
         continue;

      if (1 == sscanf(s, "include \"%1023[^\"]\"", newname))
      {
         temp = malloc(strlen(myfilename) + strlen(newname) + 2);
         prog_load(prog, make_pathname(temp, myfilename, newname));
         free(temp);
      }
      else
      { 
         add_stmt(prog, myfilename, lineno, s);
      }
   }
   fclose(fp);
   free(buf);
   free(myfilename);
}




