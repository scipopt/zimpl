#pragma ident "$Id: zimpl.c,v 1.49 2003/10/25 09:39:04 bzfkocht Exp $"
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

int verbose = VERB_NORMAL;

static const char* banner = 
"****************************************************\n" \
"* Zuse Institute Mathematical Programming Language *\n" \
"* Release 2.01b Copyright (C)2003 by Thorsten Koch *\n" \
"****************************************************\n" \
"*   This is free software and you are welcome to   *\n" \
"*     redistribute it under certain conditions     *\n" \
"*      ZIMPL comes with ABSOLUTELY NO WARRANTY     *\n" \
"****************************************************\n";

static const char* options = "bD:fF:hn:o:prt:v:";
static const char* usage =
"usage: %s [-bfhpr][-D name=value][-F filter][-n cs|cn|cf][-o outfile][-t lp|mps|hum][-v 0-5] filename\n";

static const char* help =
"\n" \
"  -b             enable bison debugging output.\n" \
"  -D name=value  Setzt einen Parameter auf einen Wert.\n" \
"  -f             enable flex debugging output.\n" \
"  -F filter      filter output, for example \"gzip -c >%%s.gz\"\n" \
"  -h             show this help.\n" \
"  -n cm|cn|cf    name column make/name/full\n" \
"  -o outfile     select name for the output file. Default is the name of\n" \
"                 the input file without extension.\n" \
"  -p             presolve LP.\n" \
"  -r             write branching order file.\n" \
"  -t lp|mps|hum  select output format. Either LP (default), MPS format\n" \
"                 or human readable HUM.\n" \
"  -v[0-5]        verbosity level: 0 = quiet, 1 = default, up to 5 = debug\n" \
"  filename       is the name of the input ZPL file.\n" \
"\n" ; 

static char* add_extention(const char* filename, const char* extension)
{
   char* basename;
   
   assert(filename != NULL);
   assert(strlen(filename) > 0);
   assert(extension != NULL);

   basename = malloc(strlen(filename) + strlen(extension) + 1);

   assert(basename != NULL);

   strcpy(basename, filename);
   
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

   for(i = strlen(filename) - 1; i >= 0; i--)
      if (filename[i] == DIRSEP || filename[i] == '.')
         break;

   if (i >= 0 && filename[i] == '.')
      filename[i] = '\0';

   i = strlen(filename);
   
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

static void add_parameter(const char* def)
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
   set   = set_new(0, 1, SET_NO_HASH);
   set_add_member(set, tuple_new(0), SET_ADD_END, SET_CHECK_NONE);

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

int main(int argc, char* const* argv)
{
   Prog*       prog;
   Set*        set;
   const char* extension;
   char*       filter   = strdup("%s");
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
   FILE*       (*openfile)(const char*, const char*) = fopen;
   int         (*closefile)(FILE*)                   = fclose;

   yydebug       = 0;
   yy_flex_debug = 0;

   while((c = getopt(argc, argv, options)) != -1)
   {
      switch(c)
      {
      case 'b' :
         yydebug = 1;
         break;
      case 'D' :
         break;
      case 'h' :
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
         default :
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

   if (verbose >= VERB_NORMAL)
      puts(banner);
   
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
   default :
      abort();
   }
   outfile = add_extention(basefile, extension);
   tblfile = add_extention(basefile, ".tbl");
   ordfile = add_extention(basefile, ".ord");
   
   cmdpipe = malloc(strlen(basefile) + strlen(filter) + 256);

   assert(cmdpipe != NULL);

   gmp_init(verbose >= VERB_VERBOSE);
   str_init();
   numb_init();
   elem_init();

   /* Make symbol to hold entries of internal variables
    */
   set = set_new(0, 0, SET_NO_HASH);
   (void)symbol_new("@@", SYM_VAR, set, 100, NULL);
   set_free(set);
   
   /* Do it again Sam, to get the defines from the command line
    */
   optind = 0;
   while((c = getopt(argc, argv, options)) != -1)
   {
      switch(c)
      {
      case 'D' :
         add_parameter(optarg);
         break;
      default :
         break;
      }
   }

   prog = prog_new();

   for(i = optind; i < argc; i++)
      prog_load(prog, argv[i]);

   if (prog_is_empty(prog))
   {
      fprintf(stderr, "*** Error 168: No program statements to execute\n");
      exit(EXIT_FAILURE);
   }
   if (verbose >= VERB_DEBUG)
      prog_print(stderr, prog);
   
   xlp_alloc(argv[optind]);

   prog_execute(prog);

   /* Presolve
    */
   if (presolve)
      xlp_presolve();

   if (verbose >= VERB_NORMAL)
      xlp_stat();
   
   xlp_scale();
   
   /* Write Output
    */
   sprintf(cmdpipe, filter, outfile);

   if (verbose >= VERB_NORMAL)
      printf("Writing [%s]\n", cmdpipe);

   if (NULL == (fp = (*openfile)(cmdpipe, "w")))
   {
      fprintf(stderr, "*** Error 104: File open failed ");
      perror(outfile);
      exit(EXIT_FAILURE);
   }
   xlp_write(fp, format);

   check_write_ok(fp, outfile);

   (void)(*closefile)(fp);

   /* We do not need the translation table for human readable format
    * (And the orderfile is also senseless)
    */
   if (format != LP_FORM_HUM)
   {
      /* Write translation table
       */
      sprintf(cmdpipe, filter, tblfile);

      if (verbose >= VERB_NORMAL)
         printf("Writing [%s]\n", cmdpipe);

      if (NULL == (fp = (*openfile)(cmdpipe, "w")))
      {
         fprintf(stderr, "*** Error 104: File open failed");
         perror(tblfile);
         exit(EXIT_FAILURE);
      }
      xlp_transtable(fp, format);

      check_write_ok(fp, tblfile);

      (void)(*closefile)(fp);

      /* Write order file 
       */
      if (write_order)
      {
         sprintf(cmdpipe, filter, ordfile);

         if (verbose >= VERB_NORMAL)
            printf("Writing [%s]\n", cmdpipe);

         if (NULL == (fp = (*openfile)(cmdpipe, "w")))
         {
            fprintf(stderr, "*** Error 104: File open failed ");
            perror(ordfile);
            exit(EXIT_FAILURE);
         }
         xlp_orderfile(fp, format);
         
         check_write_ok(fp, ordfile);
         
         (void)(*closefile)(fp);
      }
   }  
   if (verbose >= VERB_DEBUG) 
      symbol_print_all(stderr);

#if defined(__INSURE__) || !defined(NDEBUG) || defined(FREEMEM)
   
   /* Now clean up. 
    */
   prog_free(prog);

   xlp_free();
   symbol_exit();
   define_exit();
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


