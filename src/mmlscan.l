/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mmlscan.l                                                     */
/*   Name....: MML Lexcal Analyser                                           */
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
%{   
#pragma ident "@(#) $Id: mmlscan.l,v 1.21 2003/03/18 11:47:59 bzfkocht Exp $"

#include <ctype.h>
#include <assert.h>
#include <string.h>

#include "portab.h"
/* Leads to problems with stdlib.h - reason? */
/* #include "mshell.h" */
#include "mme.h"
#include "mmlparse.h"

extern void yyerror(const char* s);

#define YY_DECL int yylex(YYSTYPE* lval)

YY_DECL;

static int yycolumn = 0;

/*lint -e506 -e525 -e528 -e527 -e551 -e553 -e633 */
/*lint -e713 -e716 -e725 -e732 -e734 -e737 -e744 -e774 -e750 */
/*lint -e825 */
/*lint -esym(613,b,buf) -esym(563,find_rule) */
/*lint -esym(773,BEGIN) -esym(760,FLEX_DEBUG) */
%}
%option noyylineno
%option never-interactive
%option noyywrap
%option noyymore
%option noreject
%option nounput
%pointer
%%
"**"|"^"                  { yycolumn += yyleng; return POW; }
":="                      { yycolumn += yyleng; return ASGN; }
"=="                      { yycolumn += yyleng; return CMP_EQ; }
"<="                      { yycolumn += yyleng; return CMP_LE; }
">="                      { yycolumn += yyleng; return CMP_GE; }
"<"                       { yycolumn += yyleng; return CMP_LT; }
">"                       { yycolumn += yyleng; return CMP_GT; }
"!="                      { yycolumn += yyleng; return CMP_NE; }
"!"                       { yycolumn += yyleng; return FAC; }
"not"                     { yycolumn += yyleng; return NOT; }
"and"                     { yycolumn += yyleng; return AND; }
"or"                      { yycolumn += yyleng; return OR; }
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
"exists"                  { yycolumn += yyleng; return EXISTS; }
"if"                      { yycolumn += yyleng; return IF; }
"then"                    { yycolumn += yyleng; return THEN; }
"else"                    { yycolumn += yyleng; return ELSE; }
"end"                     { yycolumn += yyleng; return END; }
"to"|".."                 { yycolumn += yyleng; return TO; }
"by"                      { yycolumn += yyleng; return BY; }
"union"                   { yycolumn += yyleng; return UNION; }
"inter"                   { yycolumn += yyleng; return INTER; }
"symdiff"                 { yycolumn += yyleng; return SYMDIFF; }
"cross"                   { yycolumn += yyleng; return CROSS; }
"proj"                    { yycolumn += yyleng; return PROJ; }
"without"|"\\"            { yycolumn += yyleng; return WITHOUT; }
"mod"|"modulo"            { yycolumn += yyleng; return MOD; }
"div"                     { yycolumn += yyleng; return DIV; }
"min"                     { yycolumn += yyleng; return MIN; }
"max"                     { yycolumn += yyleng; return MAX; }
"read"                    { yycolumn += yyleng; return READ; }
"as"                      { yycolumn += yyleng; return AS; }
"skip"                    { yycolumn += yyleng; return SKIP; }
"use"                     { yycolumn += yyleng; return USE; }
"comment"                 { yycolumn += yyleng; return COMMENT; }
"scale"                   { yycolumn += yyleng; return SCALE; }
"separate"                { yycolumn += yyleng; return SEPARATE; }
"card"                    { yycolumn += yyleng; return CARD; }
"abs"                     { yycolumn += yyleng; return ABS; }
"floor"                   { yycolumn += yyleng; return FLOOR; }
"ceil"                    { yycolumn += yyleng; return CEIL; }
"log"                     { yycolumn += yyleng; return LOG; }
"ln"                      { yycolumn += yyleng; return LN; }
"exp"                     { yycolumn += yyleng; return EXP; }
"priority"                { yycolumn += yyleng; return PRIORITY; }
"startval"                { yycolumn += yyleng; return STARTVAL; }
"default"                 { yycolumn += yyleng; return DEFAULT; }
"subsets"                 { yycolumn += yyleng; return SUBSETS; }
"powerset"                { yycolumn += yyleng; return POWERSET; }
"indexset"                { yycolumn += yyleng; return INDEXSET; }
"print"                   { yycolumn += yyleng; return PRINT; }
"infinity"                { yycolumn += yyleng; return INFTY; }
"random"                  { yycolumn += yyleng; return RANDOM; }

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

      if (NULL != (sym = symbol_lookup(yytext)))
      {
         lval->sym = sym;

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
            abort();
         }
      }
      lval->name = str_new(yytext);

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
static const Stmt* yystmt = NULL;

void yyerror(const char* s)
{
   fprintf(stderr, "*** File %s Line %d : %s\n", 
      stmt_get_filename(yystmt), stmt_get_lineno(yystmt), s);
   show_source(stderr, stmt_get_text(yystmt), yycolumn);
   abort();
}

const Stmt* scan_get_stmt(void)
{
   return yystmt;
}

int scan_get_column(void)
{
   return yycolumn;
}

void parse_stmt(const Stmt* stmt)
{
   YY_BUFFER_STATE state;

   yy_delete_buffer(YY_CURRENT_BUFFER);

   yystmt   = stmt;
   yycolumn = 0;
   state    = yy_scan_string(stmt_get_text(stmt));

   if (yyparse() != 0)
   {
      fprintf(stderr, "*** Parsing error\n");
      exit(1);
   }
   yy_delete_buffer(state);
}
