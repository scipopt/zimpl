#pragma ident "$Id: zimpllib.c,v 1.6 2006/01/27 19:57:32 bzfkocht Exp $"
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

#include <gmp.h>

#include "lint.h"
#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "xlpglue.h"
#include "gmpmisc.h"
#include "zimpllib.h"

extern int yydebug;
extern int yy_flex_debug;

int verbose = VERB_QUIET;

static const char* banner = 
"****************************************************\n" \
"* Zuse Institute Mathematical Programming Language *\n" \
"* Release %-5s Copyright (C)2005 by Thorsten Koch *\n" \
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
   abort(); /* to get a stack strace */
#endif
}

Bool zpl_read(const char* filename)
{
   Prog*       prog = NULL;
   Set*        set;
   Bool        ret = FALSE;
   
   yydebug       = 0;
   yy_flex_debug = 0;

   printf(banner, VERSION);

   gmp_init(verbose >= VERB_VERBOSE);
   str_init();
   numb_init(13021967UL);
   elem_init();
   set_init();
 
   if (0 == setjmp( zpl_read_env))
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

   symbol_exit();
   define_exit();
   set_exit();
   elem_exit();
   numb_exit();
   str_exit();
   gmp_exit();

   return ret;
}
