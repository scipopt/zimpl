#ident "@(#) $Id: code.c,v 1.17 2003/02/19 15:55:52 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: code.c                                                        */
/*   Name....: Code Node Functions                                           */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"
#include "code.h"
#include "inst.h"

typedef union code_value CodeValue;

union code_value
{
   double       numb;
   const char*  strg;
   const char*  name;
   Tuple*       tuple;
   Set*         set;
   Term*        term;
   Entry*       entry;
   IdxSet*      idxset;
   Bool         bool;
   int          size;
   List*        list;
   VarType      vartype;
   ConType      contype;
   RDef*        rdef;
   RPar*        rpar;
   unsigned int bits;
   Symbol*      sym;
};

#define MAX_CHILDS 7

struct code_node
{
   SID
   CodeType    type;
   Inst        eval;
   CodeValue   value;
   CodeNode*   child[MAX_CHILDS];
   const Stmt* stmt;
   int         column;
};

#define CODE_SID  0x436f6465

static CodeNode* root;

inline Bool code_is_valid(const CodeNode* node)
{
   return ((node != NULL) && SID_ok(node, CODE_SID));
}

CodeNode* code_new_inst(Inst inst, int childs, ...)
{
   va_list   ap;
   CodeNode* node = calloc(1, sizeof(*node));
   int       i;
   CodeNode* child;
   
   assert(inst != INST_NULL);
   assert(node != NULL);

   node->type   = CODE_ERR; /* call to eval() will set this */
   node->eval   = inst;
   /* Das sollte eigendlich ein parameter sein, aber wozu bei jedem
    * code_new diese Funktionen mit uebergeben.
    */
   node->stmt   = scan_get_stmt();
   node->column = scan_get_column();
   
   SID_set(node, CODE_SID);
   assert(code_is_valid(node));

   /*lint -save -e826 -e662 */
   va_start(ap, childs);

   for(i = 0; i < childs; i++)
   {
      child = va_arg(ap, CodeNode*);
      assert((child == NULL) || code_is_valid(child));
      node->child[i] = child;
   }
   va_end(ap);
   /*lint -restore */
   
   return node;
}

CodeNode* code_new_numb(double numb)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(node != NULL);

   node->type       = CODE_NUMB;
   node->eval       = i_nop;
   node->value.numb = numb;
   node->stmt       = scan_get_stmt();
   node->column     = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

CodeNode* code_new_strg(const char* strg)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(strg != NULL);
   assert(node != NULL);

   node->type       = CODE_STRG;
   node->eval       = i_nop;
   node->value.strg = strg;
   node->stmt       = scan_get_stmt();
   node->column     = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

CodeNode* code_new_name(const char* name)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(name != NULL);
   assert(node != NULL);

   node->type       = CODE_NAME;
   node->eval       = i_nop;
   node->value.name = name;
   node->stmt       = scan_get_stmt();
   node->column     = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

CodeNode* code_new_size(int size)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(size >= 0);
   assert(node != NULL);

   node->type       = CODE_SIZE;
   node->eval       = i_nop;
   node->value.size = size;
   node->stmt       = scan_get_stmt();
   node->column     = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

CodeNode* code_new_vartype(VarType vartype)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(node != NULL);

   node->type          = CODE_VARTYPE;
   node->eval          = i_nop;
   node->value.vartype = vartype;
   node->stmt          = scan_get_stmt();
   node->column        = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

CodeNode* code_new_contype(ConType contype)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(node != NULL);

   node->type          = CODE_CONTYPE;
   node->eval          = i_nop;
   node->value.contype = contype;
   node->stmt          = scan_get_stmt();
   node->column        = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

CodeNode* code_new_bits(unsigned int bits)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(node != NULL);

   node->type       = CODE_BITS;
   node->eval       = i_nop;
   node->value.bits = bits;
   node->stmt       = scan_get_stmt();
   node->column     = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

CodeNode* code_new_symbol(Symbol* sym)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(node != NULL);

   node->type       = CODE_SYM;
   node->eval       = i_nop;
   node->value.sym  = sym;
   node->stmt       = scan_get_stmt();
   node->column     = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

static inline void code_free_value(const CodeNode* node)
{
   assert(code_is_valid(node));

   switch(node->type)
   {
   case CODE_ERR :
   case CODE_VOID :
      /* Kann passieren, wenn bei code_value_() ein bis dahin unbenutzter
       * Knoten verwendet wird.
       */
      break;
   case CODE_NUMB :
   case CODE_STRG :
   case CODE_NAME :
      break;
   case CODE_TUPLE :
      tuple_free(node->value.tuple);
      break;
   case CODE_SET :
      set_free(node->value.set);
      break;
   case CODE_TERM :
      term_free(node->value.term);
      break;
   case CODE_ENTRY :
      entry_free(node->value.entry);
      break;
   case CODE_IDXSET :
      idxset_free(node->value.idxset);
      break;
   case CODE_BOOL :
   case CODE_SIZE :
      break;
   case CODE_LIST :
      list_free(node->value.list);
      break;
   case CODE_VARTYPE :
   case CODE_CONTYPE :
      break;
   case CODE_RDEF :
      rdef_free(node->value.rdef);
      break;
   case CODE_RPAR :
      rpar_free(node->value.rpar);
      break;
   case CODE_BITS :
      break;
   case CODE_SYM :
      break;
   default :
      abort();
   }
}

void code_free(CodeNode* node)
{
   int i;

   for(i = 0; i < MAX_CHILDS; i++)
      if (node->child[i] != NULL)
         code_free(node->child[i]);

   code_free_value(node);
   free(node);
}

void code_set_child(CodeNode* node, int idx, CodeNode* child)
{
   assert(code_is_valid(node));
   assert(idx   >= 0);
   assert(idx   <  MAX_CHILDS);
   assert(child != NULL);

   node->child[idx] = child;
}

CodeType code_get_type(const CodeNode* node)
{
   assert(code_is_valid(node));

   return node->type;
}

Inst code_get_inst(const CodeNode* node)
{
   assert(code_is_valid(node));

   return node->eval;
}
     
void code_set_root(CodeNode* node)
{
   assert(code_is_valid(node));
   
   root = node;
}

CodeNode* code_get_root(void)
{
   return root;
}

static inline CodeNode* code_check_type(CodeNode* node, CodeType expected)
{
   assert(code_is_valid(node));

   if (node->type != expected)
   {
      fprintf(stderr, "*** Error: Type error, expected %d got %d\n",
         expected, node->type);
      code_errmsg(node);
      abort();
   }
   return node;
}

void code_errmsg(const CodeNode* node)
{
   fprintf(stderr, "*** File: %s Line %d\n",
      stmt_get_filename(node->stmt),
      stmt_get_lineno(node->stmt));

   show_source(stderr, stmt_get_text(node->stmt), node->column);
}

inline CodeNode* code_eval(CodeNode* node)
{
   assert(code_is_valid(node));

   return (*node->eval)(node);
}

/* ----------------------------------------------------------------------------
 * Get Funktionen
 * ----------------------------------------------------------------------------
 */

inline CodeNode* code_get_child(const CodeNode* node, int no)
{
   assert(code_is_valid(node));
   assert(no              >= 0);
   assert(no              <  MAX_CHILDS);
   assert(node->child[no] != NULL);
   
   return node->child[no];
}

inline double code_get_numb(CodeNode* node)
{
   return code_check_type(node, CODE_NUMB)->value.numb;
}

inline const char* code_get_strg(CodeNode* node)
{
   return code_check_type(node, CODE_STRG)->value.strg;
}

inline const char* code_get_name(CodeNode* node)
{
   return code_check_type(node, CODE_NAME)->value.name;
}

inline const Tuple* code_get_tuple(CodeNode* node)
{
   return code_check_type(node, CODE_TUPLE)->value.tuple;
}

inline const Set* code_get_set(CodeNode* node)
{
   return code_check_type(node, CODE_SET)->value.set;
}

inline const IdxSet* code_get_idxset(CodeNode* node)
{
   return code_check_type(node, CODE_IDXSET)->value.idxset;
}

inline const Entry* code_get_entry(CodeNode* node)
{
   return code_check_type(node, CODE_ENTRY)->value.entry;
}

inline const Term* code_get_term(CodeNode* node)
{
   return code_check_type(node, CODE_TERM)->value.term;
}

inline int code_get_size(CodeNode* node)
{
   return code_check_type(node, CODE_SIZE)->value.size;
}

inline Bool code_get_bool(CodeNode* node)
{
   return code_check_type(node, CODE_BOOL)->value.bool;
}

inline const List* code_get_list(CodeNode* node)
{
   return code_check_type(node, CODE_LIST)->value.list;
}

inline VarType code_get_vartype(CodeNode* node)
{
   return code_check_type(node, CODE_VARTYPE)->value.vartype;
}

inline ConType code_get_contype(CodeNode* node)
{
   return code_check_type(node, CODE_CONTYPE)->value.contype;
}

inline const RDef* code_get_rdef(CodeNode* node)
{
   return code_check_type(node, CODE_RDEF)->value.rdef;
}

inline const RPar* code_get_rpar(CodeNode* node)
{
   return code_check_type(node, CODE_RPAR)->value.rpar;
}

inline unsigned int code_get_bits(CodeNode* node)
{
   return code_check_type(node, CODE_BITS)->value.bits;
}

inline Symbol* code_get_symbol(CodeNode* node)
{
   return code_check_type(node, CODE_SYM)->value.sym;
}

/* ----------------------------------------------------------------------------
 * Value Funktionen
 * ----------------------------------------------------------------------------
 */
void code_value_numb(CodeNode* node, double numb)
{
   assert(code_is_valid(node));

   code_free_value(node);
   
   node->type       = CODE_NUMB;
   node->value.numb = numb;
}

void code_value_strg(CodeNode* node, const char* strg)
{
   assert(code_is_valid(node));
   assert(strg != NULL);
   
   code_free_value(node);

   node->type       = CODE_STRG;
   node->value.strg = strg;
}

void code_value_name(CodeNode* node, const char* name)
{
   assert(code_is_valid(node));
   assert(name != NULL);
   
   code_free_value(node);

   node->type       = CODE_NAME;
   node->value.name = name;
}

void code_value_tuple(CodeNode* node, const Tuple* tuple)
{
   assert(code_is_valid(node));
   assert(tuple_is_valid(tuple));

   code_free_value(node);

   node->type        = CODE_TUPLE;
   node->value.tuple = tuple_copy(tuple);
}

void code_value_set(CodeNode* node, const Set* set)
{
   assert(code_is_valid(node));
   assert(set_is_valid(set));

   code_free_value(node);

   node->type      = CODE_SET;
   node->value.set = set_copy(set);
}

void code_value_idxset(CodeNode* node, const IdxSet* idxset)
{
   assert(code_is_valid(node));
   assert(idxset_is_valid(idxset));

   code_free_value(node);

   node->type         = CODE_IDXSET;
   node->value.idxset = idxset_copy(idxset);
}

void code_value_entry(CodeNode* node, const Entry* entry)
{
   assert(code_is_valid(node));
   assert(entry_is_valid(entry));

   code_free_value(node);

   node->type        = CODE_ENTRY;
   node->value.entry = entry_copy(entry);
}

void code_value_term(CodeNode* node, const Term* term)
{
   assert(code_is_valid(node));
   assert(term_is_valid(term));

   code_free_value(node);

   node->type       = CODE_TERM;
   node->value.term = term_copy(term);
}

void code_value_bool(CodeNode* node, Bool bool)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type       = CODE_BOOL;
   node->value.bool = bool;
}

void code_value_size(CodeNode* node, int size)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type       = CODE_SIZE;
   node->value.size = size;
}

void code_value_list(CodeNode* node, const List* list)
{
   assert(code_is_valid(node));
   assert(list_is_valid(list));

   code_free_value(node);

   node->type       = CODE_LIST;
   node->value.list = list_copy(list);
}

void code_value_vartype(CodeNode* node, VarType vartype)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type          = CODE_VARTYPE;
   node->value.vartype = vartype;
}

void code_value_contype(CodeNode* node, ConType contype)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type          = CODE_CONTYPE;
   node->value.contype = contype;
}

void code_value_rdef(CodeNode* node, const RDef* rdef)
{
   assert(code_is_valid(node));
   assert(rdef_is_valid(rdef));

   code_free_value(node);

   node->type       = CODE_RDEF;
   node->value.rdef = rdef_copy(rdef);
}

void code_value_rpar(CodeNode* node, const RPar* rpar)
{
   assert(code_is_valid(node));
   assert(rpar_is_valid(rpar));

   code_free_value(node);

   node->type       = CODE_RPAR;
   node->value.rpar = rpar_copy(rpar);
}

void code_value_bits(CodeNode* node, unsigned int bits)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type       = CODE_BITS;
   node->value.bits = bits;
}

void code_value_void(CodeNode* node)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type = CODE_VOID;
}

void code_copy_value(CodeNode* dst, const CodeNode* src)
{
   assert(code_is_valid(dst));
   assert(code_is_valid(src));
   
   code_free_value(dst);

   switch(src->type)
   {
   case CODE_NUMB :
      dst->value.numb = src->value.numb;
      break;
   case CODE_STRG :
      dst->value.strg = src->value.strg;
      break;
   case CODE_NAME :
      dst->value.name = src->value.name;
      break;
   case CODE_TUPLE:
      dst->value.tuple = tuple_copy(src->value.tuple);
      break;
   case CODE_SET :
      dst->value.set = set_copy(src->value.set);
      break;
   case CODE_IDXSET :
      dst->value.idxset = idxset_copy(src->value.idxset);
      break;
   case CODE_ENTRY :
      dst->value.entry = entry_copy(src->value.entry);
      break;
   case CODE_TERM :
      dst->value.term = term_copy(src->value.term);
      break;
   case CODE_BOOL :
      dst->value.bool = src->value.bool;
      break;
   case CODE_SIZE :
      dst->value.size = src->value.size;
      break;
   case CODE_LIST :
      dst->value.list = list_copy(src->value.list);
      break;
   case CODE_VARTYPE :
      dst->value.vartype = src->value.vartype;
      break;
   case CODE_CONTYPE :
      dst->value.contype = src->value.contype;
      break;
   case CODE_RDEF :
      dst->value.rdef = rdef_copy(src->value.rdef);
      break;
   case CODE_RPAR :
      dst->value.rpar = rpar_copy(src->value.rpar);
      break;
   case CODE_BITS :
      dst->value.bits = src->value.bits;
      break;
   case CODE_VOID :
      break;
   default :
      abort();
   }
   dst->type = src->type;
}

/* ----------------------------------------------------------------------------
 * Macro Funktionen for better inlining
 * ----------------------------------------------------------------------------
 */
CodeNode* code_eval_child(const CodeNode* node, int no)
{
   return code_eval(code_get_child(node, no));
}

double code_eval_child_numb(const CodeNode* node, int no)
{
   return code_get_numb(code_eval(code_get_child(node, no)));
}

const char* code_eval_child_strg(const CodeNode* node, int no)
{
   return code_get_strg(code_eval(code_get_child(node, no)));
}

const char*code_eval_child_name(const CodeNode* node, int no)
{
   return code_get_name(code_eval(code_get_child(node, no)));
}

const Tuple* code_eval_child_tuple(const CodeNode* node, int no)
{
   return code_get_tuple(code_eval(code_get_child(node, no)));
}

const Set* code_eval_child_set(const CodeNode* node, int no)
{
   return code_get_set(code_eval(code_get_child(node, no)));
}

const IdxSet* code_eval_child_idxset(const CodeNode* node, int no)
{
   return code_get_idxset(code_eval(code_get_child(node, no)));
}

const Entry* code_eval_child_entry(const CodeNode* node, int no)
{
   return code_get_entry(code_eval(code_get_child(node, no)));
}

const Term* code_eval_child_term(const CodeNode* node, int no)
{
   return code_get_term(code_eval(code_get_child(node, no)));
}

int code_eval_child_size(const CodeNode* node, int no)
{
   return code_get_size(code_eval(code_get_child(node, no)));
}

Bool code_eval_child_bool(const CodeNode* node, int no)
{
   return code_get_bool(code_eval(code_get_child(node, no)));
}

const List* code_eval_child_list(const CodeNode* node, int no)
{
   return code_get_list(code_eval(code_get_child(node, no)));
}

VarType code_eval_child_vartype(const CodeNode* node, int no)
{
   return code_get_vartype(code_eval(code_get_child(node, no)));
}

ConType code_eval_child_contype(const CodeNode* node, int no)
{
   return code_get_contype(code_eval(code_get_child(node, no)));
}

const RDef* code_eval_child_rdef(const CodeNode* node, int no)
{
   return code_get_rdef(code_eval(code_get_child(node, no)));
}

const RPar* code_eval_child_rpar(const CodeNode* node, int no)
{
   return code_get_rpar(code_eval(code_get_child(node, no)));
}

unsigned int code_eval_child_bits(const CodeNode* node, int no)
{
   return code_get_bits(code_eval(code_get_child(node, no)));
}

Symbol* code_eval_child_symbol(const CodeNode* node, int no)
{
   return code_get_symbol(code_eval(code_get_child(node, no)));
}








