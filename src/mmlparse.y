%{
#pragma ident "@(#) $Id: mmlparse.y,v 1.49 2003/09/18 11:55:49 bzfkocht Exp $"
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
/*lint -e428 -e574 -e525 -e527 -e661 -e662 -e676 */
/*lint -e713 -e717 -e732 -e734 -e737 -e744 -e750 -e764 */
/*lint -esym(746,__yy_memcpy) -esym(516,__yy_memcpy) */
/*lint -esym(718,yylex) -esym(746,yylex) */
/*lint -esym(644,yyval,yylval) -esym(550,yynerrs) */
/*lint -esym(553,__GNUC__)  -esym(578,yylen) */
/*lint -esym(768,bits) -esym(553,YYSTACK_USE_ALLOCA) */ 
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "code.h"
#include "inst.h"
   
#define YYERROR_VERBOSE 1

/*lint -sem(yyerror, 1p && nulterm(1), r_no) */ 
extern void yyerror(const char* s);
 
%}
%pure_parser

%union
{
   unsigned int bits;
   Numb*        numb;
   const char*  strg;
   const char*  name;
   Symbol*      sym;
   Define*      def;
   CodeNode*    code;
};

%token DECLSET DECLPAR DECLVAR DECLMIN DECLMAX DECLSUB
%token DEFNUMB DEFSTRG DEFSET PRINT
%token BINARY INTEGER REAL
%token ASGN DO WITH IN TO BY FORALL EMPTY_TUPLE EMPTY_SET EXISTS
%token PRIORITY STARTVAL DEFAULT
%token CMP_LE CMP_GE CMP_EQ CMP_LT CMP_GT CMP_NE INFTY
%token AND OR NOT
%token SUM MIN MAX
%token IF THEN ELSE END
%token INTER UNION CROSS SYMDIFF WITHOUT PROJ
%token MOD DIV POW
%token CARD ABS SGN FLOOR CEIL LOG LN EXP RANDOM
%token READ AS SKIP USE COMMENT
%token SUBSETS INDEXSET POWERSET
%token <sym> NUMBSYM STRGSYM VARSYM SETSYM
%token <def> NUMBDEF STRGDEF SETDEF DEFNAME
%token <name> NAME
%token <strg> STRG
%token <numb> NUMB
%token <bits> SCALE
%token <bits> SEPARATE

%type <code> stmt decl_set decl_par decl_var decl_obj decl_sub
%type <code> def_numb def_strg def_set
%type <code> stmt_print
%type <code> constraint
%type <code> expr expr_list symidx tuple tuple_list sexpr lexpr read read_par
%type <code> idxset subterm summand factor vexpr term name_list
%type <code> expr_entry expr_entry_list set_entry set_entry_list par_default
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
%left  ABS SGN CARD FLOOR CEIL PROJ LOG LN EXP
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
   | def_numb   { code_set_root($1); }
   | def_strg   { code_set_root($1); }
   | def_set    { code_set_root($1); }
   | stmt_print { code_set_root($1); }
   ;

/* ----------------------------------------------------------------------------
 * --- Set Declaration
 * ----------------------------------------------------------------------------
 */
decl_set
   : DECLSET NAME ASGN sexpr ';' {
         $$ = code_new_inst(i_newsym_set1, 3,
            code_new_name($2),                                       /* Name */
            code_new_inst(i_idxset_new, 3,                      /* index set */
               code_new_inst(i_tuple_empty, 0),
               code_new_inst(i_set_pseudo, 0),
               code_new_inst(i_bool_true, 0)),
            $4);                                              /* initial set */
      }
   | DECLSET NAME '[' idxset ']' ASGN sexpr ';' {
         $$ = code_new_inst(i_newsym_set1, 3,
            code_new_name($2),                                       /* Name */
            $4,                                                 /* index set */
            $7);                                                      /* set */
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
               code_new_inst(i_set_pseudo, 0),
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
 * --- Define Declaration
 * ----------------------------------------------------------------------------
 */
def_numb
   : DEFNUMB DEFNAME '(' name_list ')' ASGN expr ';' {
         $$ = code_new_inst(i_newdef, 3, code_new_define($2), $4, $7);
      }
   ;

def_strg
   : DEFSTRG DEFNAME '(' name_list ')' ASGN expr ';' {
         $$ = code_new_inst(i_newdef, 3, code_new_define($2), $4, $7);
      }
   ;

def_set
   : DEFSET DEFNAME '(' name_list ')' ASGN sexpr ';' {
         $$ = code_new_inst(i_newdef, 3, code_new_define($2), $4, $7);
      }
   ;

name_list
   : NAME {
         $$ = code_new_inst(i_elem_list_new, 1, code_new_name($1));
      }
   | name_list ',' NAME {
         $$ = code_new_inst(i_elem_list_add, 2, $1, code_new_name($3));
      }
   ;

/* ----------------------------------------------------------------------------
 * --- Param Declaration
 * ----------------------------------------------------------------------------
 */
decl_par
   : DECLPAR NAME '[' idxset ']' ASGN expr_entry_list par_default ';' {
         $$ = code_new_inst(i_newsym_para1, 4, code_new_name($2), $4, $7, $8);
      }
   | DECLPAR NAME '[' idxset ']' ASGN expr par_default ';' {
         $$ = code_new_inst(i_newsym_para2, 4, code_new_name($2), $4, $7, $8);
      }
   | DECLPAR NAME ASGN expr ';' {
         $$ = code_new_inst(i_newsym_para1, 4,
            code_new_name($2),
            code_new_inst(i_idxset_new, 3,
               code_new_inst(i_tuple_empty, 0),
               code_new_inst(i_set_pseudo, 0),
               code_new_inst(i_bool_true, 0)),
            code_new_inst(i_entry_list_new, 1,
               code_new_inst(i_entry, 2,
                  code_new_inst(i_tuple_empty, 0),
                  $4)), code_new_inst(i_nop, 0));
      }
   ;

par_default
   : /* empty */   { $$ = code_new_inst(i_nop, 0); }
   | DEFAULT expr  { $$ = code_new_inst(i_entry, 2, code_new_inst(i_tuple_empty, 0), $2); }
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
   | DECLVAR NAME '[' idxset ']' BINARY priority startval ';' {
         $$ = code_new_inst(i_newsym_var, 7,
            code_new_name($2),
            $4,
            code_new_varclass(VAR_BIN),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            $7, $8);
      }
   | DECLVAR NAME var_type lower upper priority startval ';' {
         $$ = code_new_inst(i_newsym_var, 7,
            code_new_name($2),
            code_new_inst(i_idxset_new, 3,
               code_new_inst(i_tuple_empty, 0),
               code_new_inst(i_set_pseudo, 0),
               code_new_inst(i_bool_true, 0)),
            $3, $4, $5, $6, $7);
      }
   | DECLVAR NAME BINARY priority startval ';' {
         $$ = code_new_inst(i_newsym_var, 7,
            code_new_name($2),
            code_new_inst(i_idxset_new, 3,
               code_new_inst(i_tuple_empty, 0),
               code_new_inst(i_set_pseudo, 0),
               code_new_inst(i_bool_true, 0)),
            code_new_varclass(VAR_BIN),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            $4, $5);
      }
   ;

var_type
   : /* empty */      { $$ = code_new_varclass(VAR_CON); }
   | REAL             { $$ = code_new_varclass(VAR_CON); }
   | INTEGER          { $$ = code_new_varclass(VAR_INT); }
/*   | BINARY           { $$ = code_new_varclass(VAR_BIN); }*/
   ;

lower
   : /* empty */      {
         $$ = code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0)));
      }
   | CMP_GE expr      { $$ = code_new_inst(i_bound_new, 1, $2); }
   | CMP_GE '-' INFTY { $$ = code_new_bound(BOUND_MINUS_INFTY); }
   ;

upper
   : /* empty */      { $$ = code_new_bound(BOUND_INFTY); }
   | CMP_LE expr      { $$ = code_new_inst(i_bound_new, 1, $2); }
   | CMP_LE INFTY     { $$ = code_new_bound(BOUND_INFTY); }
   ;

priority
   : /* empty */      { $$ = code_new_numb(numb_new_integer(0)); }
   | PRIORITY expr    { $$ = $2; }
   ;

startval
   : /* empty */      { $$ = code_new_numb(numb_new_integer(0)); }
   | STARTVAL expr    { $$ = $2; }
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
   | expr con_type term con_attr_list {
        $$ = code_new_inst(i_constraint, 4,
           code_new_inst(i_term_expr, 1, $1),
           $2, $3, code_new_bits($4));
     }
   | expr con_type term CMP_LE expr con_attr_list {
        $$ = code_new_inst(i_rangeconst, 6, $1, $3, $5, $2,
           code_new_contype(CON_RHS), code_new_bits($6)); 
     }
   | expr con_type term CMP_GE expr con_attr_list {
        $$ = code_new_inst(i_rangeconst, 6, $5, $3, $1, $2,
           code_new_contype(CON_LHS), code_new_bits($6)); 
     }
   | FORALL idxset DO constraint {
        $$ = code_new_inst(i_forall, 2, $2, $4);
     }
   ;

con_attr_list
   : /* empty */                { $$ = 0; }
   | con_attr_list ',' con_attr { $$ = $1 | $3; }
   ;

con_attr
   : SCALE     { $$ = LP_FLAG_CON_SCALE; }
   | SEPARATE  { $$ = LP_FLAG_CON_SEPAR; }
   ;

con_type
   : CMP_LE  { $$ = code_new_contype(CON_RHS); }
   | CMP_GE  { $$ = code_new_contype(CON_LHS); }
   | CMP_EQ  { $$ = code_new_contype(CON_EQUAL); }
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
   : factor                { $$ = $1; }
   | factor '*' expr       { $$ = code_new_inst(i_term_coeff, 2, $1, $3); }
   | factor '/' expr       {
         $$ = code_new_inst(i_term_coeff, 2, $1,
            code_new_inst(i_expr_div, 2, code_new_numb(numb_new_integer(1)), $3));
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
   | '+' '(' term ')' %prec UNARY { $$ = $3; }           
   | '-' '(' term ')' %prec UNARY {
         $$ = code_new_inst(i_term_coeff, 2, $3, code_new_numb(numb_new_integer(-1)));
      }
   | expr '*' '(' term ')'  { $$ = code_new_inst(i_term_coeff, 2, $4, $1); }
   ;

vexpr
   : VARSYM symidx          {
         $$ = code_new_inst(i_symbol_deref, 2, code_new_symbol($1), $2);
      } 
   | '+' VARSYM symidx %prec UNARY  {
         $$ = code_new_inst(i_symbol_deref, 2, code_new_symbol($2), $3);
      } 
   | '-' VARSYM symidx %prec UNARY  { 
         $$ = code_new_inst(i_term_coeff, 2,
            code_new_inst(i_symbol_deref, 2, code_new_symbol($2), $3),
            code_new_numb(numb_new_integer(-1)));
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
         $$ = code_new_inst(i_symbol_deref, 2, code_new_symbol($1), $2);
      }
   | SETDEF '(' expr_list ')' {
         $$ = code_new_inst(i_define_deref, 2, code_new_define($1), $3);
      }
   | EMPTY_SET { $$ = code_new_inst(i_set_empty, 1, code_new_size(0)); }
   | '{' expr TO expr BY expr '}' {
         $$ = code_new_inst(i_set_range, 3, $2, $4, $6);
      }
   | '{' expr TO expr '}' {
         $$ = code_new_inst(i_set_range, 3, $2, $4, code_new_numb(numb_new_integer(1)));
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
   | '{' idxset '}'     { $$ = code_new_inst(i_set_idxset, 1, $2); }
   | PROJ '(' sexpr ',' tuple ')' {
         $$ = code_new_inst(i_set_proj, 2, $3, $5);
       }
    | INDEXSET '(' SETSYM ')' {
          $$ = code_new_inst(i_set_indexset, 1, code_new_symbol($3));
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
    | NOT lexpr          { $$ = code_new_inst(i_bool_not, 1, $2); }
    | '(' lexpr ')'      { $$ = $2; }
    | tuple IN sexpr     { $$ = code_new_inst(i_bool_is_elem, 2, $1, $3); } 
    | EXISTS '(' idxset ')' %prec EXISTS { $$ = code_new_inst(i_bool_exists, 1, $3); } 
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
         $$ = code_new_inst(i_symbol_deref, 2, code_new_symbol($1), $2);
      }
   | STRGSYM symidx { 
         $$ = code_new_inst(i_symbol_deref, 2, code_new_symbol($1), $2);
      }
   | NUMBDEF '(' expr_list ')' {
         $$ = code_new_inst(i_define_deref, 2, code_new_define($1), $3);
      }
   | STRGDEF '(' expr_list ')' {
         $$ = code_new_inst(i_define_deref, 2, code_new_define($1), $3);
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
   | SGN '(' expr ')'      { $$ = code_new_inst(i_expr_sgn, 1, $3); }
   | FLOOR '(' expr ')'    { $$ = code_new_inst(i_expr_floor, 1, $3); }
   | CEIL '(' expr ')'     { $$ = code_new_inst(i_expr_ceil, 1, $3); }
   | LOG '(' expr ')'      { $$ = code_new_inst(i_expr_log, 1, $3); }
   | LN '(' expr ')'       { $$ = code_new_inst(i_expr_ln, 1, $3); }
   | EXP '(' expr ')'      { $$ = code_new_inst(i_expr_exp, 1, $3); }

   | '+' expr %prec UNARY  { $$ = $2; }
   | '-' expr %prec UNARY  { $$ = code_new_inst(i_expr_neg, 1, $2); }
   | '(' expr ')'          { $$ = $2; }
   | RANDOM '(' expr ',' expr ')' {
         $$ = code_new_inst(i_expr_rand, 2, $3, $5);
      }
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
   | MIN '(' expr_list ')' {
         $$ = code_new_inst(i_expr_min2, 1, $3);
      }
   | MAX '(' expr_list ')' {
         $$ = code_new_inst(i_expr_max2, 1, $3);
      }
   ;








