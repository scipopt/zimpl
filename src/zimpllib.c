#pragma ident "$Id: zimpllib.c,v 1.1 2005/09/02 02:02:43 bzfkocht Exp $"
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

#include <gmp.h>

#include "lint.h"
#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "xlpglue.h"
#include "gmpmisc.h"

void zimpl_read(const char* filename)
{
   Prog*       prog;
   Set*        set;

   gmp_init(verbose >= VERB_VERBOSE);
   str_init();
   numb_init();
   elem_init();
   set_init();

   set = set_pseudo_new();
   (void)symbol_new(SYMBOL_NAME_INTERNAL, SYM_VAR, set, 100, NULL);
   set_free(set);
   
   prog = prog_new();

   prog_load(prog, filename);

   if (prog_is_empty(prog))
   {
      fprintf(stderr, "*** Error 168: No program statements to execute\n");
      exit(EXIT_FAILURE);
   }
   if (verbose >= VERB_DEBUG)
      prog_print(stderr, prog);
   
   xlp_alloc(filename);

   prog_execute(prog);

   prog_free(prog);

   xlp_free();

   symbol_exit();
   define_exit();
   set_exit();
   elem_exit();
   numb_exit();
   str_exit();
   gmp_exit();
}
