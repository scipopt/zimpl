%{
#ident "@(#) $Id: mmlparse.y,v 1.4 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mmlparse.y                                                    */
/*   Name....: MML Parser                                                    */
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
/*lint -e428 -e574 -e525 -e750 -e737 -e734 -e744 -e732 -e713 */
/*lint -esym(746,__yy_memcpy) -esym(516,__yy_memcpy) */
/*lint -esym(718,yylex) -esym(746,yylex) */
/*lint -esym(644,yyval,yylval) -esym(550,yynerrs) */
/*lint -esym(553,__GNUC__)  -esym(578,yylen) */
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"
#include "code.h"
#include "inst.h"
   
#define YYERROR_VERBOSE 1
   
extern void yyerror(const char* s);

%}
%pure_parser

%union
{
   double      numb;
   const char* strg;
   const char* name;
   CodeNode*   code;
};

%token DECLSET DECLPAR DECLVAR DECLMIN DECLMAX DECLSUB
%token BINARY INTEGER REAL
%token ASGN DO WITH IN FORALL EMPTY_TUPLE
%token CMP_LE CMP_GE CMP_EQ CMP_LT CMP_GT CMP_NE
%token AND OR NOT
%token SUM
%token <name> NUMBSYM
%token <name> STRGSYM
%token <name> VARSYM
%token <name> SETSYM
%token <name> NAME
%token <strg> STRG
%token <numb> NUMB

%type <code> stmt decl_set decl_par decl_var decl_obj decl_sub
%type <code> expr expr_list symidx tuple tuple_list sexpr lexpr
%type <code> idxset product factor term ineq entry entry_list
%type <code> var_type ineq_type lower upper condition;

%right ASGN
%left  ','
%left  OR
%left  AND
%left  CMP_EQ CMP_NE CMP_LE CMP_LT CMP_GE CMP_GT
%left  NOT
%left  '+' '-' 
%left  SUM
%left  '*' '/'
%left  UNARY
%%
stmt
   : decl_set   { code_set_root($1); }
   | decl_par   { code_set_root($1); }
   | decl_var   { code_set_root($1); }
   | decl_obj   { code_set_root($1); }
   | decl_sub   { code_set_root($1); }
   ;

/* ----------------------------------------------------------------------------
 * --- SET Declaration
 * ----------------------------------------------------------------------------
 */
decl_set
   : DECLSET NAME ASGN sexpr ';' {
         $$ = code_new_inst(i_newsym_set, 3,
            code_new_name($2),                                /* Name */
            code_new_inst(i_set_empty, 1, code_new_size(0)),  /* index set */
            $4);                                              /* initial */
      }
   ;

/* ----------------------------------------------------------------------------
 * --- PARAM Declaration
 * ----------------------------------------------------------------------------
 */
decl_par
   : DECLPAR NAME '[' sexpr ']' ASGN entry_list ';' {
         $$ = code_new_inst(i_newsym_numb, 3, code_new_name($2), $4, $7);
      }
   ;

/* ----------------------------------------------------------------------------
 * --- Var Declaration
 * ----------------------------------------------------------------------------
 */
decl_var
   : DECLVAR NAME '[' sexpr ']' var_type lower upper ';' {
         $$ = code_new_inst(i_newsym_var, 5,
            code_new_name($2), $4, $6, $7, $8);
      }
   ;

var_type
   : /* empty */ { $$ = code_new_vartype(VAR_CON); }
   | REAL        { $$ = code_new_vartype(VAR_CON); }
   | INTEGER     { $$ = code_new_vartype(VAR_INT); }
   | BINARY      { $$ = code_new_vartype(VAR_BIN); }
   ;

lower
   : /* empty */ { $$ = code_new_numb(0.0); }
   | CMP_GE expr { $$ = $2; }
   ;

upper
   : /* empty */ { $$ = code_new_numb(INFINITY); }
   | CMP_LE expr { $$ = $2; }
   ;

/* ----------------------------------------------------------------------------
 * --- DATA 
 * ----------------------------------------------------------------------------
 */
entry_list
   : entry                 { $$ = code_new_inst(i_entry_list_new, 1, $1); }
   | entry_list ',' entry  { $$ = code_new_inst(i_entry_list_add, 2, $1, $3); }
   ;

entry
   : tuple ',' expr       { $$ = code_new_inst(i_entry, 2, $1, $3); }
   ;

/* ----------------------------------------------------------------------------
 * --- 
 * ----------------------------------------------------------------------------
 */

decl_obj
   : DECLMIN NAME DO term ';' {
         $$ = code_new_inst(i_object_min, 2, code_new_name($2), $4);
      }
   | DECLMAX NAME DO term ';' {
         $$ = code_new_inst(i_object_max, 2, code_new_name($2), $4);
      }
   ;

/* ----------------------------------------------------------------------------
 * --- 
 * ----------------------------------------------------------------------------
 */
decl_sub
   : DECLSUB NAME DO ineq ';' {
         $$ = code_new_inst(i_once, 2, code_new_name($2), $4);
      }
   | DECLSUB NAME DO FORALL idxset DO ineq ';' {
         $$ = code_new_inst(i_forall, 3, code_new_name($2), $5, $7); 
      }
   ;

ineq 
   : term ineq_type expr { $$ = code_new_inst(i_ineq_new, 3, $1, $2, $3); }
   ;

ineq_type
   : CMP_LE  { $$ = code_new_ineqtype(INEQ_LE); }
   | CMP_GE  { $$ = code_new_ineqtype(INEQ_GE); }
   | CMP_EQ  { $$ = code_new_ineqtype(INEQ_EQ); }
   ;

term
   : term '+' product       { $$ = code_new_inst(i_term_add, 2, $1, $3); }
   | term '-' product       { $$ = code_new_inst(i_term_sub, 2, $1, $3);}
   | product                { $$ = $1; }
   ;

product
   : SUM idxset DO factor   { $$ = code_new_inst(i_term_sum, 2, $2, $4); }
   | factor                 { $$ = $1; }
   ;

factor
   : VARSYM symidx          {
         $$ = code_new_inst(i_symbol_deref, 2, code_new_name($1), $2);
      } 
   | expr '*' VARSYM symidx { 
         $$ = code_new_inst(i_term_coeff, 2,
            code_new_inst(i_symbol_deref, 2, code_new_name($3), $4), $1);
      }
   | '(' term ')'           { $$ = $2; }
   ;

idxset
   : tuple IN sexpr condition {
         $$ = code_new_inst(i_idxset_new, 3, $1, $3, $4);
      }
   ;

condition
   : /* empty */ { $$ = code_new_inst(i_bool_true, 0); }
   | WITH lexpr  { $$ = $2; }
   ;

sexpr
   : SETSYM symidx  {
         $$ = code_new_inst(i_symbol_deref, 2, code_new_name($1), $2);
      }
   | sexpr '+' sexpr    { $$ = code_new_inst(i_set_union, 2, $1, $3); }
   | sexpr '*' sexpr    { $$ = code_new_inst(i_set_cross, 2, $1, $3); }
   | '(' sexpr ')'      { $$ = $2; }
   | '{' tuple_list '}' { $$ = code_new_inst(i_set_new, 1, $2); }
   ;

tuple_list
   : tuple {
         $$ = code_new_inst(i_tuple_list_new, 1, $1);
      }
   | tuple_list ',' tuple  {
         $$ = code_new_inst(i_tuple_list_add, 2, $1, $3);
      }
   ;

lexpr
   : expr CMP_EQ expr { $$ = code_new_inst(i_bool_eq, 2, $1, $3); }
   | expr CMP_NE expr { $$ = code_new_inst(i_bool_ne, 2, $1, $3); }
   | expr CMP_GT expr { $$ = code_new_inst(i_bool_gt, 2, $1, $3); }
   | expr CMP_GE expr { $$ = code_new_inst(i_bool_ge, 2, $1, $3); }
   | expr CMP_LT expr { $$ = code_new_inst(i_bool_lt, 2, $1, $3); }
   | expr CMP_LE expr { $$ = code_new_inst(i_bool_le, 2, $1, $3); }
   | lexpr AND lexpr  { $$ = code_new_inst(i_bool_and, 2, $1, $3); }
   | lexpr OR lexpr   { $$ = code_new_inst(i_bool_or, 2, $1, $3); }
   | NOT lexpr        { $$ = code_new_inst(i_bool_not, 2, $2); }
   | '(' lexpr ')'    { $$ = $2; }
   ;
 
tuple
   : EMPTY_TUPLE             { $$ = code_new_inst(i_tuple_empty, 0); }
   | CMP_LT expr_list CMP_GT { $$ = code_new_inst(i_tuple_new, 1, $2);  }
   ;

symidx
   : /* empty */  {
         $$ = code_new_inst(i_tuple_empty, 0);
      }
   | '[' expr_list ']' {
         $$ = code_new_inst(i_tuple_new, 1, $2);
      }
   ;

expr_list
   : expr {
         $$ = code_new_inst(i_elem_list_new, 1, $1);
      }
   | expr_list ',' expr {
         $$ = code_new_inst(i_elem_list_add, 2, $1, $3);
      }
   ;

expr
   : NUMB       { $$ = code_new_numb($1); }
   | STRG       { $$ = code_new_strg($1);  }
   | NAME       {
         $$ = code_new_inst(i_local_deref, 1, code_new_name($1));
      }
   | NUMBSYM symidx { 
         $$ = code_new_inst(i_symbol_deref, 2, code_new_name($1), $2);
      }
   | STRGSYM symidx { 
         $$ = code_new_inst(i_symbol_deref, 2, code_new_name($1), $2);
      }
   | expr '+' expr         { $$ = code_new_inst(i_expr_add, 2, $1, $3); }
   | expr '-' expr         { $$ = code_new_inst(i_expr_sub, 2, $1, $3); }
   | expr '*' expr         { $$ = code_new_inst(i_expr_mul, 2, $1, $3); }
   | expr '/' expr         { $$ = code_new_inst(i_expr_div, 2, $1, $3); }
   | '+' expr %prec UNARY  { $$ = $2; }
   | '-' expr %prec UNARY  { $$ = code_new_inst(i_expr_neg, 1, $2); }
   | '(' expr ')'          { $$ = $2; }
   ;

