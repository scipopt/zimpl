#pragma ident "@(#) $Id: mme.h,v 1.76 2006/09/09 10:00:21 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mme.h                                                         */
/*   Name....: Mathematical Modelling Engine                                 */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2006 by Thorsten Koch <koch@zib.de>
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
#ifndef _MME_H_
#define _MME_H_

#ifndef _BOOL_H_
#error "Need to include bool.h before mme.h"
#endif

#ifndef _RATLPTYPES_H_
#error "Need to include ratlptypes.h before mme.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum element_type
{
   ELEM_ERR = 0, ELEM_FREE, ELEM_NUMB, ELEM_STRG, ELEM_NAME
};

enum set_check_type
{
   SET_CHECK_NONE, SET_CHECK_QUIET, SET_CHECK_WARN
};

enum symbol_type     { SYM_ERR = 0, SYM_NUMB, SYM_STRG, SYM_SET, SYM_VAR };
enum hash_type       { HASH_ERR = 0, HASH_TUPLE, HASH_ENTRY, HASH_ELEM_IDX, HASH_NUMB };
enum define_type     { DEF_ERR = 0, DEF_NUMB, DEF_STRG, DEF_BOOL, DEF_SET };

enum con_name_format { CON_FORM_MAKE, CON_FORM_NAME, CON_FORM_FULL };
enum statement_type
{
   STMT_ERR = 0, STMT_SET, STMT_PARAM, STMT_VAR, STMT_MIN, STMT_MAX,
   STMT_CONS, STMT_DEF, STMT_DO, STMT_SOS
};

enum bound_type
{
   BOUND_ERROR = 0, BOUND_VALUE, BOUND_INFTY, BOUND_MINUS_INFTY
};

/* the following is not in code.h because code.h needs mme.h anyway,
 * but we also need these declaratons.
 */
enum code_type
{
   CODE_ERR = 0, CODE_NUMB, CODE_STRG, CODE_NAME, CODE_TUPLE,
   CODE_SET, CODE_TERM, CODE_BOOL, CODE_SIZE, 
   CODE_IDXSET, CODE_LIST, CODE_VOID, CODE_ENTRY, CODE_VARCLASS, CODE_CONTYPE,
   CODE_RDEF, CODE_RPAR, CODE_BITS, CODE_SYM, CODE_DEF, CODE_BOUND
};

typedef enum code_type           CodeType;
typedef struct code_node         CodeNode;

typedef CodeNode*              (*Inst)(CodeNode* self);

typedef struct number            Numb;
typedef enum bound_type          BoundType;
typedef struct bound             Bound;
typedef enum element_type        ElemType;
typedef struct element           Elem;
typedef struct tuple             Tuple;
typedef union set                Set;
typedef union set_iter           SetIter;
typedef enum set_check_type      SetCheckType;
typedef struct entry             Entry;
typedef enum symbol_type         SymbolType;
typedef struct symbol            Symbol;
typedef enum define_type         DefineType;
typedef struct define            Define;
typedef struct index_set         IdxSet;
typedef struct term              Term;
typedef struct local             Local;
typedef struct list_element      ListElem;
typedef struct list              List;
typedef enum hash_type           HashType;
typedef struct hash              Hash;
typedef struct heap              Heap;
typedef struct read_param        RPar;
typedef struct read_definition   RDef;

typedef enum con_name_format     ConNameForm;
typedef enum statement_type      StmtType;
typedef struct statement         Stmt;
typedef struct program           Prog;
typedef struct meta_file_ptr     MFP;

union heap_data
{
   Entry* entry;
};

typedef union heap_data          HeapData;
typedef int                    (*HeapCmp)(HeapData, HeapData);

#define VERB_QUIET    0
#define VERB_NORMAL   1
#define VERB_VERBOSE  2
#define VERB_CHATTER  3
#define VERB_DEBUG    5

extern int          verbose;
/*lint -function(exit,zpl_exit) */
extern void         zpl_exit(int retval);
/*lint -sem(        zpl_add_parameter, 1p && nulterm(1)) */
extern void         zpl_add_parameter(const char* def);

/* strstore.c
 */
extern void         str_init(void);
extern void         str_exit(void);
/*lint -sem(        str_new, 1p && nulterm(1), @p == 1p && nulterm(@)) */
extern const char*  str_new(const char* s);
/*lint -sem(        str_hash, 1p)           */
extern unsigned int str_hash(const char* s);

/* numbgmp.c
 */
extern void         numb_init(unsigned long);
extern void         numb_exit(void);
/*lint -sem(        numb_new, @p == 1) */
extern Numb*        numb_new(void);
/*lint -sem(        numb_new_ascii, nulterm(1), 1p, @p == 1) */
extern Numb*        numb_new_ascii(const char* val);
/*lint -sem(        numb_new_integer, @p == 1) */
extern Numb*        numb_new_integer(int val);

/*lint -sem(        numb_free, custodial(1), 1p == 1) */
extern void         numb_free(Numb* numb);
/*lint -sem(        numb_is_valid, 1p == 1) */
extern Bool         numb_is_valid(const Numb* numb);

/*lint -sem(        numb_copy, 1p == 1, @p == 1) */
extern Numb*        numb_copy(const Numb* source);
/*lint -sem(        numb_equal, 1p == 1 && 2p == 1) */
extern Bool         numb_equal(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_cmp,p, 1p == 1 && 2p == 1) */
extern int          numb_cmp(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_set, 1p == 1 && 2p == 1) */
extern void         numb_set(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_add, 1p == 1 && 2p == 1) */
extern void         numb_add(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_add, 1p == 1 && 2p == 1, @p == 1) */
extern Numb*        numb_new_add(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_sub, 1p == 1 && 2p == 1) */
extern void         numb_sub(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_sub, 1p == 1 && 2p == 1, @p == 1) */
extern Numb*        numb_new_sub(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_mul, 1p == 1 && 2p == 1) */
extern void         numb_mul(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_mul, 1p == 1 && 2p == 1, @p == 1) */
extern Numb*        numb_new_mul(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_div, 1p == 1 && 2p == 1) */
extern void         numb_div(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_div, 1p == 1 && 2p == 1, @p == 1) */
extern Numb*        numb_new_div(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_intdiv, 1p == 1 && 2p == 1) */
extern void         numb_intdiv(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_intdiv, 1p == 1 && 2p == 1, @p == 1) */
extern Numb*        numb_new_intdiv(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_pow, 1p == 1 && 2n >= 0, @p == 1) */
extern Numb*        numb_new_pow(const Numb* base, int expo);
/*lint -sem(        numb_new_fac, 1n >= 0, @p == 1) */
extern Numb*        numb_new_fac(int n);
/*lint -sem(        numb_mod, 1p == 1 && 2p == 1) */
extern void         numb_mod(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_mod, 1p == 1 && 2p == 1, @p == 1) */
extern Numb*        numb_new_mod(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_neg, 1p == 1) */
extern void         numb_neg(Numb* numb);
/*lint -sem(        numb_abs, 1p == 1) */
extern void         numb_abs(Numb* numb);
/*lint -sem(        numb_sgn, 1p == 1) */
extern void         numb_sgn(Numb* numb);
/*lint -sem(        numb_get_sgn, 1p == 1, @n >= -1 && @n <= 1) */
extern int          numb_get_sgn(const Numb* numb);
/*lint -sem(        numb_ceil, 1p == 1) */
extern void         numb_ceil(Numb* numb);
/*lint -sem(        numb_floor, 1p == 1) */
extern void         numb_floor(Numb* numb);
/*lint -sem(        numb_new_log, 1p == 1, @p == 1 || @p == 0) */
extern Numb*        numb_new_log(const Numb* numb);
/*lint -sem(        numb_new_sqrt, 1p == 1, @p == 1 || @p == 0) */
extern Numb*        numb_new_sqrt(const Numb* numb);
/*lint -sem(        numb_new_exp, 1p == 1, @p == 1) */
extern Numb*        numb_new_exp(const Numb* numb);
/*lint -sem(        numb_new_ln, 1p == 1, @p == 1 || @p == 0) */
extern Numb*        numb_new_ln(const Numb* numb);
/*lint -sem(        numb_new_rand, 1p == 1 && 2p == 1, @p == 1) */
extern Numb*        numb_new_rand(const Numb* mini, const Numb* maxi);
/*lint -sem(        numb_todbl, 1p == 1) */
extern double       numb_todbl(const Numb* numb);
/*lint -sem(        numb_print, 1p == 1 && 2p == 1) */
extern void         numb_print(FILE* fp, const Numb* numb);
/*lint -sem(        numb_hash, 1p == 1) */
extern unsigned int numb_hash(const Numb* numb);
/*lint -sem(        numb_tostr, 1p == 1, @p && nulterm(@)) */
extern char*        numb_tostr(const Numb* numb);
/*lint -sem(        numb_zero, @p == 1) */
extern const Numb*  numb_zero(void);
/*lint -sem(        numb_one, @p == 1) */
extern const Numb*  numb_one(void);
/*lint -sem(        numb_minusone, @p == 1) */
extern const Numb*  numb_minusone(void);
/*lint -sem(        numb_is_int, 1p == 1) */
extern Bool         numb_is_int(const Numb* numb);
/*lint -sem(        numb_toint, 1p == 1) */
extern int          numb_toint(const Numb* numb);
/*lint -sem(        numb_is_number, nulterm(1), 1p) */
extern Bool         numb_is_number(const char *s);

#ifdef __GMP_H__
/*lint -sem(        numb_new_mpq, @p == 1) */
extern Numb*        numb_new_mpq(const mpq_t val);
/*lint -sem(        numb_new_mpq, 1p == 1) */
extern void         numb_get_mpq(const Numb* numb, mpq_t value);
#endif /* __GMP_H__ */

/* bound.c
 */
/*lint -sem(        bound_new, @p == 1) */
extern Bound*       bound_new(BoundType type, const Numb* value);
/*lint -sem(        bound_free, custodial(1), 1p == 1) */
extern void         bound_free(Bound* bound);
/*lint -sem(        bound_is_valid, 1p == 1) */
extern Bool         bound_is_valid(const Bound* bound);
/*lint -sem(        bound_copy, 1p == 1, @p == 1) */
extern Bound*       bound_copy(const Bound* source);
/*lint -sem(        bound_get_type, 1p == 1) */
extern BoundType    bound_get_type(const Bound* bound);
/*lint -sem(        bound_get_value, 1p == 1) */
extern const Numb*  bound_get_value(const Bound* bound);


/* list.c
 */
#define LIST_NULL ((List*)0)

/*lint -sem(        list_new_elem, 1p == 1, @p == 1) */
extern List*        list_new_elem(const Elem* elem);
/*lint -sem(        list_new_tuple, 1p == 1, @p == 1) */
extern List*        list_new_tuple(const Tuple* tuple);
/*lint -sem(        list_new_entry, 1p == 1, @p == 1) */
extern List*        list_new_entry(const Entry* entry);
/*lint -sem(        list_new_list, 1p == 1, @p == 1) */
extern List*        list_new_list(const List* list);
/*lint -sem(        list_free, custodial(1), 1p == 1) */
extern void         list_free(List* list);
/*lint -sem(        list_is_valid, 1p == 1) */
extern Bool         list_is_valid(const List* list);
/*lint -sem(        list_is_elemlist, 1p == 1) */
extern Bool         list_is_elemlist(const List* list);
/*lint -sem(        list_is_entrylist, 1p == 1) */
extern Bool         list_is_entrylist(const List* list);
/*lint -sem(        list_is_tuplelist, 1p == 1) */
extern Bool         list_is_tuplelist(const List* list);
/*lint -sem(        list_copy, 1p == 1, @p == 1) */
extern List*        list_copy(const List* list);
/*lint -sem(        list_add_elem, 1p == 1 && 2p == 1) */
extern void         list_add_elem(List* list, const Elem* elem);
/*lint -sem(        list_add_tuple, 1p == 1 && 2p == 1) */
extern void         list_add_tuple(List* list, const Tuple* tuple);
/*lint -sem(        list_add_entry, 1p == 1 && 2p == 1) */
extern void         list_add_entry(List* list, const Entry* entry);
/*lint -sem(        list_add_list, 1p == 1 && 2p == 1) */
extern void         list_add_list(List* list, const List* ll);
/*lint -sem(        list_get_elems, 1p == 1, @n >= 0) */
extern int          list_get_elems(const List* list);
/*lint -sem(        list_get_elem,  1p == 1, @p == 1) */
extern const Elem*  list_get_elem(const List* list, ListElem** idxp);
/*lint -sem(        list_get_tuple, 1p == 1, @p == 1) */
extern const Tuple* list_get_tuple(const List* list, ListElem** idxp);
/*lint -sem(        list_get_entry, 1p == 1, @p == 1) */
extern const Entry* list_get_entry(const List* list, ListElem** idxp);
/*lint -sem(        list_get_list, 1p == 1, @p == 1) */
extern const List*  list_get_list(const List* list, ListElem** idxp);
/*lint -sem(        list_print, 1p == 1 && 2p == 1) */
extern void         list_print(FILE* fp, const List* list);

/* hash.c
 */
/*lint -sem(        hash_new, 2n >= 0, @p == 1) */
extern Hash*        hash_new(HashType type, int size);
/*lint -sem(        hash_free, 1p == 1) */
extern void         hash_free(Hash* hash);
/*lint -sem(        hash_is_valid, 1p == 1) */
extern Bool         hash_is_valid(const Hash* hash);
/*lint -sem(        hash_add_tuple, 1p == 1 && 2p == 1) */
extern void         hash_add_tuple(Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_add_entry, 1p == 1 && 2p == 1) */
extern void         hash_add_entry(Hash* hash, const Entry* entry);
/*lint -sem(        hash_add_elem_idx, 1p == 1 && 2p == 1 && 3n >= 0) */
extern void         hash_add_elem_idx(Hash* hash, const Elem* elem, int idx);
/*lint -sem(        hash_add_numb, 1p == 1 && 2p == 1) */
extern void         hash_add_numb(Hash* hash, const Numb* numb);
/*lint -sem(        hash_has_tuple, 1p == 1 && 2p == 1) */
extern Bool         hash_has_tuple(const Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_has_entry, 1p == 1 && 2p == 1) */
extern Bool         hash_has_entry(const Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_has_numb, 1p == 1 && 2p == 1) */
extern Bool         hash_has_numb(const Hash* hash, const Numb* numb);
/*lint -sem(        hash_lookup_entry, 1p == 1 && 2p == 1) */
extern const Entry* hash_lookup_entry(const Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_lookup_elem_idx, 1p == 1 && 2p == 1) */
extern int          hash_lookup_elem_idx(const Hash* hash, const Elem* elem);

/* heap.c
 */
/*lint -sem(        heap_new_entry, 1n > 0 && 2p == 1, @p == 1) */
extern Heap*        heap_new_entry(int size, HeapCmp entry_cmp);
/*lint -sem(        heap_free, 1p == 1) */
extern void         heap_free(Heap* heap);
/*lint -sem(        heap_is_valid, 1p == 1) */
extern Bool         heap_is_valid(const Heap* heap);
/*lint -sem(        heap_push_entry, 1p == 1 && 2p == 1) */
extern void         heap_push_entry(Heap* heap, Entry* entry);
/*lint -sem(        heap_pop_entry, 1p == 1, @p == 1) */
extern Entry*       heap_pop_entry(Heap* heap);
/*lint -sem(        heap_top_entry, 1p == 1, @p == 1) */
extern const Entry* heap_top_entry(const Heap* heap);
/*lint -sem(        heap_is_full, 1p == 1) */
extern Bool         heap_is_full(const Heap* heap);
/*lint -sem(        heap_is_empty, 1p == 1) */
extern Bool         heap_is_empty(const Heap* heap);

/* element.c
 */
#define ELEM_NULL  ((Elem*)0)

extern void         elem_init(void);
extern void         elem_exit(void);
/*lint -sem(        elem_new_numb, @p == 1) */
extern Elem*        elem_new_numb(const Numb* n);
/*lint -sem(        elem_new_strg, nulterm(1), 1p, @p == 1) */
extern Elem*        elem_new_strg(const char* s);
/*lint -sem(        elem_new_name, nulterm(1), 1p, @p == 1) */
extern Elem*        elem_new_name(const char* s);
/*lint -sem(        elem_free, custodial(1), 1p == 1) */
extern void         elem_free(Elem* elem);
/*lint -sem(        elem_is_valid, 1p == 1) */
extern Bool         elem_is_valid(const Elem* elem);
/*lint -sem(        elem_copy, 1p == 1, @p == 1) */
extern Elem*        elem_copy(const Elem* elem);
/*lint -sem(        elem_cmp, 1p == 1 && 2p == 1) */
extern Bool         elem_cmp(const Elem* elem_a, const Elem* elem_b);
/*lint -sem(        elem_get_type, 1p == 1) */
extern ElemType     elem_get_type(const Elem* elem);
/*lint -sem(        elem_get_numb, 1p == 1) */
extern const Numb*  elem_get_numb(const Elem* elem);
/*lint -sem(        elem_get_strg, 1p == 1, @p && nulterm(@)) */
extern const char*  elem_get_strg(const Elem* elem);
/*lint -sem(        elem_get_name, 1p == 1, @p && nulterm(@)) */
extern const char*  elem_get_name(const Elem* elem);
/*lint -sem(        elem_print, 1p == 1 && 2p == 1) */
extern void         elem_print(FILE* fp, const Elem* elem);
/*lint -sem(        elem_hash, 1p == 1) */
extern unsigned int elem_hash(const Elem* elem);
/*lint -sem(        elem_tostr, 1p == 1, @p && nulterm(@)) */
extern char*        elem_tostr(const Elem* elem);

/* tuple.c
 */
#define TUPLE_NULL ((Tuple*)0)

/*lint -sem(        tuple_new, 1n >= 0, @p == 1) */
extern Tuple*       tuple_new(int dim);
/*lint -sem(        tuple_free, custodial(1), 1p == 1) */
extern void         tuple_free(Tuple* tuple);
/*lint -sem(        tuple_is_valid, 1p == 1) */
extern Bool         tuple_is_valid(const Tuple* tuple);
/*lint -sem(        tuple_copy, 1p == 1, @p == 1) */
extern Tuple*       tuple_copy(const Tuple* tuple);
/*lint -sem(        tuple_cmp, 1p == 1 && 2p == 1) */
extern Bool         tuple_cmp(const Tuple* tuple_a, const Tuple* tuple_b);
/*lint -sem(        tuple_get_dim, 1p == 1, @n > 0) */
extern int          tuple_get_dim(const Tuple* tuple);
/*lint -sem(        tuple_set_elem, custodial(3), 1p == 1 && 2n >= 0 && 3p == 1) */
extern void         tuple_set_elem(Tuple* tuple, int idx, Elem* elem);
/*lint -sem(        tuple_get_elem, 1p == 1 && 2n >= 0, @p == 1) */
extern const Elem*  tuple_get_elem(const Tuple* tuple, int idx);
/*lint -sem(        tuple_combine, 1p == 1 && 2p == 1, @p == 1) */
extern Tuple*       tuple_combine(const Tuple* ta, const Tuple* tb);
/*lint -sem(        tuple_print, 1p == 1 && 2p == 1) */
extern void         tuple_print(FILE* fp, const Tuple* tuple);
/*lint -sem(        tuple_hash, 1p == 1) */
extern unsigned int tuple_hash(const Tuple* tuple);
/*lint -sem(        tuple_tostr, 1p == 1, @p && nulterm(@)) */
extern char*        tuple_tostr(const Tuple* tuple);

/* set.c
 */
extern void         set_init(void);
extern void         set_exit(void);
/*lint -sem(        set_free, custodial(1), 1p == 1) */
extern void         set_free(Set* set);
/*lint -sem(        set_is_valid, 1p == 1) */
extern Bool         set_is_valid(const Set* set);
/*lint -sem(        set_copy, 1p == 1, @p == 1) */
extern Set*         set_copy(const Set* set);
/*lint -sem(        set_lookup, 1p == 1 && 2p == 1) */
extern Bool         set_lookup(const Set* set, const Tuple* tuple);
/*lint -sem(        set_iter_init, 1p == 1, @p == 1) */
extern SetIter*     set_iter_init(const Set* set, const Tuple* pattern);
/*lint -sem(        set_iter_next, 1p == 1 && 2p == 1) */
extern Tuple*       set_iter_next(SetIter* iter, const Set* set);
/*lint -sem(        set_iter_exit, 1p == 1 && 2p == 1) */
extern void         set_iter_exit(SetIter* iter, const Set* set);
/*lint -sem(        set_get_dim, 1p == 1, @n >= 0) */
extern int          set_get_dim(const Set* set);
/*lint -sem(        set_get_members, 1p == 1, @n >= 0) */
extern int          set_get_members(const Set* set);
/*lint -sem(        set_get_tuple, 1p == 1 && 2n >= 0) */
extern Tuple*       set_get_tuple(const Set* set, int idx);
/*lint -sem(        set_print, 1p == 1 && 2p == 1) */
extern void         set_print(FILE* fp, const Set* set);

/*lint -sem(        set_empty_new, 1n >= 0, @p == 1) */
extern Set*         set_empty_new(int dim);
/*lint -sem(        set_pseudo_new, @p == 1) */
extern Set*         set_pseudo_new(void);
/*lint -sem(        set_new_from_list, 1p == 1, @p == 1) */
extern Set*         set_new_from_list(const List* list, SetCheckType check);
/*lint -sem(        set_range_new, @p == 1) */
extern Set*         set_range_new(int begin, int end, int step);
/*lint -sem(        set_prod_new, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_prod_new(const Set* a, const Set* b);
/*lint -sem(        set_union, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_union(const Set* seta, const Set* setb);
/*lint -sem(        set_inter, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_inter(const Set* set_a, const Set* set_b);
/*lint -sem(        set_minus, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_minus(const Set* set_a, const Set* set_b);
/*lint -sem(        set_sdiff, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_sdiff(const Set* set_a, const Set* set_b);
/*lint -sem(        set_proj, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_proj(const Set* set_a, const Tuple* pattern);
/*lint -sem(        set_is_subseteq, 1p == 1 && 2p == 1) */
extern Bool         set_is_subseteq(const Set* set_a, const Set* set_b);
/*lint -sem(        set_is_subset, 1p == 1 && 2p == 1) */
extern Bool         set_is_subset(const Set* set_a, const Set* set_b);
/*lint -sem(        set_is_equal, 1p == 1 && 2p == 1) */
extern Bool         set_is_equal(const Set* set_a, const Set* set_b);
/*lint -sem(        set_subset_list, 1p == 1 && 2n > 0 && 4p == 1, @p == 1) */
extern List*        set_subsets_list(
   const Set* set, int subset_size, List* list, int* idx);


/* entry.c
 */
#define ENTRY_NULL ((Entry*)0)

/*lint -sem(        entry_new_numb, 1p == 1, @p == 1) */
extern Entry*       entry_new_numb(const Tuple* tuple, const Numb* numb);
/*lint -sem(        entry_new_strg, nulterm(2), 1p == 1 && 2p, @p == 1) */
extern Entry*       entry_new_strg(const Tuple* tuple, const char* strg);
/*lint -sem(        entry_new_set, 1p == 1 && 2p == 1, @p == 1) */
extern Entry*       entry_new_set(const Tuple* tuple, const Set* set);
/*lint -sem(        entry_new_var, 1p == 1 && 2p == 1, @p == 1) */
extern Entry*       entry_new_var(const Tuple* tuple, Var* var);
/*lint -sem(        entry_free, custodial(1), 1p == 1) */
extern void         entry_free(Entry* entry);
/*lint -sem(        entry_is_valid, 1p == 1) */
extern Bool         entry_is_valid(const Entry* entry);
/*lint -sem(        entry_copy, 1p == 1, @p == 1) */
extern Entry*       entry_copy(const Entry* entry);
/*lint -sem(        entry_cmp, 1p == 1 && 2p == 1) */
extern Bool         entry_cmp(const Entry* entry, const Tuple* tuple);
/*lint -sem(        entry_get_type, 1p == 1, @n >= 0) */
extern SymbolType   entry_get_type(const Entry* entry);
/*lint -sem(        entry_get_tuple, 1p == 1, @p == 1) */
extern const Tuple* entry_get_tuple(const Entry* entry);
/*lint -sem(        entry_get_numb, 1p == 1) */
extern const Numb*  entry_get_numb(const Entry* entry);
/*lint -sem(        entry_get_strg, 1p == 1, @p && nulterm(@)) */
extern const char*  entry_get_strg(const Entry* entry);
/*lint -sem(        entry_get_set, 1p == 1, @p == 1) */
extern const Set*   entry_get_set(const Entry* entry);
/*lint -sem(        entry_get_var, 1p == 1, @p == 1) */
extern Var*         entry_get_var(const Entry* entry);
/*lint -sem(        entry_print, 1p == 1 && 2p == 1) */
extern void         entry_print(FILE* fp, const Entry* entry);

/* symbol.c
 */
#define SYMBOL_NAME_INTERNAL  "@@"

/*lint -sem(        symbol_new, nulterm(1), 1p && 3p == 1 && 4n >= 0, @p == 1) */
extern Symbol*      symbol_new(const char* name,
   SymbolType type, const Set* set, int estimated_size, const Entry* deflt);
extern void         symbol_exit(void);
/*lint -sem(        symbol_is_valid, 1p == 1) */
extern Bool         symbol_is_valid(const Symbol* symbol);
/*lint -sem(        symbol_lookup, nulterm(1), 1p, r_null) */
extern Symbol*      symbol_lookup(const char* name);
/*lint -sem(        symbol_has_entry, 1p == 1 && 2p == 1) */
extern Bool         symbol_has_entry(const Symbol* sym, const Tuple* tuple);
/*lint -sem(        symbol_lookup_entry, 1p == 1 && 2p == 1) */
extern const Entry* symbol_lookup_entry(const Symbol* sym, const Tuple* tuple);
/*lint -sem(        symbol_add_entry, custodial(2), 1p == 1 && 2p == 1) */
extern void         symbol_add_entry(Symbol* sym, Entry* entry);
/*lint -sem(        symbol_get_dim, 1p == 1, @n >= 0) */
extern int          symbol_get_dim(const Symbol* sym);
/*lint -sem(        symbol_get_iset, 1p == 1, @p == 1) */
extern const Set*   symbol_get_iset(const Symbol* sym);
/*lint -sem(        symbol_get_name, 1p == 1, @p && nulterm(@)) */
extern const char*  symbol_get_name(const Symbol* sym);
/*lint -sem(        symbol_get_type, 1p == 1) */
extern SymbolType   symbol_get_type(const Symbol* sym);
/*lint -sem(        symbol_get_numb, 1p == 1) */
extern const Numb*  symbol_get_numb(const Symbol* sym, int idx);
/*lint -sem(        symbol_get_strg, 1p == 1, @p && nulterm(@)) */
extern const char*  symbol_get_strg(const Symbol* sym, int idx);
/*lint -sem(        symbol_get_set, 1p == 1, @p == 1) */
extern const Set*   symbol_get_set(const Symbol* sym, int idx);
/*lint -sem(        symbol_get_var, 1p == 1, @p == 1) */
extern Var*         symbol_get_var(const Symbol* sym, int idx);
/*lint -sem(        symbol_print, 1p == 1 && 2p == 1) */
extern void         symbol_print(FILE* fp, const Symbol* sym);
/*lint -sem(        symbol_print_all, 1p == 1) */
extern void         symbol_print_all(FILE* fp);

/* define.c
 */
/*lint -sem(        define_new, nulterm(1), 1p, @p == 1) */
extern Define*      define_new(const char* name, DefineType type);
/*lint -sem(        define_set_param, custodial(2), 1p == 1 && 2p == 1) */
extern void         define_set_param(Define* def, Tuple* param);
/*lint -sem(        define_set_code, 1p == 1 && 2p == 1) */
extern void         define_set_code(Define* def, CodeNode* code);
extern void         define_exit(void);
/*lint -sem(        define_is_valid, 1p == 1) */
extern Bool         define_is_valid(const Define* def);
/*lint -sem(        define_lookup, nulterm(1), 1p, r_null) */
extern Define*      define_lookup(const char* name);
/*lint -sem(        define_get_name, 1p == 1, @p && nulterm(@)) */
extern const char*  define_get_name(const Define* def);
/*lint -sem(        define_get_type, 1p == 1) */
extern DefineType   define_get_type(const Define* def);
/*lint -sem(        define_get_param, 1p == 1) */
extern const Tuple* define_get_param(const Define* def);
/*lint -sem(        define_get_code, 1p == 1) */
extern CodeNode*    define_get_code(const Define* def);

/* idxset.c
 */
/*lint -sem(        idxset_new, 1p == 1 && 2p == 1 && 3p == 1, @p == 1) */
extern IdxSet*      idxset_new(
   const Tuple* tuple, const Set* set, CodeNode* lexpr, Bool is_unrestricted);
/*lint -sem(        idxset_free, custodial(1), 1p == 1) */
extern void         idxset_free(IdxSet* idxset);
/*lint -sem(        idxset_is_valid, 1p == 1) */
extern Bool         idxset_is_valid(const IdxSet* idxset);
/*lint -sem(        idxset_copy, 1p == 1, @p == 1) */
extern IdxSet*      idxset_copy(const IdxSet* source);
/*lint -sem(        idxset_get_lexpr, 1p == 1, @p == 1) */
extern CodeNode*    idxset_get_lexpr(const IdxSet* idxset);
/*lint -sem(        idxset_get_tuple, 1p == 1, @p == 1) */
extern const Tuple* idxset_get_tuple(const IdxSet* idxset);
/*lint -sem(        idxset_get_set, 1p == 1, @p == 1) */
extern const Set*   idxset_get_set(const IdxSet* idxset);
/*lint -sem(        idxset_is_unrestricted, 1p == 1) */
extern Bool         idxset_is_unrestricted(const IdxSet* idxset);
/*lint -sem(        idxset_print, 1p == 1 && 2p == 1) */
extern void         idxset_print(FILE* fp, const IdxSet* idxset);

/* local.c
 */
extern void         local_init(void);
extern void         local_exit(void);
extern void         local_drop_frame(void);
/*lint -sem(        local_lookup, nulterm(1), 1p, r_null) */
extern const Elem*  local_lookup(const char* name);
/*lint -sem(        local_install_tuple, 1p == 1 && 2p == 1) */
extern void         local_install_tuple(const Tuple* patt, const Tuple* vals);
/*lint -sem(        local_print_all, 1p == 1) */
extern void         local_print_all(FILE* fp);
/*lint -sem(        local_tostrall, @p && nulterm(@)) */
extern char*        local_tostrall(void);

/* term.c
 */
#define TERM_PRINT_SYMBOL  1

/*lint -sem(        term_new, 1n > 0, @p == 1) */
extern Term*        term_new(int size);
/*lint -sem(        term_add_elem, 1p == 1 && 2p == 1) */
extern void         term_add_elem(Term* term, const Entry* entry, const Numb* coeff);
/*lint -sem(        term_free, custodial(1), 1p == 1) */
extern void         term_free(Term* term);
/*lint -sem(        term_is_valid, 1p == 1) */
extern Bool         term_is_valid(const Term* term);
/*lint -sem(        term_copy, 1p == 1, @p == 1) */
extern Term*        term_copy(const Term* term);
/*lint -sem(        term_print, 1p == 1 && 2p == 1 && 3n >= 1) */
extern void         term_print(FILE* fp, const Term* term, int flag);
/*lint -sem(        term_append_term, 1p == 1 && 2p == 1) */
extern void         term_append_term(Term* term_a, const Term* term_b);
/*lint -sem(        term_add_term, 1p == 1 && 2p == 1, @p == 1) */
extern Term*        term_add_term(const Term* term_a, const Term* term_b);
/*lint -sem(        term_sub_term, 1p == 1 && 2p == 1, @p == 1) */
extern Term*        term_sub_term(const Term* term_a, const Term* term_b);
/*lint -sem(        term_add_constant, 1p == 1 && 2p == 1) */
extern void         term_add_constant(Term* term, const Numb* value);
/*lint -sem(        term_sub_constant, 1p == 1 && 2p == 1) */
extern void         term_sub_constant(Term* term, const Numb* value);
/*lint -sem(        term_mul_coeff, 1p == 1 && 2p == 1) */
extern void         term_mul_coeff(Term* term, const Numb* value);
/*lint -sem(        term_get_constant, 1p == 1) */
extern const Numb*  term_get_constant(const Term* term);
/*lint -sem(        term_negate, 1p == 1) */
extern void         term_negate(Term* term);
/*lint -sem(        term_to_objective, 1p == 1) */
extern void         term_to_objective(const Term* term);
/*lint -sem(        term_to_nzo, 1p == 1 && 2p == 1) */
extern void         term_to_nzo(const Term* term, Con* con);
/*lint -sem(        term_to_sos, 1p == 1 && 2p == 1) */
extern Bool         term_to_sos(const Term* term, Sos* sos);
/*lint -sem(        term_get_elements, 1p == 1, @n >= 0) */
extern int          term_get_elements(const Term* term);
/*lint -sem(        term_get_lower_bound, 1p == 1, @p == 1) */
extern Bound*       term_get_lower_bound(const Term* term);
/*lint -sem(        term_get_upper_bound, 1p == 1, @p == 1) */
extern Bound*       term_get_upper_bound(const Term* term);
/*lint -sem(        term_is_all_integer, 1p == 1) */
extern Bool         term_is_all_integer(const Term* term);

/* rdefpar.c
 */
/*lint -sem(       rdef_new, nulterm(1), nulterm(2), 1p && 2p, @p == 1) */
extern RDef*       rdef_new(const char* filename, const char* template);
/*lint -sem(       rdef_free, custodial(1), 1p == 1) */
extern void        rdef_free(RDef* rdef);
/*lint -sem(       rdef_is_valid, 1p == 1) */
extern Bool        rdef_is_valid(const RDef* rdef);
/*lint -sem(       rdef_copy, 1p == 1, @p == 1) */
extern RDef*       rdef_copy(const RDef* rdef);
/*lint -sem(       rdef_set_param, 1p == 1 && 2p == 1) */
extern void        rdef_set_param(RDef* rdef, const RPar* rpar);
/*lint -sem(       rdef_get_filename, 1p == 1, @p && nulterm(@)) */
extern const char* rdef_get_filename(const RDef* rdef);
/*lint -sem(       rdef_get_template, 1p == 1, @p && nulterm(@)) */
extern const char* rdef_get_template(const RDef* rdef);
/*lint -sem(       rdef_get_comment, 1p == 1, @p && nulterm(@)) */
extern const char* rdef_get_comment(const RDef* rdef);
/*lint -sem(       rdef_get_use, 1p == 1) */
extern int         rdef_get_use(const RDef* rdef);
/*lint -sem(       rdef_get_skip, 1p == 1) */
extern int         rdef_get_skip(const RDef* rdef);

/*lint -sem(       rpar_new_skip, @p == 1) */
extern RPar*       rpar_new_skip(int skip);
/*lint -sem(       rpar_new_use, @p == 1) */
extern RPar*       rpar_new_use(int use);
/*lint -sem(       rpar_new_comment, 1p && nulterm(1), @p == 1) */
extern RPar*       rpar_new_comment(const char* comment);
/*lint -sem(       rpar_free, custodial(1), 1p == 1) */
extern void        rpar_free(RPar* rpar);
/*lint -sem(       rpar_is_valid, 1p == 1) */
extern Bool        rpar_is_valid(const RPar* rpar);
/*lint -sem(       rpar_copy, 1p == 1, @p == 1) */
extern RPar*       rpar_copy(const RPar* rpar);

/* conname.c
 */
extern void        conname_format(ConNameForm format);
extern void        conname_free(void);
/*lint -sem(       conname_set, nulterm(1), 1p) */
extern Bool        conname_set(const char* prefix);
/*lint -sem(       conname_set, @p && nulterm(@)) */
extern const char* conname_get(void);
extern void        conname_next(void);

/* stmt.c
 */
/*lint -sem(        stmt_new, nulterm(2), nulterm(4),
                    2p && 3n >= 0 && 4p, @p == 1) */
extern Stmt*        stmt_new(StmtType type, const char* filename, int lineno,
   const char* text);
/*lint -sem(        stmt_free, custodial(1), 1p == 1) */
extern void         stmt_free(Stmt* stmt);
/*lint -sem(        stmt_is_valid, 1p == 1) */
extern Bool         stmt_is_valid(const Stmt* stmt);
/*lint -sem(        stmt_get_filename, 1p == 1, @p && nulterm(@)) */
extern const char*  stmt_get_filename(const Stmt* stmt);
/*lint -sem(        stmt_get_lineno, 1p == 1, @n > 0) */
extern int          stmt_get_lineno(const Stmt* stmt);
/*lint -sem(        stmt_get_text, 1p == 1, @p && nulterm(@)) */
extern const char*  stmt_get_text(const Stmt* stmt);
/*lint -sem(        stmt_parse, 1p == 1) */
extern void         stmt_parse(Stmt* stmt);
/*lint -sem(        stmt_execute, 1p == 1) */
extern void         stmt_execute(const Stmt* stmt);
/*lint -sem(        stmt_print, 1p == 1 && 2p == 1) */
extern void         stmt_print(FILE* fp, const Stmt* stmt);
/*lint -sem(        stmt_trigger_warning, 1n >= 0) */
extern Bool         stmt_trigger_warning(int no);

/* prog.c
 */
/*lint -sem(        prog_new, 1p) */
extern Prog*        prog_new(void);
/*lint -sem(        prog_free, custodial(1), 1p == 1) */
extern void         prog_free(Prog* prog);
/*lint -sem(        prog_is_valid, 1p == 1) */
extern Bool         prog_is_valid(const Prog* prog);
/*lint -sem(        prog_is_empty, 1p == 1) */
extern Bool         prog_is_empty(const Prog* prog);
/*lint -sem(        prog_add_stmt, custodial(2), 1p == 1 && 2p == 1) */
extern void         prog_add_stmt(Prog* prog, Stmt* stmt);
/*lint -sem(        prog_print, 1p == 1 && 2p == 1) */
extern void         prog_print(FILE* fp, const Prog* prog);
/*lint -sem(        prog_execute, 1p == 1) */
extern void         prog_execute(const Prog* prog);

/* load.c
 */
/*lint -sem(        prog_load, nulterm(2), 1p == 1 && 2p) */
extern void         prog_load(Prog* prog, const char* filename);

/* source.c
 */
/*lint -sem(        shwo_source, nulterm(2), 1p == 1 && 2p) */
extern void         show_source(FILE* fp, const char* text, int column);

/* metaio.c
 */
/*lint -sem(        mio_add_strg_file, nulterm(1), nulterm(2), 1p && 2p) */
extern void         mio_add_strg_file(const char* name, const char* content, Bool use_copy);
extern void         mio_init(void);
extern void         mio_exit(void);
/*lint -sem(        mio_open, nulterm(2), 2p) */
/*lint -function(   fopen(1), mio_open(1)) */
/*lint -function(   fopen(r), mio_open(r)) */
extern MFP*         mio_open(const char* name, const char* ext);
/*lint -function(   fclose, mio_close) */
extern void         mio_close(MFP* mfp);
/*lint -function(   fgetc, mio_getc) */
extern int          mio_getc(const MFP* mfp);
/*lint -function(   fgets(1), mio_gets(2))) */
/*lint -function(   fgets(2), mio_gets(3))) */
/*lint -function(   fgets(3), mio_gets(1))) */
/*lint -function(   fgets(r), mio_gets(r))) */
extern char*        mio_gets(const MFP* mfp, char* buf, int len);
/*lint -sem(        mio_get_line, 1p) */
extern char*        mio_get_line(const MFP* mfp);

/* vinst.c
 */
extern void interns_init(void);
extern void interns_exit(void);

/* mmlparse.y
 */
extern int          yyparse(void);

/* mmlscan.l
 */
/*lint -sem(        parse_stmt, 1p) */
extern void         parse_stmt(const Stmt* stmt);
extern const Stmt*  scan_get_stmt(void);
extern int          scan_get_column(void);

#define Min(a, b)    (((a) <= (b)) ? (a) : (b)) 
#define Sgn(a)       (((a) > 0) ? 1 : (((a) < 0) ? -1 : 0))

/* Directory separator, so we could redefine it for Windoof.
 */
#ifndef DIRSEP
#define DIRSEP '/'
#endif /* DIRSEP */

#ifndef NDEBUG
#define SID unsigned int sid;
#define SID_set(p, id)  (p->sid = id)
#define SID_del(p)      (p->sid = 0xffffffff)
#define SID_ok(p, id)   (p->sid == id)
#define SID_set2(p, id) (p.sid = id)
#define SID_del2(p)     (p.sid = 0xffffffff)
#define SID_ok2(p, id)  (p.sid == id)
#else /* NDEBUG */
#define SID              /* */
#define SID_set(p, sid)  /* */
#define SID_del(p)       /* */
#define SID_ok(p, id)    TRUE
#define SID_set2(p, sid) /* */
#define SID_del2(p)      /* */
#define SID_ok2(p, id)   TRUE
#endif /* NDEBUG */

#define DISPERSE(x) (1664525U * (x) + 1013904223U);

#ifdef TRACE
#define Trace(fname) fprintf(stderr, "Trace: %s\n", fname);
#else
#define Trace(fname) /* */
#endif /* TRACE */

#ifdef __cplusplus
}
#endif

#endif /* _MME_H_ */
