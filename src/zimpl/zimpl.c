/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: zimpl.c                                                       */
/*   Name....: Zuse Institute Mathematical Programming Language              */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2018 by Thorsten Koch <koch@zib.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "zimpl/lint.h"
#include <stdbool.h>
#include "zimpl/mshell.h"
#include "zimpl/stkchk.h"
#include "zimpl/random.h"
#include "zimpl/blkmem.h"
#include "zimpl/ratlptypes.h"
#include "zimpl/numb.h"
#include "zimpl/elem.h"
#include "zimpl/tuple.h"
#include "zimpl/mme.h"
#include "zimpl/set.h"
#include "zimpl/symbol.h"
#include "zimpl/define.h"
#include "zimpl/bound.h"
#include "zimpl/mono.h"
#include "zimpl/term.h"
#include "zimpl/stmt.h"
#include "zimpl/local.h"
#include "zimpl/list.h"
#include "zimpl/entry.h"
#include "zimpl/conname.h"
#include "zimpl/xlpglue.h"
#include "zimpl/zlpglue.h"
#include "zimpl/prog.h"
#include "zimpl/metaio.h"
#include "zimpl/strstore.h"
#include "zimpl/zimpllib.h"

extern int yydebug;
extern int yy_flex_debug;

static const char* const options = "bD:fF:hl:mn:o:OP:rs:t:v:V";
static const char* const usage   = "usage: %s [options] file ...\n";
static const char* const title   = "This file was automatically generated by Zimpl";

static const char* const help =
"\n" \
"  -b             enable bison debugging output.\n" \
"  -D name=value  assign value to parameter name.\n" \
"  -f             enable flex debugging output.\n" \
"  -F filter      filter output, for example \"gzip -c >%%s.gz\"\n" \
"  -h             show this help.\n" \
"  -l length      Maximum length of names in output file.\n" \
"  -m             write CPLEX MIP start value file.\n"
"  -n cm|cn|cf    name constraint make/name/full\n" \
"  -o outfile     select name for the output file. Default is the name of\n" \
"                 the input file without extension.\n" \
"  -P cmd         Pipe input through command, e.g. \"cpp -DONLY_X %%s\"\n" \
"  -r             write CPLEX branching order file.\n" \
"  -s seed        random number generator seed.\n" \
"  -t lp|mps|hum|rlp|pip  select output format. Either LP (default), MPS format,\n" \
"                 human readable HUM, randomly permuted LP, or PIP polynomial IP.\n" \
"  -v[0-5]        verbosity level: 0 = quiet, 1 = default, up to 5 = debug\n" \
"  -V             print program version\n" \
"  filename       is the name of the input ZPL file.\n" \
"\n" ; 

/* "  -O             optimize LP by preprocessing.\n" \
 */
#ifdef WITH_CALLTRACE /* Does only work with gcc */
void __cyg_profile_func_enter(void *this_fn, void *call_site) __attribute__((no_instrument_function));
void __cyg_profile_func_exit(void *this_fn, void *call_site) __attribute__((no_instrument_function));
static int call_depth = 0;

void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
   call_depth++;
   
   fprintf(stderr, "%p %d\n", this_fn, call_depth);
}

void __cyg_profile_func_exit(void *this_fn, void *call_site) 
{
   call_depth--;
}
#endif

static char* add_extention(const char* filename, const char* extension)
{
   char* basename;
   
   assert(filename != NULL);
   assert(strlen(filename) > 0);
   assert(extension != NULL);

   basename = malloc(strlen(filename) + strlen(extension) + 1);

   assert(basename != NULL);

   strcpy(basename, filename);

   /*lint -e{429} suppress 'Custodial pointer 'basename' (line 96) has not been freed or returned' */
   return strcat(basename, extension);
}

static const char* strip_path(const char* filename)
{
   const char* s;
   
   assert(filename != NULL);
   assert(strlen(filename) > 0);
   
   s = strrchr(filename, DIRSEP);

   return (s == NULL) ? filename : s + 1;
}

static char* strip_extension(char* filename)
{
   int i;
   
   assert(filename != NULL);
   assert(strlen(filename) > 0);

   for(i = (int)strlen(filename) - 1; i >= 0; i--)
      if (filename[i] == DIRSEP || filename[i] == '.')
         break;

   if (i >= 0 && filename[i] == '.')
      filename[i] = '\0';

   i = (int)strlen(filename);
   
   if (i == 0 || filename[i - 1] == DIRSEP)
   {
      fprintf(stderr, "*** Error 101: Bad filename\n");
      exit(EXIT_FAILURE);
   }
   return filename;
}

static void check_write_ok(FILE* fp, const char* filename)
{
   if (ferror(fp))
   {
      fprintf(stderr, "*** Error 102: File write error\n");
      perror(filename);
   }
}

int main(int argc, char* const* argv)
{
   Prog*         prog;
   Set*          set;
   void*         lp;
   const char*   extension = "";
   char*         filter    = strdup("%s");
   char*         outfile;
   char*         tblfile;
   char*         ordfile;
   char*         mstfile;
   char*         basefile = NULL;
   char*         inppipe  = NULL;
   char*         outpipe;
   LpFormat      format   = LP_FORM_LPF;
   FILE*         fp;
   bool          write_order = false;
   bool          write_mst   = false;
   bool          presolve    = false;
   int           name_length = 0;
   char*         prog_text;
   unsigned long seed = 13021967UL;
   char**        param_table;
   int           param_count = 0;
   int           c;
   int           i;
   FILE*         (*openfile)(const char*, const char*) = fopen;
   int           (*closefile)(FILE*)                   = fclose;

   stkchk_init();
   
   yydebug       = 0;
   yy_flex_debug = 0;
   verbose       = VERB_NORMAL;
   param_table   = malloc(sizeof(*param_table));
   
   while((c = getopt(argc, argv, options)) != -1)
   {
      switch(c)
      {
      case 'b' :
         yydebug = 1;
         break;
      case 'D' :
         param_table =
            realloc(param_table, ((unsigned int)param_count + 1) * sizeof(*param_table));
         param_table[param_count] = strdup(optarg);
         param_count++;
         break;
      case 'h' :
         zpl_print_banner(stdout, true);
         printf(usage, argv[0]);
         puts(help);
         exit(0);
      case 'f' :
         yy_flex_debug = 1;
         break;
      case 'F' :
         free(filter);
         
         filter    = strdup(optarg);
         openfile  = popen;
         closefile = pclose;
         break;
      case 'l' :
         name_length = atoi(optarg);
         break;
      case 'm' :
         write_mst = true;
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
      case 'O' :
         presolve = true;
         break;
      case 'P' :
         inppipe = strdup(optarg);
         break;
      case 's' :
         seed = (unsigned long)atol(optarg);
         break;
      case 'r' :
         write_order = true;
         break;
      case 't' :
         switch(tolower(*optarg))
         {
         case 'h' :
            format = LP_FORM_HUM;
            break;
         case 'm' :
            format = LP_FORM_MPS;
            break;
         case 'l' :
            format = LP_FORM_LPF;
            break;
         case 'p' :
            format = LP_FORM_PIP;
            break;
         case 'r' :
            format = LP_FORM_RLP;
            break;
         default :
            if (verbose > VERB_QUIET)
               fprintf(stderr,
                  "--- Warning 103: Output format \"%s\" not supported, using LP format\n",
                  optarg);
            format = LP_FORM_LPF;
            break;
         }
         break;
      case 'v' :
         verbose = atoi(optarg);
         break;
      case 'V' :
         printf("%s\n", VERSION);
         exit(0);
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

   zpl_print_banner(stdout, true);
   
   if (basefile == NULL)
      basefile = strip_extension(strdup(strip_path(argv[optind])));

   switch(format)
   {
   case LP_FORM_LPF :
      extension = ".lp";
      break;
   case LP_FORM_MPS :
      extension = ".mps";
      break;
   case LP_FORM_HUM :
      extension = ".hum";
      break;
   case LP_FORM_RLP :
      extension = ".rlp";
      break;
   case LP_FORM_PIP :
      extension = ".pip";
      break;
   default :
      abort();
   }
   assert(extension != NULL);

   outfile = add_extention(basefile, extension);
   tblfile = add_extention(basefile, ".tbl");
   ordfile = add_extention(basefile, ".ord");
   mstfile = add_extention(basefile, ".mst");
   
   outpipe = malloc(strlen(basefile) + strlen(filter) + 256);

   assert(outpipe != NULL);

   blk_init();
   str_init();
   rand_init(seed);
   numb_init(true);
   elem_init();
   set_init();
   mio_init();
   interns_init();
   local_init();
   
   /* Make symbol to hold entries of internal variables
    */
   set = set_pseudo_new();
   (void)symbol_new(SYMBOL_NAME_INTERNAL, SYM_VAR, set, 100, ENTRY_NULL);
   set_free(set);
   
   /* Now store the param defines
    */
   for(i = 0; i < param_count; i++)
      zpl_add_parameter(param_table[i]);

   /* Next we read in the zpl program(s)
    */
   prog = prog_new();

   for(i = optind; i < argc; i++)
      prog_load(prog, inppipe, argv[i]);

   if (prog_is_empty(prog))
   {
      fprintf(stderr, "*** Error 168: No program statements to execute\n");
      exit(EXIT_FAILURE);
   }
   if (verbose >= VERB_DEBUG)
      prog_print(stderr, prog);
   
   lp = xlp_alloc(argv[optind], write_mst || write_order, NULL);
   zlp_setnamelen(lp, name_length);
   
   prog_execute(prog, lp);

   /* Presolve
    */
   if (presolve)
      fprintf(stderr, "--- Warning: Presolve no longer support. If you need it, send me an email\n");
#if 0 
      if (!zlp_presolve())
         exit(EXIT_SUCCESS);
#endif
   if (verbose >= VERB_NORMAL)
      zlp_stat(lp);
   
   /* Write order file 
    */
   if (write_order)
   {
      sprintf(outpipe, filter, ordfile, "ord");

      if (verbose >= VERB_NORMAL)
         printf("Writing [%s]\n", outpipe);

      if (NULL == (fp = (*openfile)(outpipe, "w")))
      {
         fprintf(stderr, "*** Error 104: File open failed ");
         perror(ordfile);
         exit(EXIT_FAILURE);
      }
      zlp_orderfile(lp, fp, format);
         
      check_write_ok(fp, ordfile);
         
      (void)(*closefile)(fp);
   }
   /* Write MST file 
    */
   if (write_mst)
   {
      sprintf(outpipe, filter, mstfile, "mst");

      if (verbose >= VERB_NORMAL)
         printf("Writing [%s]\n", outpipe);

      if (NULL == (fp = (*openfile)(outpipe, "w")))
      {
         fprintf(stderr, "*** Error 104: File open failed ");
         perror(mstfile);
         exit(EXIT_FAILURE);
      }
      zlp_mstfile(lp, fp, format);
         
      check_write_ok(fp, mstfile);
         
      (void)(*closefile)(fp);
   }
   /* Write Output
    */
   sprintf(outpipe, filter, outfile, "lp");

   if (verbose >= VERB_NORMAL)
      printf("Writing [%s]\n", outpipe);

   if (NULL == (fp = (*openfile)(outpipe, "w")))
   {
      fprintf(stderr, "*** Error 104: File open failed ");
      perror(outfile);
      exit(EXIT_FAILURE);
   }
   if (format != LP_FORM_RLP)
      prog_text = prog_tostr(prog, format == LP_FORM_MPS ? "* " : "\\ ", title, 128);
   else
   {
      prog_text = malloc(strlen(title) + 4);
      
      assert(prog_text != NULL);

      sprintf(prog_text, "\\%s\n", title);
   }
   zlp_write(lp, fp, format, prog_text);

   check_write_ok(fp, outfile);

   (void)(*closefile)(fp);

   /* We do not need the translation table for human readable format
    * Has to be written after the LP file, so the scaling has been done.
    */
   if (format != LP_FORM_HUM)
   {
      /* Write translation table
       */
      sprintf(outpipe, filter, tblfile, "tbl");

      if (verbose >= VERB_NORMAL)
         printf("Writing [%s]\n", outpipe);

      if (NULL == (fp = (*openfile)(outpipe, "w")))
      {
         fprintf(stderr, "*** Error 104: File open failed");
         perror(tblfile);
         exit(EXIT_FAILURE);
      }
      zlp_transtable(lp, fp, format);

      check_write_ok(fp, tblfile);

      (void)(*closefile)(fp);
   }




   free(prog_text);
   
   if (verbose >= VERB_DEBUG) 
      symbol_print_all(stderr);

#if defined(__INSURE__) || !defined(NDEBUG) || defined(FREEMEM)
   
   /* Now clean up. 
    */
   if (inppipe != NULL)
      free(inppipe);
   
   for(i = 0; i < param_count; i++)
      free(param_table[i]);
   free(param_table);
   
   prog_free(prog);

   local_exit();
   interns_exit();
   xlp_free(lp);
   mio_exit();
   symbol_exit();
   define_exit();
   set_exit();
   elem_exit();
   numb_exit();
   str_exit();
   blk_exit();
   
   free(mstfile);
   free(ordfile);
   free(outfile);
   free(tblfile);
   free(basefile);
   free(filter);
   free(outpipe);

   if (verbose >= VERB_NORMAL)
   {
      mem_display(stdout);
      stkchk_maximum(stdout);
   }
#endif /* __INSURE__ || !NDEBUG || FREEMEM */
   return 0;
}







