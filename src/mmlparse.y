%{
#ident "@(#) $Id: mmlparse.y,v 1.25 2002/10/31 09:28:55 bzfkocht Exp $"
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
/*lint -esym(768,bits) */ 
   
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
   unsigned int bits;
   double       numb;
   const char*  strg;
   const char*  name;
   CodeNode*    code;
};

%token DECLSET DECLPAR DECLVAR DECLMIN DECLMAX DECLSUB PRINT
%token BINARY INTEGER REAL
%token ASGN DO WITH IN TO BY FORALL EMPTY_TUPLE EXISTS
%token PRIORITY STARTVAL
%token CMP_LE CMP_GE CMP_EQ CMP_LT CMP_GT CMP_NE
%token AND OR NOT
%token SUM MIN MAX
%token IF THEN ELSE END
%token INTER UNION CROSS SYMDIFF WITHOUT PROJ
%token MOD DIV POW
%token CARD ABS FLOOR CEIL LOG LN EXP
%token READ AS SKIP USE COMMENT
%token SUBSETS INDEXSET POWERSET
%token <name> NUMBSYM
%token <name> STRGSYM
%token <name> VARSYM
%token <name> SETSYM
%token <name> NAME
%token <strg> STRG
%token <numb> NUMB
%token <bits> SCALE
%token <bits> SEPARATE

%type <code> stmt decl_set decl_par decl_var decl_obj decl_sub
%type <code> stmt_print
%type <code> constraint
%type <code> expr expr_list symidx tuple tuple_list sexpr lexpr read read_par
%type <code> idxset subterm summand factor vexpr term
%type <code> expr_entry expr_entry_list set_entry set_entry_list
%type <code> var_type con_type lower upper priority startval condition
%type <bits> con_attr con_attr_list

%right ASGN
%left  TERMOP
%left  ','
%right '('
%left  ')'
%left  OR
%left  EXISTS
%left  AND
%left  CMP_EQ CMP_NE CMP_LE CMP_LT CMP_GE CMP_GT
%left  IN
%left  NOT
%left  UNION WITHOUT SYMDIFF
%left  INTER
%left  CROSS
%left  '+' '-' 
%left  SUM MIN MAX
%left  '*' '/' MOD DIV
%left  ABS CARD FLOOR CEIL PROJ LOG LN EXP
%left  UNARY
%left  POW
%left  FAC
%%
stmt
   : decl_set   { code_set_root($1); }
   | decl_par   { code_set_root($1); }
   | decl_var   { code_set_root($1); }
   | decl_obj   { code_set_root($1); }
   | decl_sub   { code_set_root($1); }
   | stmt_print { code_set_root($1); }
   ;

/* ----------------------------------------------------------------------------
 * --- SET Declaration
 * ----------------------------------------------------------------------------
 */
decl_set
   : DECLSET NAME ASGN idxset ';' {
         $$ = code_new_inst(i_newsym_set1, 3,
            code_new_name($2),                                /* Name */
            code_new_inst(i_set_empty, 1, code_new_size(0)),  /* index set */
            $4);                                              /* initial */
      }
   | DECLSET NAME '[' idxset ']' ASGN set_entry_list ';' {
         $$ = code_new_inst(i_newsym_set2, 3,
            code_new_name($2),                                       /* Name */
            $4,                                                 /* index set */
            $7);                                   /* initial set_entry_list */
      }
   | DECLSET NAME '[' ']' ASGN set_entry_list ';' {
         $$ = code_new_inst(i_newsym_set2, 3,
            code_new_name($2),                                       /* Name */
            code_new_inst(i_idxset_new, 3,                      /* index set */
               code_new_inst(i_tuple_empty, 0),
               code_new_inst(i_set_empty, 1, code_new_size(0)),
               code_new_inst(i_bool_true, 0)),
            $6);                                   /* initial set_entry_list */
      }
   ;

set_entry_list
   : set_entry             { $$ = code_new_inst(i_entry_list_new, 1, $1); }
   | set_entry_list ',' set_entry  {
         $$ = code_new_inst(i_entry_list_add, 2, $1, $3);
      }
   | SUBSETS '(' sexpr ',' expr ')' {
         $$ = code_new_inst(i_entry_list_subsets, 2, $3, $5);
      }
   | POWERSET '(' sexpr ')' {
         $$ = code_new_inst(i_entry_list_powerset, 1, $3);
      }
   ;

set_entry
   : tuple sexpr           { $$ = code_new_inst(i_entry, 2, $1, $2); }
   ;


/* ----------------------------------------------------------------------------
 * --- PARAM Declaration
 * ----------------------------------------------------------------------------
 */
decl_par
   : DECLPAR NAME '[' idxset ']' ASGN expr_entry_list ';' {
         $$ = code_new_inst(i_newsym_para1, 3, code_new_name($2), $4, $7);
      }
   | DECLPAR NAME '[' idxset ']' ASGN expr ';' {
         $$ = code_new_inst(i_newsym_para2, 3, code_new_name($2), $4, $7);
      }
   | DECLPAR NAME ASGN expr ';' {
         $$ = code_new_inst(i_newsym_para1, 3,
            code_new_name($2),
            code_new_inst(i_idxset_new, 3,
               code_new_inst(i_tuple_empty, 0),
               code_new_inst(i_set_empty, 1, code_new_size(0)),
               code_new_inst(i_bool_true, 0)),
            code_new_inst(i_entry_list_new, 1,
               code_new_inst(i_entry, 2,
                  code_new_inst(i_tuple_empty, 0),
                  $4)));
      }
   ;

/* ----------------------------------------------------------------------------
 * --- Var Declaration
 * ----------------------------------------------------------------------------
 */
decl_var
   : DECLVAR NAME '[' idxset ']' var_type lower upper priority startval ';' {
         $$ = code_new_inst(i_newsym_var, 7,
            code_new_name($2), $4, $6, $7, $8, $9, $10);
      }
   | DECLVAR NAME var_type lower upper priority startval ';' {
         $$ = code_new_inst(i_newsym_var, 7,
            code_new_name($2),
            code_new_inst(i_idxset_new, 3,
               code_new_inst(i_tuple_empty, 0),
               code_new_inst(i_set_empty, 1, code_new_size(0)),
               code_new_inst(i_bool_true, 0)),
            $3, $4, $5, $6, $7);
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

priority
   : /* empty */   { $$ = code_new_numb(0.0); }
   | PRIORITY expr { $$ = $2; }
   ;

startval
   : /* empty */   { $$ = code_new_numb(INFINITY); }
   | STARTVAL expr { $$ = $2; }
   ;

/* ----------------------------------------------------------------------------
 * --- DATA 
 * ----------------------------------------------------------------------------
 */
expr_entry_list
   : expr_entry            { $$ = code_new_inst(i_entry_list_new, 1, $1); }
   | expr_entry_list ',' expr_entry  {
         $$ = code_new_inst(i_entry_list_add, 2, $1, $3);
      }
   | read                  { $$ = code_new_inst(i_read, 1, $1); } 
   ;

expr_entry
   : tuple expr            { $$ = code_new_inst(i_entry, 2, $1, $2); }
   ;

/* ----------------------------------------------------------------------------
 * --- Objective Declaration
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
 * --- Subto Declaration
 * ----------------------------------------------------------------------------
 */
decl_sub
   : DECLSUB NAME DO constraint ';' {
        $$ = code_new_inst(i_subto, 2, code_new_name($2), $4);
     }
   ;

constraint
   : term con_type term con_attr_list {
        $$ = code_new_inst(i_constraint, 4, $1, $2, $3, code_new_bits($4));
     }   
   | FORALL idxset DO constraint {
        $$ = code_new_inst(i_forall, 2, $2, $4);
     }
   ;
/*
   : term con_type expr con_attr_list {
        $$ = code_new_inst(i_constraint, 4, $1, $2, $3, code_new_bits($4));
     }   

   : DECLSUB NAME DO term con_type expr con_attr_list ';' {
         $$ = code_new_inst(i_once, 5, code_new_name($2),
            $4, $5, $6, code_new_bits($7));
      }
   | DECLSUB NAME DO FORALL idxset DO term con_type expr con_attr_list ';' {
         $$ = code_new_inst(i_forall, 6, code_new_name($2),
            $5, $7, $8, $9, code_new_bits($10)); 
      }
   ;
*/

con_attr_list
   : /* empty */                { $$ = 0; }
   | con_attr_list ',' con_attr { $$ = $1 | $3; }
   ;

con_attr
   : SCALE     { $$ = LP_FLAG_CON_SCALE; }
   | SEPARATE  { $$ = LP_FLAG_CON_SEPAR; }
   ;

con_type
   : CMP_LE  { $$ = code_new_contype(CON_LE); }
   | CMP_GE  { $$ = code_new_contype(CON_GE); }
   | CMP_EQ  { $$ = code_new_contype(CON_EQ); }
   ;

term
   : term '+' subterm       { $$ = code_new_inst(i_term_add, 2, $1, $3); }
   | term '-' subterm       { $$ = code_new_inst(i_term_sub, 2, $1, $3); }
   | subterm                { $$ = $1; }
   ;

subterm
   : summand                { $$ = $1; }
   | expr %prec TERMOP      { $$ = code_new_inst(i_term_expr, 1, $1); }
   | expr '+' summand       { $$ = code_new_inst(i_term_const, 2, $3, $1); }
   | expr '-' summand       {
         $$ = code_new_inst(i_term_sub, 2,
            code_new_inst(i_term_expr, 1, $1),
            $3);
      }
   ;

summand
   : factor                 { $$ = $1; }
   | factor '*' expr       { $$ = code_new_inst(i_term_coeff, 2, $1, $3); }
   | factor '/' expr       {
         $$ = code_new_inst(i_term_coeff, 2, $1,
            code_new_inst(i_expr_div, 2, code_new_numb(1.0), $3));
      }
   | SUM idxset DO summand %prec SUM {
         $$ = code_new_inst(i_term_sum, 2, $2, $4);
      }
   | expr '*' SUM idxset DO summand %prec SUM {
         $$ = code_new_inst(i_term_coeff, 2, 
            code_new_inst(i_term_sum, 2, $4, $6),
            $1);
      }
   ;

factor
   : vexpr                  { $$ = $1; }
   | expr '*' vexpr         { $$ = code_new_inst(i_term_coeff, 2, $3, $1); }
   | '(' term ')'           { $$ = $2; }   
   | expr '*' '(' term ')'  { $$ = code_new_inst(i_term_coeff, 2, $4, $1); }
   ;

vexpr
   : VARSYM symidx          {
         $$ = code_new_inst(i_symbol_deref, 2, code_new_name($1), $2);
      } 
   | '+' VARSYM symidx %prec UNARY  {
         $$ = code_new_inst(i_symbol_deref, 2, code_new_name($2), $3);
      } 
   | '-' VARSYM symidx %prec UNARY  { 
         $$ = code_new_inst(i_term_coeff, 2,
            code_new_inst(i_symbol_deref, 2, code_new_name($2), $3),
            code_new_numb(-1.0));
      } 
   ;   

/* ----------------------------------------------------------------------------
 * --- Print Statement
 * ----------------------------------------------------------------------------
 */
stmt_print
   : PRINT expr ';'    { $$ = code_new_inst(i_print, 1, $2); }
   | PRINT tuple ';'   { $$ = code_new_inst(i_print, 1, $2); }
   | PRINT sexpr ';'   { $$ = code_new_inst(i_print, 1, $2); }
   | PRINT SETSYM ';'  { $$ = code_new_inst(i_print, 1, code_new_name($2)); }
   ;


/* ----------------------------------------------------------------------------
 * --- 
 * ----------------------------------------------------------------------------
 */
idxset
   : tuple IN sexpr condition {
         $$ = code_new_inst(i_idxset_new, 3, $1, $3, $4);
      }
   | sexpr condition {
         $$ = code_new_inst(i_idxset_new, 3,
            code_new_inst(i_tuple_empty, 0), $1, $2);
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
   | '{' expr TO expr BY expr '}' {
         $$ = code_new_inst(i_set_range, 3, $2, $4, $6);
      }
   | '{' expr TO expr '}' {
         $$ = code_new_inst(i_set_range, 3, $2, $4, code_new_numb(1.0));
      }
   | sexpr UNION sexpr  { $$ = code_new_inst(i_set_union, 2, $1, $3); }
   | sexpr '+' sexpr %prec UNION {
         $$ = code_new_inst(i_set_union, 2, $1, $3);
      }
   | sexpr SYMDIFF sexpr  { $$ = code_new_inst(i_set_sdiff, 2, $1, $3); }
   | sexpr WITHOUT sexpr  { $$ = code_new_inst(i_set_minus, 2, $1, $3); }
   | sexpr '-' sexpr %prec WITHOUT {
         $$ = code_new_inst(i_set_minus, 2, $1, $3);
      }
   | sexpr CROSS   sexpr  { $$ = code_new_inst(i_set_cross, 2, $1, $3); }
   | sexpr '*' sexpr {
         $$ = code_new_inst(i_set_cross, 2, $1, $3);
      }
   | sexpr INTER   sexpr  { $$ = code_new_inst(i_set_inter, 2, $1, $3); }
   | '(' sexpr ')'      { $$ = $2; }
   | '{' tuple_list '}' { $$ = code_new_inst(i_set_new_tuple, 1, $2); }
   | '{' expr_list '}'  { $$ = code_new_inst(i_set_new_elem, 1, $2); }
   | PROJ '(' sexpr ',' tuple ')' {
         $$ = code_new_inst(i_set_proj, 2, $3, $5);
      }
   | INDEXSET '(' SETSYM ')' {
         $$ = code_new_inst(i_set_indexset, 1, code_new_name($3));
      }
   ;

read
   : READ expr AS expr { $$ = code_new_inst(i_read_new, 2, $2, $4); }
   | read read_par     { $$ = code_new_inst(i_read_param, 2, $1, $2); }
   ;

read_par
   : SKIP expr    { $$ = code_new_inst(i_read_skip, 1, $2); }
   | USE expr     { $$ = code_new_inst(i_read_use, 1, $2); }
   | COMMENT expr { $$ = code_new_inst(i_read_comment, 1, $2); }
   ;
 
tuple_list
   : tuple {
         $$ = code_new_inst(i_tuple_list_new, 1, $1);
      }
   | tuple_list ',' tuple  {
         $$ = code_new_inst(i_tuple_list_add, 2, $1, $3);
      }
   | read { $$ = code_new_inst(i_read, 1, $1); }
   ;

lexpr
   : expr CMP_EQ expr   { $$ = code_new_inst(i_bool_eq, 2, $1, $3); }
   | expr CMP_NE expr   { $$ = code_new_inst(i_bool_ne, 2, $1, $3); }
   | expr CMP_GT expr   { $$ = code_new_inst(i_bool_gt, 2, $1, $3); }
   | expr CMP_GE expr   { $$ = code_new_inst(i_bool_ge, 2, $1, $3); }
   | expr CMP_LT expr   { $$ = code_new_inst(i_bool_lt, 2, $1, $3); }
   | expr CMP_LE expr   { $$ = code_new_inst(i_bool_le, 2, $1, $3); }
   | sexpr CMP_EQ sexpr { $$ = code_new_inst(i_bool_seq, 2, $1, $3); }
   | sexpr CMP_NE sexpr { $$ = code_new_inst(i_bool_sneq, 2, $1, $3); }
   | sexpr CMP_GT sexpr { $$ = code_new_inst(i_bool_subs, 2, $3, $1); }
   | sexpr CMP_GE sexpr { $$ = code_new_inst(i_bool_sseq, 2, $3, $1); }
   | sexpr CMP_LT sexpr { $$ = code_new_inst(i_bool_subs, 2, $1, $3); }
   | sexpr CMP_LE sexpr { $$ = code_new_inst(i_bool_sseq, 2, $1, $3); }
   | lexpr AND lexpr    { $$ = code_new_inst(i_bool_and, 2, $1, $3); }
   | lexpr OR lexpr     { $$ = code_new_inst(i_bool_or, 2, $1, $3); }
   | NOT lexpr          { $$ = code_new_inst(i_bool_not, 2, $2); }
   | '(' lexpr ')'      { $$ = $2; }
   | tuple IN sexpr     { $$ = code_new_inst(i_bool_is_elem, 2, $1, $3); } 
/*   | EXISTS idxset %prec EXISTS { $$ = code_new_inst(i_bool_exists, 1, $2); }*/
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
   | expr MOD expr         { $$ = code_new_inst(i_expr_mod, 2, $1, $3); }
   | expr DIV expr         { $$ = code_new_inst(i_expr_intdiv, 2, $1, $3); }
   | expr POW expr         { $$ = code_new_inst(i_expr_pow, 2, $1, $3); }
   | expr FAC              { $$ = code_new_inst(i_expr_fac, 1, $1); }
   | CARD '(' sexpr ')'    { $$ = code_new_inst(i_expr_card, 1, $3); }
   | ABS '(' expr ')'      { $$ = code_new_inst(i_expr_abs, 1, $3); }
   | FLOOR '(' expr ')'    { $$ = code_new_inst(i_expr_floor, 1, $3); }
   | CEIL '(' expr ')'     { $$ = code_new_inst(i_expr_ceil, 1, $3); }
   | LOG '(' expr ')'      { $$ = code_new_inst(i_expr_log, 1, $3); }
   | LN '(' expr ')'       { $$ = code_new_inst(i_expr_ln, 1, $3); }
   | EXP '(' expr ')'      { $$ = code_new_inst(i_expr_exp, 1, $3); }
   | '+' expr %prec UNARY  { $$ = $2; }
   | '-' expr %prec UNARY  { $$ = code_new_inst(i_expr_neg, 1, $2); }
   | '(' expr ')'          { $$ = $2; }
   | IF lexpr THEN expr ELSE expr END {
         $$ = code_new_inst(i_expr_if, 3, $2, $4, $6);
      }
   | MIN idxset DO expr %prec MIN {
         $$ = code_new_inst(i_expr_min, 2, $2, $4);
      }
   | MAX idxset DO expr %prec MAX {
         $$ = code_new_inst(i_expr_max, 2, $2, $4);
      }
   | SUM idxset DO expr %prec SUM {
         $$ = code_new_inst(i_expr_sum, 2, $2, $4);
      }   
   ;








