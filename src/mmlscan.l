/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mmlscan.l                                                     */
/*   Name....: MML Lexcal Analyser                                           */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
%{   
#ident "@(#) $Id: mmlscan.l,v 1.3 2001/01/30 08:23:46 thor Exp $"

#include <ctype.h>
#include <assert.h>
#include <string.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"
#include "mmlparse.h"

extern void yyerror(const char* s);

#define YY_DECL int yylex(YYSTYPE* lval)

YY_DECL;

/*lint -e506 -e525 -e528 -e527 -e551 -e553 -e633 */
/*lint -e713 -e716 -e725 -e732 -e734 -e737 -e744 -e774 -e750 */
/*lint -esym(613,b,buf) -esym(563,find_rule) */
/*lint -esym(773,BEGIN) -esym(760,FLEX_DEBUG) */

static int yycolumn = 0;
%}
%option noyylineno
%option never-interactive
%option noyywrap
%option noyymore
%option noreject
%option nounput
%pointer
%%
":="                      { yycolumn += yyleng; return ASGN; }
"=="                      { yycolumn += yyleng; return CMP_EQ; }
"<="                      { yycolumn += yyleng; return CMP_LE; }
">="                      { yycolumn += yyleng; return CMP_GE; }
"set"                     { yycolumn += yyleng; return DECLSET; }
"param"                   { yycolumn += yyleng; return DECLPAR; }
"var"                     { yycolumn += yyleng; return DECLVAR; }
"maximize"                { yycolumn += yyleng; return DECLMAX; }
"minimize"                { yycolumn += yyleng; return DECLMIN; }
"subto"                   { yycolumn += yyleng; return DECLSUB; }
"in"                      { yycolumn += yyleng; return IN; }
"with"|"|"                { yycolumn += yyleng; return WITH; }
"do"|":"                  { yycolumn += yyleng; return DO; }
"binary"                  { yycolumn += yyleng; return BINARY; }
"integer"                 { yycolumn += yyleng; return INTEGER; }
"real"                    { yycolumn += yyleng; return REAL; }
"<>"                      { yycolumn += yyleng; return EMPTY_TUPLE; }
"sum"                     { yycolumn += yyleng; return SUM; }
"forall"                  { yycolumn += yyleng; return FORALL; }
[0-9]+("."[0-9]+)?([eEdD][-+]?[0-9]+)?  { 
      yycolumn += yyleng; 
      sscanf(yytext, "%lf", &lval->numb);
      return NUMB; 
   }
"."[0-9]+([eEdD][-+]-?[0-9]+)?  { 
      yycolumn += yyleng; 
      sscanf(yytext, "%lf", &lval->numb);
      return NUMB; 
   }

[A-Za-z_][A-Za-z0-9_]* {
      Symbol* sym;

      yycolumn += yyleng; 

      lval->name = str_new(yytext);

      if (NULL != (sym = symbol_lookup(yytext)))
      {
         switch(symbol_get_type(sym))
         {
         case SYM_NUMB :
            return NUMBSYM;
         case SYM_STRG :
            return STRGSYM;
         case SYM_VAR :
            return VARSYM;
         case SYM_SET :
            return SETSYM;
         default :
            assert(0);
         }
      }
      return NAME;
   }

\"[^\"]*\" { 
      yycolumn += yyleng; 
      yytext[strlen(yytext) - 1] = '\0';
      lval->strg = str_new(yytext + 1); 
      return STRG; 
   }
\'[^\']*\' { 
      yycolumn += yyleng; 
      yytext[strlen(yytext) - 1] = '\0';
      lval->strg = str_new(yytext + 1); 
      return STRG; 
   }

[[:space:]]+              { yycolumn += yyleng; }

.                         { yycolumn += yyleng; return *yytext; }

<<EOF>>                   { yycolumn += yyleng; return EOF; }

%%
static int yylineno = -1;

void yyerror(const char* s)
{
   fprintf(stderr, "Error line: %d Col: %d %s\n", yylineno, yycolumn, s);
   exit(-1);
}

int scan_get_lineno(void)
{
   return yylineno;
}

int scan_get_column(void)
{
   return yycolumn;
}

int parse_string(const char* s, int lineno)
{
   YY_BUFFER_STATE state;
   int             ret;

   yy_delete_buffer(YY_CURRENT_BUFFER);

   yylineno = lineno;
   yycolumn = 0;
   state    = yy_scan_string(s);

   ret = yyparse();

   yy_delete_buffer(state);

   return ret;
}
