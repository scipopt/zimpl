#ident "$Id: zimpl.c,v 1.1 2001/01/26 07:11:37 thor Exp $"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"

extern int yydebug;
extern int yy_flex_debug;

int verbose = 0;

int main(int argc, char** argv)
{
   const char* usage = "usage: %s filename\n";
   
   Prog* prog;
   int   c;

   yydebug       = 0;
   yy_flex_debug = 0;

   while((c = getopt(argc, argv, "bfv")) != -1)
   {
      switch(c)
      {
      case 'b' :
         yydebug = 1;
         break;
      case 'f' :
         yy_flex_debug = 1;
         break;
      case 'v' :
         verbose = 1;
         break;
      case '?':
         fprintf(stderr, usage, argv[0]);
         exit(0);
      default :
         assert(0);
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
   
   if (!prog_load(prog))
   {
      if (verbose)
         prog_print(stderr, prog);
      
      (void)prog_execute(prog);
      
      prog_free(prog);
   }
   if (verbose)
      symbol_print_all(stderr);
   
   symbol_exit();
   elem_exit();
   str_exit();
   
   mem_display(stderr);
   
   return 0;
}


