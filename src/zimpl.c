#pragma ident "$Id: zimpl.c,v 1.33 2003/08/22 15:53:45 bzfkocht Exp $"
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

#define FORM_LP  0
#define FORM_MPS 1

extern int yydebug;
extern int yy_flex_debug;

Bool verbose   = FALSE;
Bool zpldebug  = FALSE;
Bool mangling  = TRUE;

static const char* banner = 
"****************************************************\n" \
"* Zuse Institute Mathematical Programming Language *\n" \
"* Release 1.99c Copyright (C)2003 by Thorsten Koch *\n" \
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
"  -r          write branching order file.\n" \
"  -h          show this help.\n" \
"  -p          presolve LP.\n" \
"  -v          enable verbose output.\n" \
"  -n cm|cn|cf name column make/name/full\n" \
"  -m          do not mangle variable names\n" \
"  -t lp|mps   select output format. Either LP (default) or MPS format.\n" \
"  -o outfile  select name for the output file. Default is the name of\n" \
"              the input file without extension.\n" \
"  -F filter   filter output, for example \"gzip -c >%%s.gz\"\n" \
"  filename    is the name of the input ZPL file.\n" \
"\n" ; 

static char* change_extention(const char* filename, const char* extension)
{
   char* basename;
   int   i;
   
   assert(filename != NULL);
   assert(strlen(filename) > 0);
   assert(extension != NULL);

   basename = malloc(strlen(filename) + strlen(extension) + 1);

   assert(basename != NULL);

   strcpy(basename, filename);
   
   /* clip old extention (if any)
    */
   for(i = strlen(basename) - 1; i >= 0; i--)
      if (basename[i] == '/' || basename[i] == '.')
         break;

   if (basename[i] == '.')
      basename[i] = '\0';

   if (strlen(basename) == 0)
   {
      fprintf(stderr, "*** Error: Bad filename \"%s\"\n", filename);
      abort();
   }
   return strcat(basename, extension);
}

static const char* strip_path(const char* filename)
{
   const char* s;
   
   assert(filename != NULL);
   assert(strlen(filename) > 0);
   
   s = strrchr(filename, '/');

   return (s == NULL) ? filename : s + 1;
}

static void check_write_ok(FILE* fp, const char* filename)
{
   if (ferror(fp))
   {
      fprintf(stderr, "*** Error: while writing file %s", filename);
      perror(" ");
   }
}

int main(int argc, char* const* argv)
{
   const char* usage =
      "usage: %s [-hvrp][-n cs|cn|cf][-t lp|mps][-o outfile][-F filter] filename\n";
   
   Prog*       prog;
   char*       filter   = NULL;
   char*       outfile  = NULL;
   char*       tblfile  = NULL;
   char*       ordfile  = NULL;
   char*       basefile = NULL;
   char*       cmdpipe  = NULL;
   LpFormat    format   = LP_FORM_LPF;
   FILE*       fp;
   Bool        write_order = FALSE;
   Bool        presolve    = FALSE;
   int         c;
   int         i;
   
   yydebug       = 0;
   yy_flex_debug = 0;

   while((c = getopt(argc, argv, "bdfF:hmn:o:prt:v")) != -1)
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
      case 'F' :
         filter = strdup(optarg);
         break;
      case 'm' :
         fprintf(stderr,
            "*WARNING* the generated LP/MPS files will be invalid\n");
         mangling = FALSE;
         break;
      case 'n' :
         if (*optarg != 'c')
         {
            fprintf(stderr, usage, argv[0]);
            exit(0);
         }
         switch(optarg[1])
         {
         case 'm' :
            conname_format(CON_FORM_MAKE);
            break;
         case 'n' :
            conname_format(CON_FORM_NAME);
            break;
         case 'f' :
            conname_format(CON_FORM_FULL);
            break;
         default :
            fprintf(stderr, usage, argv[0]);
            exit(0);
         }
         break;
      case 'o' :
         basefile = strdup(optarg);
         break;
      case 'p' :
         presolve = TRUE;
         break;
      case 'r' :
         write_order = TRUE;
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
   if (basefile == NULL)
      basefile = strdup(strip_path(argv[optind]));
   
   outfile = change_extention(basefile,
      (format == LP_FORM_LPF) ? ".lp" : ".mps");
   tblfile = change_extention(basefile, ".tbl");
   ordfile = change_extention(basefile, ".ord");

   if (filter == NULL)
      filter = strdup("cat >%s");

   cmdpipe = malloc(strlen(basefile) + strlen(filter) + 256);

   assert(cmdpipe != NULL);

   gmp_init(verbose);
   str_init();
   numb_init();
   elem_init();
      
   prog = prog_new();

   for(i = optind; i < argc; i++)
      prog_load(prog, argv[i]);

   if (zpldebug)
      prog_print(stderr, prog);
   
   xlp_alloc(argv[optind]);

   prog_execute(prog);

   /* Presolve
    */
   if (presolve)
      xlp_presolve();
   
   xlp_scale();
   
   /* Write Output
    */
   sprintf(cmdpipe, filter, outfile);

   if (verbose)
      printf("Writing [%s]\n", cmdpipe);
   
   if (NULL == (fp = popen(cmdpipe, "w")))
   {
      fprintf(stderr, "*** Error: when writing file %s", outfile);
      perror(" ");
      abort();
   }
   xlp_write(fp, format, "This file was automatically generated by Zimpl");

   check_write_ok(fp, outfile);
   
   pclose(fp);

   /* Write translation table
    */
   sprintf(cmdpipe, filter, tblfile);

   if (verbose)
      printf("Writing [%s]\n", cmdpipe);

   if (NULL == (fp = popen(cmdpipe, "w")))
   {
      fprintf(stderr, "*** Error: when writing file %s", tblfile);
      perror(" ");
      abort();
   }
   xlp_transtable(fp, format);

   check_write_ok(fp, tblfile);

   pclose(fp);

   /* Write order file 
    */
   if (write_order)
   {
      sprintf(cmdpipe, filter, ordfile);

      if (verbose)
         printf("Writing [%s]\n", cmdpipe);

      if (NULL == (fp = popen(cmdpipe, "w")))
      {
         fprintf(stderr, "*** Error: when writing file %s", ordfile);
         perror(" ");
         abort();
      }
      xlp_orderfile(fp);

      check_write_ok(fp, ordfile);
      
      pclose(fp);
   }
   
   if (zpldebug) 
      symbol_print_all(stderr);

#if defined(__INSURE__) || !defined(NDEBUG) || defined(FREEMEM)
   
   /* Now clean up. 
    */
   prog_free(prog);

   xlp_free();
   symbol_exit();
   elem_exit();
   numb_exit();
   str_exit();
   gmp_exit();

   free(ordfile);
   free(outfile);
   free(tblfile);
   free(basefile);
   free(filter);
   free(cmdpipe);
   
   mem_display(stderr);
#endif /* __INSURE__ || !NDEBUG || FREEMEM */
   return 0;
}


