#ident "$Id: zimpl.c,v 1.7 2002/05/26 12:44:57 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: zimpl.c                                                       */
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
#include <ctype.h>
#include <string.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"

#define FORM_LP  0
#define FORM_MPS 1

extern int yydebug;
extern int yy_flex_debug;

Bool verbose  = FALSE;
Bool zpldebug = FALSE;

static const char* banner = 
"****************************************************\n" \
"* Zuse Institute Mathematical Programming Language *\n" \
"* Release 1.02, Copyright (C)2002 by Thorsten Koch *\n" \
"****************************************************\n" \
"*   This is free software and you are welcome to   *\n" \
"*     redistribute it under certain conditions     *\n" \
"*      ZIMPL comes with ABSOLUTELY NO WARRANTY     *\n" \
"****************************************************\n" \
"\n" \
"usage: zimpl [-bdfhv][-t lp|mps][-o outfile] files\n" \
"\n" \
"  -b          enable bison debugging output.\n" \
"  -d          enable zimpl debugging output.\n" \
"  -f          enable flex debugging output.\n" \
"  -h          this help.\n" \
"  -v          enable verbose output.\n" \
"  -t lp|mps   select output format. Either LP (default) or MPS format.\n" \
"  -o outfile  select name for the output file. Default is the name of\n" \
"              the input file without extension.\n" \
"  filename    is the name of the input ZPL file.\n" \
"\n" ; 

static char* extend_basename(const char* filename, const char* extension)
{
   const char* s;
   const char* e;
   char*       basename;
   char*       t;
   
   assert(filename != NULL);
   assert(strlen(filename) > 0);
   assert(extension != NULL);
   
   s = strrchr(filename, '/');
   s = (s == NULL) ? filename : s + 1;
   e = strrchr(filename, '.');

   if ((e == NULL) || (e < s))
      e = filename + strlen(filename);

   if (s >= e)
   {
      fprintf(stderr, "*** Error: Bad filename \"%s\"\n", filename);
      abort();
   }
   assert(e - s > 0);
   
   basename = malloc(e - s + strlen(extension) + 1);

   assert(basename != NULL);
   
   for(t = basename; s < e; s++)
      *t++ = *s;
   *t = '\0';

   return strcat(basename, extension);
}

int main(int argc, char* const* argv)
{
   const char* usage = "usage: %s [-h][-v][-t lp|mps][-o outfile] filename\n";
   
   Prog*  prog;
   char*  outfile  = NULL;
   char*  tblfile  = NULL;
   char*  basefile = NULL;
   LpForm format   = LP_FORM_LPF;
   FILE*  fp_out;
   FILE*  fp_tbl;
   int    c;
   int    i;
   
   yydebug       = 0;
   yy_flex_debug = 0;

   while((c = getopt(argc, argv, "bdfho:t:v")) != -1)
   {
      switch(c)
      {
      case 'b' :
         yydebug = 1;
         break;
      case 'd' :
         zpldebug = TRUE;
         break;
      case 'h' :
         printf(banner);
         exit(0);
      case 'f' :
         yy_flex_debug = 1;
         break;
      case 'o' :
         basefile = strdup(optarg);
         break;
      case 't' :
         format = (tolower(*optarg) == 'm') ? LP_FORM_MPS : LP_FORM_LPF;
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
   outfile = extend_basename(
      (basefile == NULL) ? argv[optind] : basefile,
      (format == LP_FORM_LPF) ? ".lp" : ".mps");
   tblfile = extend_basename(
      (basefile == NULL) ? argv[optind] : basefile, ".tbl");
   
   str_init();
   elem_init();
   
   prog = prog_new();

   for(i = optind; i < argc; i++)
      prog_load(prog, argv[i]);

   if (zpldebug)
      prog_print(stderr, prog);
   
   lps_alloc(argv[optind]);

   prog_execute(prog);

   lps_scale();
   
   /* Write Output
    */
   if (NULL == (fp_out = fopen(outfile, "w")))
   {
      perror(outfile);
      abort();
   }
   lps_write(fp_out, format);

   fclose(fp_out);

   /* Write translation table
    */
   if (NULL == (fp_tbl = fopen(tblfile, "w")))
   {
      perror(tblfile);
      abort();
   }
   lps_transtable(fp_tbl);

   fclose(fp_tbl);

   if (zpldebug) 
      symbol_print_all(stderr);

#if defined(__INSURE__) || !defined(NDEBUG)
   
   /* Now clean up. 
    */
   prog_free(prog);

   lps_free();
   symbol_exit();
   elem_exit();
   str_exit();
   free(outfile);
   free(tblfile);

   if (basefile != NULL)
      free(basefile);
   
   mem_display(stderr);
#endif /* __INSURE__ || !NDEBUG */
   return 0;
}


