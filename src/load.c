#pragma ident "@(#) $Id: load.c,v 1.21 2003/10/04 16:22:08 bzfkocht Exp $"
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
         exit(EXIT_FAILURE);
      }
      if (c == EOF)
      {
         if (cnt > 0)
         {
            (*buf)[cnt] = '\0';
            fprintf(stderr, "--- Warning 162: Line %d: Trailing \"%.20s\" ignored\n",
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
   StmtType type;

   assert(prog     != NULL);
   assert(filename != NULL);
   assert(text     != NULL);

   if (!strncmp(text, "set ", 4))
      type = STMT_SET;
   else if (!strncmp(text, "param ", 6))
      type = STMT_PARAM;
   else if (!strncmp(text, "var ", 4))
      type = STMT_VAR;
   else if (!strncmp(text, "minimize ", 9))
      type = STMT_MIN;
   else if (!strncmp(text, "maximize ", 9))
      type = STMT_MAX;
   else if (!strncmp(text, "subto ", 6))
      type = STMT_CONS;
   else if (!strncmp(text, "defnumb ", 8))
      type = STMT_DEF;
   else if (!strncmp(text, "defstrg ", 8))
      type = STMT_DEF;
   else if (!strncmp(text, "defset ", 7))
      type = STMT_DEF;
   else if (!strncmp(text, "do ", 3))
      type = STMT_DO;
   else
   {
      fprintf(stderr, "*** Error 163: Line %d: Syntax Error\n", lineno);
      show_source(stderr, text, 1);

      exit(EXIT_FAILURE);
   }
   prog_add_stmt(prog, stmt_new(type, filename, lineno, text));
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
      exit(EXIT_FAILURE);
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




