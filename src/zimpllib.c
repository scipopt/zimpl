#pragma ident "$Id: zimpllib.c,v 1.12 2006/09/09 10:00:22 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: zimpllib.c                                                    */
/*   Name....: ZIMPL Library Interface                                       */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2005 by Thorsten Koch <koch@zib.de>
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
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>

#include "lint.h"
#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "xlpglue.h"
#include "zimpllib.h"

extern int yydebug;
extern int yy_flex_debug;

int verbose = VERB_QUIET;

static const char* banner = 
"****************************************************\n" \
"* Zuse Institute Mathematical Programming Language *\n" \
"* Release %-5s Copyright (C)2006 by Thorsten Koch *\n" \
"****************************************************\n\n";

static jmp_buf zpl_read_env;
static Bool    is_longjmp_ok = FALSE;

void zpl_exit(int retval)
{
   if (is_longjmp_ok)
      longjmp(zpl_read_env, retval);

#ifdef NDEBUG
   exit(retval);
#else
   abort(); /* to get a stack trace */
#endif
}

static Bool is_valid_identifier(const char* s)
{
   assert(s != NULL);

   /* Identifiers start with a letter or a '_'
    */
   if (!isalpha(*s) || *s == '_')
      return FALSE;

   /* Then letters, digits or '_' can follow.
    */
   while(isalnum(*++s) || *s == '_')
      ;

   return *s == '\0';
}

void zpl_add_parameter(const char* def)
{
   const char* warning =
      "--- Warning 175: Illegal syntax for command line define \"%s\" -- ignored\n";
   Set*    set;
   Symbol* sym;
   Numb*   numb;
   Tuple*  tuple;
   Entry*  entry;
   char*   name;
   char*   value;

   assert(def != NULL);
   
   name  = strdup(def);
   value = strchr(name, '=');
   
   if (value == NULL)
   {
      fprintf(stderr, warning, def);
      free(name);
      return;
   }
   *value = '\0';
   value++;

   if (strlen(name) == 0 || strlen(value) == 0 || !is_valid_identifier(name))
   {
      fprintf(stderr, warning, def);
      free(name);
      return;
   }
   set   = set_pseudo_new();
   sym   = symbol_new(str_new(name), SYM_ERR, set, 1, ENTRY_NULL);
   tuple = tuple_new(0);   

   if (!numb_is_number(value))
      entry = entry_new_strg(tuple, str_new(value));
   else
   {
      numb  = numb_new_ascii(value);
      entry = entry_new_numb(tuple, numb);
      numb_free(numb);
   }
   symbol_add_entry(sym, entry);
   
   tuple_free(tuple);
   set_free(set); 
   free(name); 
}

Bool zpl_read(const char* filename)
{
   Prog*       prog = NULL;
   Set*        set;
   Bool        ret = FALSE;
   
   yydebug       = 0;
   yy_flex_debug = 0;

   printf(banner, VERSION);

   str_init();
   numb_init(13021967UL);
   elem_init();
   set_init();
   mio_init();
   interns_init();
   local_init();
   
   if (0 == setjmp(zpl_read_env))
   {
      is_longjmp_ok = TRUE;
      
      set = set_pseudo_new();
      (void)symbol_new(SYMBOL_NAME_INTERNAL, SYM_VAR, set, 100, NULL);
      set_free(set);
   
      prog = prog_new();

      prog_load(prog, filename);

      if (prog_is_empty(prog))
         fprintf(stderr, "*** Error 168: No program statements to execute\n");
      else
      {
         if (verbose >= VERB_DEBUG)
            prog_print(stderr, prog);
   
         xlp_alloc(filename);

         prog_execute(prog);

         ret = TRUE;
      }
   }
   is_longjmp_ok = FALSE;

   xlp_free();

   if (prog != NULL)
      prog_free(prog);

   local_exit();
   interns_exit();
   mio_exit();
   symbol_exit();
   define_exit();
   set_exit();
   elem_exit();
   numb_exit();
   str_exit();

   return ret;
}

Bool zpl_read_with_args(int argc, char** argv)
{
   const char* options = "D:sv:";

   unsigned long seed = 13021967UL;
   char**        param_table;
   int           param_count = 0;
   int           c;
   int           i;
   Prog*         prog = NULL;
   Set*          set;
   Bool          ret = FALSE;
   
   yydebug       = 0;
   yy_flex_debug = 0;
   param_table   = malloc(sizeof(*param_table));

   printf(banner, VERSION);

   /* getopt might be called more than once
    */
   optind = 1;
   
   while((c = getopt(argc, argv, options)) != -1)
   {
      switch(c)
      {
      case 'D' :
         param_table =
            realloc(param_table, ((unsigned int)param_count + 1) * sizeof(*param_table));
         param_table[param_count] = strdup(optarg);

         if (verbose >= VERB_DEBUG)
            printf("Parameter %d [%s]\n", param_count, param_table[param_count]);

         param_count++;
         break;
      case 's' :
         seed = (unsigned long)atol(optarg);
         break;
      case 'v' :
         verbose = atoi(optarg);
         break;
      case '?':
         fprintf(stderr, "Unknown option '%c'\n", c);
         return FALSE;
      default :
         abort();
      }
   }
   if ((argc - optind) < 1)
   {
      fprintf(stderr, "Filename missing\n");
      return FALSE;
   }

   str_init();
   numb_init(seed);
   elem_init();
   set_init();
   mio_init();
   interns_init();
   local_init();
   
   if (0 == setjmp( zpl_read_env))
   {
      is_longjmp_ok = TRUE;
      
      /* Make symbol to hold entries of internal variables
       */
      set = set_pseudo_new();
      (void)symbol_new(SYMBOL_NAME_INTERNAL, SYM_VAR, set, 100, NULL);
      set_free(set);
   
      /* Now store the param defines
       */
      for(i = 0; i < param_count; i++)
         zpl_add_parameter(param_table[i]);

      prog = prog_new();

      for(i = optind; i < argc; i++)
         prog_load(prog, argv[i]);

      if (prog_is_empty(prog))
         fprintf(stderr, "*** Error 168: No program statements to execute\n");
      else
      {
         if (verbose >= VERB_DEBUG)
            prog_print(stderr, prog);
   
         xlp_alloc(argv[optind]);

         prog_execute(prog);

         ret = TRUE;
      }
   }
   is_longjmp_ok = FALSE;

   xlp_free();

   /* Now clean up. 
    */
   for(i = 0; i < param_count; i++)
      free(param_table[i]);
   free(param_table);

   if (prog != NULL)
      prog_free(prog);

   local_exit();
   interns_exit();
   mio_exit();
   symbol_exit();
   define_exit();
   set_exit();
   elem_exit();
   numb_exit();
   str_exit();

   return ret;
}
