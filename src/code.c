#ident "@(#) $Id: code.c,v 1.2 2001/01/28 19:16:13 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: code.c                                                        */
/*   Name....: Code Functions                                                */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"

#define CODE_SID  0x436f6465
#define INST_NULL ((Inst)0)
//#define CODE_NULL ((CodeNode*)0)

typedef union code_value CodeValue;

union code_value
{
   double      numb;
   const char* strg;
   const char* name;
   Tuple*      tuple;
   Set*        set;
   Term*       term;
   Ineq*       ineq;
   Entry*      entry;
   Symbol*     symbol;
   IdxSet*     idxset;
   int         bool;
   int         size;
   List*       list;
   VarType     vartype;
   IneqType    ineqtype;
};

#define MAX_CHILDS 6

struct code_node
{
   SID
   CodeType  type;
   Inst      eval;
   CodeValue value;
   CodeNode* child[MAX_CHILDS];
   int       lineno;
   int       column;
};

static CodeNode* root;

CodeNode* code_new_inst(Inst inst, int childs, ...)
{
   va_list   ap;
   CodeNode* node = calloc(1, sizeof(*node));
   int       i;
   CodeNode* child;
   
   assert(inst != INST_NULL);
   assert(node != NULL);

   node->eval   = inst;
   /* Das sollte eigendlich ein parameter sein, aber wozu bei jedem
    * code_new diese Funktionen mit uebergeben.
    */
   node->lineno = scan_get_lineno();
   node->column = scan_get_column();
   
   SID_set(node, CODE_SID);
   assert(code_is_valid(node));

   va_start(ap, childs);

   for(i = 0; i < childs; i++)
   {
      child = va_arg(ap, CodeNode*);
      assert((child == NULL) || code_is_valid(child));
      node->child[i] = child;
   }
   va_end(ap);
   
   return node;
}

CodeNode* code_new_numb(double numb)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(node != NULL);

   node->type       = CODE_NUMB;
   node->eval       = i_nop;
   node->value.numb = numb;
   node->lineno     = scan_get_lineno();
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
   node->lineno     = scan_get_lineno();
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
   node->lineno     = scan_get_lineno();
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
   node->lineno     = scan_get_lineno();
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
   node->lineno        = scan_get_lineno();
   node->column        = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

CodeNode* code_new_ineqtype(IneqType ineqtype)
{
   CodeNode* node = calloc(1, sizeof(*node));

   assert(node != NULL);

   node->type           = CODE_INEQTYPE;
   node->eval           = i_nop;
   node->value.ineqtype = ineqtype;
   node->lineno         = scan_get_lineno();
   node->column         = scan_get_column();

   SID_set(node, CODE_SID);
   assert(code_is_valid(node));
   
   return node;
}

static void code_free_value(CodeNode* node)
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
   case CODE_INEQ :
      ineq_free(node->value.ineq);
      break;
   case CODE_ENTRY :
      entry_free(node->value.entry);
      break;
   case CODE_SYMBOL :
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
   case CODE_INEQTYPE :
      break;
   default :
      assert(0);
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

int code_is_valid(const CodeNode* node)
{
   return ((node != NULL) && SID_ok(node, CODE_SID));
}

void code_set_child(CodeNode* node, int idx, CodeNode* child)
{
   assert(code_is_valid(node));
   assert(idx   >= 0);
   assert(idx   <  MAX_CHILDS);
   assert(child != NULL);

   node->child[idx] = child;
}

CodeType code_get_type(CodeNode* node)
{
   assert(code_is_valid(node));

   return node->type;
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

CodeType code_eval(CodeNode* node)
{
   assert(code_is_valid(node));

   (*node->eval)(node);

   return node->type;
}

void code_execute(CodeNode* node)
{
   if (verbose)
      fprintf(stderr, "Execute\n");

   switch(code_eval(node))
   {
   case CODE_VOID :
      return;
   case CODE_NUMB :
      fprintf(stderr, "%g\n", code_get_numb(node));
      break;
   case CODE_STRG :
      fprintf(stderr, "%s\n", code_get_strg(node));
      break;
   case CODE_TUPLE :
      tuple_print(stderr, code_get_tuple(node));
      fprintf(stderr, "\n");
      break;
   case CODE_SET :
      set_print(stderr, code_get_set(node));
      fprintf(stderr, "\n");
      break;
   case CODE_IDXSET :
      idxset_print(stderr, code_get_idxset(node));
      fprintf(stderr, "\n");
      break;
   case CODE_TERM :
      term_print(stderr, code_get_term(node), TERM_PRINT_SYMBOL);
      fprintf(stderr, "\n");
      break;
   case CODE_INEQ :
      ineq_print(stderr, code_get_ineq(node));
      fprintf(stderr, "\n");
      break;
   case CODE_BOOL :
      fprintf(stderr, "%s\n", code_get_bool(node) ? "True" : "False");
      break;
   default :
      fprintf(stderr, "Unknown element type\n");
   }
   fprintf(stderr, "Execute must return void element\n");
   assert(0);
}

static CodeNode* code_check_type(CodeNode* node, CodeType expected)
{
   assert(code_is_valid(node));

   if (node->type != expected)
   {
      fprintf(stderr, 
         "Type error: expected %d, is %d, lineno=%d, column=%d\n",
         expected,
         node->type,
         node->lineno, node->column);      

      exit(1);
   }
   return node;
}

/* ----------------------------------------------------------------------------
 * Get Funktionen
 * ----------------------------------------------------------------------------
 */
CodeNode* code_get_child(CodeNode* node, int no)
{
   assert(code_is_valid(node));
   assert(no >= 0);
   assert(no <  MAX_CHILDS);

   if (node->child[no] != NULL)
      (void)code_eval(node->child[no]);
   
   return node->child[no];
}

int code_get_lineno(const CodeNode* node)
{
   assert(code_is_valid(node));
   
   return node->lineno;
}

int code_get_column(const CodeNode* node)
{
   assert(code_is_valid(node));
   
   return node->column;
}

double code_get_numb(CodeNode* node)
{
   return code_check_type(node, CODE_NUMB)->value.numb;
}

const char* code_get_strg(CodeNode* node)
{
   return code_check_type(node, CODE_STRG)->value.strg;
}

const char* code_get_name(CodeNode* node)
{
   return code_check_type(node, CODE_NAME)->value.name;
}

Tuple* code_get_tuple(CodeNode* node)
{
   return tuple_copy(code_check_type(node, CODE_TUPLE)->value.tuple);
}

Set* code_get_set(CodeNode* node)
{
   return set_copy(code_check_type(node, CODE_SET)->value.set);
}

IdxSet* code_get_idxset(CodeNode* node)
{
   return idxset_copy(code_check_type(node, CODE_IDXSET)->value.idxset);
}

Entry* code_get_entry(CodeNode* node)
{
   return entry_copy(code_check_type(node, CODE_ENTRY)->value.entry);
}

Term* code_get_term(CodeNode* node)
{
   return term_copy(code_check_type(node, CODE_TERM)->value.term);
}

Ineq* code_get_ineq(CodeNode* node)
{
   return ineq_copy(code_check_type(node, CODE_INEQ)->value.ineq);
}

int code_get_size(CodeNode* node)
{
   return code_check_type(node, CODE_SIZE)->value.size;
}

int code_get_bool(CodeNode* node)
{
   return code_check_type(node, CODE_BOOL)->value.bool;
}

List* code_get_list(CodeNode* node)
{
   return list_copy(code_check_type(node, CODE_LIST)->value.list);
}

VarType code_get_vartype(CodeNode* node)
{
   return code_check_type(node, CODE_VARTYPE)->value.vartype;
}

IneqType code_get_ineqtype(CodeNode* node)
{
   return code_check_type(node, CODE_INEQTYPE)->value.ineqtype;
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

   code_free_value(node);

   node->type       = CODE_STRG;
   node->value.strg = strg;
}

void code_value_name(CodeNode* node, const char* name)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type       = CODE_NAME;
   node->value.name = name;
}

void code_value_tuple(CodeNode* node, Tuple* tuple)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type        = CODE_TUPLE;
   node->value.tuple = tuple_copy(tuple);
}

void code_value_set(CodeNode* node, Set* set)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type      = CODE_SET;
   node->value.set = set_copy(set);
}

void code_value_idxset(CodeNode* node, IdxSet* idxset)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type         = CODE_IDXSET;
   node->value.idxset = idxset_copy(idxset);
}

void code_value_entry(CodeNode* node, Entry* entry)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type        = CODE_ENTRY;
   node->value.entry = entry_copy(entry);
}

void code_value_term(CodeNode* node, Term* term)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type       = CODE_TERM;
   node->value.term = term_copy(term);
}

void code_value_ineq(CodeNode* node, Ineq* ineq)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type       = CODE_INEQ;
   node->value.ineq = ineq_copy(ineq);
}

void code_value_bool(CodeNode* node, int bool)
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

void code_value_list(CodeNode* node, List* list)
{
   assert(code_is_valid(node));

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

void code_value_ineqtype(CodeNode* node, IneqType ineqtype)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type           = CODE_INEQTYPE;
   node->value.ineqtype = ineqtype;
}

void code_value_void(CodeNode* node)
{
   assert(code_is_valid(node));

   code_free_value(node);

   node->type = CODE_VOID;
}



