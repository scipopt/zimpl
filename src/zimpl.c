#ident "$Id: zimpl.c,v 1.3 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: zimple.c                                                      */
/*   Name....: Zuse Institute Mathematical Programming Language              */
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"

extern int yydebug;
extern int yy_flex_debug;

Bool verbose = FALSE;

static const char* banner = 
"****************************************************\n" \
"* Zuse Institute Mathematical Programming Language *\n" \
"* Release 1.0, Copyright (C) 2001 by Thorsten Koch *\n" \
"****************************************************\n" \
"*   This is free software and you are welcome to   *\n" \
"*     redistribute it under certain conditions     *\n" \
"*     ZIMPLE comes with ABSOLUTELY NO WARRANTY     *\n" \
"****************************************************\n";

int main(int argc, char** argv)
{
   const char* usage = "usage: %s [-h] filename\n";
   
   Prog* prog;
   int   c;

   yydebug       = 0;
   yy_flex_debug = 0;

   while((c = getopt(argc, argv, "bfhv")) != -1)
   {
      switch(c)
      {
      case 'b' :
         yydebug = 1;
         break;
      case 'h' :
         printf(banner);
         exit(0);
      case 'f' :
         yy_flex_debug = 1;
         break;
      case 'v' :
         verbose = TRUE;
         break;         
      case '?':
         fprintf(stderr, usage, argv[0]);
         exit(0);
      default :
         abort();
      }
   }
   if ((argc - optind) < 1)
   {
      fprintf(stderr, usage, argv[0]);      
      exit(0);
   }
   str_init();
   elem_init();
   
   prog = prog_new(argv[optind]);
   
   prog_load(prog);

   if (verbose)
      prog_print(stderr, prog);
      
   prog_execute(prog);
      
   prog_free(prog);

   if (verbose)
      symbol_print_all(stderr);
   
   symbol_exit();
   elem_exit();
   str_exit();
   
   mem_display(stderr);
   
   return 0;
}


