#ident "@(#) $Id: mme.h,v 1.12 2002/06/18 09:13:09 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mme.h                                                         */
/*   Name....: Mathematical Modelling Engine                                 */
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
#ifndef _MME_H_
#define _MME_H_

enum element_type    { ELEM_ERR = 0, ELEM_NUMB, ELEM_STRG, ELEM_NAME };
enum symbol_type     { SYM_ERR = 0, SYM_NUMB, SYM_STRG, SYM_SET, SYM_VAR };
enum hash_type       { HASH_ERR = 0, HASH_TUPLE, HASH_ENTRY };
enum lp_direct       { LP_MIN = 0, LP_MAX };
enum lp_type         { LP_ERR = 0, LP_LP, LP_IP };
enum lp_form         { LP_FORM_ERR = 0, LP_FORM_LPF, LP_FORM_MPS };
enum con_type        { CON_ERR = 0, CON_EQ, CON_LE, CON_GE };
enum var_type        { VAR_ERR = 0, VAR_CON, VAR_INT, VAR_BIN };

enum statement_type
{
   STMT_ERR = 0, STMT_SET, STMT_PARAM, STMT_VAR, STMT_MIN, STMT_MAX,
   STMT_CONS, STMT_DATA
};

enum code_type
{
   CODE_ERR = 0, CODE_NUMB, CODE_STRG, CODE_NAME, CODE_TUPLE,
   CODE_SET, CODE_TERM, CODE_BOOL, CODE_SIZE, 
   CODE_IDXSET, CODE_LIST, CODE_VOID, CODE_ENTRY, CODE_VARTYPE, CODE_CONTYPE,
   CODE_RDEF, CODE_RPAR, CODE_BITS
};

typedef enum element_type        ElemType;
typedef struct element           Elem;
typedef struct tuple             Tuple;
typedef struct set               Set;
typedef struct entry             Entry;
typedef enum symbol_type         SymbolType;
typedef struct symbol            Symbol;
typedef struct index_set         IdxSet;
typedef struct term              Term;
typedef struct local             Local;
typedef struct list_element      ListElem;
typedef struct list              List;
typedef enum hash_type           HashType;
typedef struct hash              Hash;
typedef struct read_param        RPar;
typedef struct read_definition   RDef;

typedef struct nonzero           Nzo;
typedef struct variable          Var;
typedef struct constraint        Con;
typedef struct lpstorage         Lps;
typedef enum lp_direct           LpDirect;
typedef enum lp_type             LpType;
typedef enum lp_form             LpForm;
typedef enum con_type            ConType;
typedef enum var_type            VarType;

typedef enum statement_type      StmtType;
typedef struct statement         Stmt;
typedef struct program           Prog;

typedef enum code_type           CodeType;
typedef struct code_node         CodeNode;

typedef void                   (*Inst)(CodeNode* self);

extern Bool         verbose;
extern Bool         zpldebug;

extern void         str_init(void);
extern void         str_exit(void);
/*lint -sem(        str_new, 1p, @p == 1p) */
extern const char*  str_new(const char* s);
/*lint -sem(        str_hash, 1p)           */
extern unsigned int str_hash(const char* s);

/* list.c
 */
/*lint -sem(        list_new_elem, 1p == 1, @p == 1) */
extern List*        list_new_elem(const Elem* elem);
/*lint -sem(        list_new_tuple, 1p == 1, @p == 1) */
extern List*        list_new_tuple(Tuple* tuple);
/*lint -sem(        list_new_entry, 1p == 1, @p == 1) */
extern List*        list_new_entry(Entry* entry);
/*lint -sem(        list_free, 1p == 1) */
extern void         list_free(List* list);
/*lint -sem(        list_is_valid, 1p == 1) */
extern Bool         list_is_valid(const List* list);
/*lint -sem(        list_is_entrylist, 1p == 1) */
extern Bool         list_is_entrylist(const List* list);
/*lint -sem(        list_is_tuplelist, 1p == 1) */
extern Bool         list_is_tuplelist(const List* list);
/*lint -sem(        list_copy, 1p == 1, @P == 1) */
extern List*        list_copy(List* list);
/*lint -sem(        list_add_elem, 1p == 1 && 2p == 1) */
extern void         list_add_elem(List* list, const Elem* elem);
/*lint -sem(        list_add_tuple, 1p == 1 && 2p == 1) */
extern void         list_add_tuple(List* list, Tuple* tuple);
/*lint -sem(        list_add_entry, 1p == 1 && 2p == 1) */
extern void         list_add_entry(List* list, Entry* entry);
/*lint -sem(        list_get_elems, 1p == 1, @n >= 0) */
extern int          list_get_elems(const List* list);
/*lint -sem(        list_get_elem,  1p == 1, @p == 1) */
extern const Elem*  list_get_elem(const List* list, ListElem** idxp);
/*lint -sem(        list_get_tuple, 1p == 1, @p == 1) */
extern Tuple*       list_get_tuple(const List* list, ListElem** idxp);
/*lint -sem(        list_get_entry, 1p == 1, @p == 1) */
extern Entry*       list_get_entry(const List* list, ListElem** idxp);
/*lint -sem(        list_print, 1p == 1 && 2p == 1) */
extern void         list_print(FILE* fp, const List* list);

/* hash
 */
/*lint -sem(        hash_new, @p == 1) */
extern Hash*        hash_new(HashType type);
/*lint -sem(        hash_free, 1p == 1) */
extern void         hash_free(Hash* hash);
/*lint -sem(        hash_is_valid, 1p == 1) */
extern Bool         hash_is_valid(const Hash* hash);
/*lint -sem(        hash_add_tuple, 1p == 1 && 2p == 1) */
extern void         hash_add_tuple(Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_add_entry, 1p == 1 && 2p == 1) */
extern void         hash_add_entry(Hash* hash, const Entry* entry);
/*lint -sem(        hash_has_tuple, 1p == 1 && 2p == 1) */
extern Bool         hash_has_tuple(const Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_has_entry, 1p == 1 && 2p == 1) */
extern Bool         hash_has_entry(const Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_lookup_entry, 1p == 1 && 2p == 1, @p == 1) */
extern const Entry* hash_lookup_entry(const Hash* hash, const Tuple* tuple);

/* element.c
 */
#define ELEM_NULL  ((Elem*)0)

extern void         elem_init(void);
extern void         elem_exit(void);
/*lint -sem(        elem_new_numb, @p == 1) */
extern const Elem*  elem_new_numb(double n);
/*lint -sem(        elem_new_strg, 1p, @p == 1) */
extern const Elem*  elem_new_strg(const char* s);
/*lint -sem(        elem_new_name, 1p, @p == 1) */
extern const Elem*  elem_new_name(const char* s);
/*lint -sem(        elem_is_valid, 1p == 1) */
extern Bool         elem_is_valid(const Elem* elem);
/*lint -sem(        elem_cmp, 1p == 1 && 2p == 1) */
extern Bool         elem_cmp(const Elem* elem_a, const Elem* elem_b);
/*lint -sem(        elem_get_type, 1p == 1) */
extern ElemType     elem_get_type(const Elem* elem);
/*lint -sem(        elem_get_numb, 1p == 1) */
extern double       elem_get_numb(const Elem* elem);
/*lint -sem(        elem_get_strg, 1p == 1, @p) */
extern const char*  elem_get_strg(const Elem* elem);
/*lint -sem(        elem_get_name, 1p == 1, @p) */
extern const char*  elem_get_name(const Elem* elem);
/*lint -sem(        elem_print, 1p == 1 && 2p == 1) */
extern void         elem_print(FILE* fp, const Elem* elem);
/*lint -sem(        elem_hash, 1p == 1) */
extern unsigned int elem_hash(const Elem* elem);
/*lint -sem(        elem_tostr, 1p == 1, @p > 0) */
extern char*        elem_tostr(const Elem* elem);

/* tuple.c
 */
#define TUPLE_NULL ((Tuple*)0)

/*lint -sem(        tuple_new, 1n >= 0, @p == 1) */
extern Tuple*       tuple_new(int dim);
/*lint -sem(        tuple_free, 1p == 1) */
extern void         tuple_free(Tuple* tuple);
/*lint -sem(        tuple_is_valid, 1p == 1) */
extern Bool         tuple_is_valid(const Tuple* tuple);
/*lint -sem(        tuple_copy, 1p == 1, @p == 1) */
extern Tuple*       tuple_copy(Tuple* tuple);
/*lint -sem(        tuple_cmp, 1p == 1 && 2p == 1) */
extern Bool         tuple_cmp(const Tuple* tuple_a, const Tuple* tuple_b);
/*lint -sem(        tuple_get_dim, 1p == 1, @n >= 0) */
extern int          tuple_get_dim(const Tuple* tuple);
/*lint -sem(        tuple_set_elem, 1p == 1 && 2n >= 0 && 3p == 1) */
extern void         tuple_set_elem(
   const Tuple* tuple, int idx, const Elem* elem);
/*lint -sem(        tuple_get_elem, 1p == 1 && 2n >= 0, @p == 1) */
extern const Elem*  tuple_get_elem(const Tuple* tuple, int idx);
/*lint -sem(        tuple_combine, 1p == 1 && 2p == 1, @p == 1) */
extern Tuple*       tuple_combine(const Tuple* ta, const Tuple* tb);
/*lint -sem(        tuple_print, 1p == 1 && 2p == 1) */
extern void         tuple_print(FILE* fp, const Tuple* tuple);
/*lint -sem(        tuple_hash, 1p == 1) */
extern unsigned int tuple_hash(const Tuple* tuple);
/*lint -sem(        tuple_tostr, 1p == 1, @p > 0) */
extern char*        tuple_tostr(const Tuple* tuple);

/* set.c
 */
#define SET_ADD_BEGIN  0
#define SET_ADD_END    1

/*lint -sem(        set_new, 1n >= 0, @p == 1) */
extern Set*         set_new(int dim);
/*lint -sem(        set_free, 1p == 1) */
extern void         set_free(Set* set);
/*lint -sem(        set_is_valid, 1p == 1) */
extern Bool         set_is_valid(const Set* set);
/*lint -sem(        set_copy, 1p == 1, @p == 1) */
extern Set*         set_copy(Set* set);
/*lint -sem(        set_add_member, 1p == 1 && 2p == 1 && 3n >= 0, @p == 1) */
extern void         set_add_member(Set* set, Tuple* tuple, int where);
/*lint -sem(        set_lookup, 1p == 1 && 2p == 1) */
extern Bool         set_lookup(const Set* set, const Tuple* tuple);
/*lint -sem(        set_match, 1p == 1 && 2p == 1 && 3p == 1, @p == 1) */
extern Tuple*       set_match_next(const Set* set, const Tuple* pat, int* idx);
/*lint -sem(        set_get_dim, 1p == 1, @n >= 0) */
extern int          set_get_dim(const Set* set);
/*lint -sem(        set_get_used, 1p == 1, @n >= 0) */
extern int          set_get_used(const Set* set);
/*lint -sem(        set_print, 1p == 1 && 2p == 1) */
extern void         set_print(FILE* fp, const Set* set);
/*lint -sem(        set_range, @p == 1) */
extern Set*         set_range(double start, double end, double step);
/*lint -sem(        set_cross, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_cross(const Set* seta, const Set* setb);
/*lint -sem(        set_union, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_union(const Set* seta, const Set* setb);
/*lint -sem(        set_inter, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_inter(const Set* set_a, const Set* set_b);
/*lint -sem(        set_minus, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_minus(const Set* set_a, const Set* set_b);
/*lint -sem(        set_sdiff, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_sdiff(const Set* set_a, const Set* set_b);
/*lint -sem(        set_is_subseteq, 1p == 1 && 2p == 1) */
extern Bool         set_is_subseteq(const Set* set_a, const Set* set_b);
/*lint -sem(        set_is_subset, 1p == 1 && 2p == 1) */
extern Bool         set_is_subset(const Set* set_a, const Set* set_b);
/*lint -sem(        set_is_equal, 1p == 1 && 2p == 1) */
extern Bool         set_is_equal(const Set* set_a, const Set* set_b);

/* entry.c
 */
#define ENTRY_NULL ((Entry*)0)

/*lint -sem(        entry_new_numb, 1p == 1, @p == 1) */
extern Entry*       entry_new_numb(Tuple* tuple, double numb);
/*lint -sem(        entry_new_strg, 1p == 1 && 2p, @p == 1) */
extern Entry*       entry_new_strg(Tuple* tuple, const char* strg);
/*lint -sem(        entry_new_set, 1p == 1 && 2p == 1, @p == 1) */
extern Entry*       entry_new_set(Tuple* tuple, Set* set);
/*lint -sem(        entry_new_var, 1p == 1 && 2p == 1, @p == 1) */
extern Entry*       entry_new_var(Tuple* tuple, Var* var);
/*lint -sem(        entry_free, 1p == 1) */
extern void         entry_free(Entry* entry);
/*lint -sem(        entry_is_valid, 1p == 1) */
extern Bool         entry_is_valid(const Entry* entry);
/*lint -sem(        entry_copy, 1p == 1, @p == 1) */
extern Entry*       entry_copy(Entry* entry);
/*lint -sem(        entry_cmp, 1p == 1 && 2p == 1) */
extern Bool         entry_cmp(const Entry* entry, const Tuple* tuple);
/*lint -sem(        entry_set_index, 1p == 1 && 2n >= 0) */
extern void         entry_set_index(Entry* entry, int idx);
/*lint -sem(        entry_get_type, 1p == 1, @n >= 0) */
extern SymbolType   entry_get_type(const Entry* entry);
/*lint -sem(        entry_get_tuple, 1p == 1, @p == 1) */
extern Tuple*       entry_get_tuple(const Entry* entry);
/*lint -sem(        entry_get_index, 1p == 1, @n >= 0) */
extern int          entry_get_index(const Entry* entry);
/*lint -sem(        entry_get_numb, 1p == 1) */
extern double       entry_get_numb(const Entry* entry);
/*lint -sem(        entry_get_strg, 1p == 1, @p == 1) */
extern const char*  entry_get_strg(const Entry* entry);
/*lint -sem(        entry_get_set, 1p == 1, @p == 1) */
extern Set*         entry_get_set(const Entry* entry);
/*lint -sem(        entry_get_var, 1p == 1, @p == 1) */
extern Var*         entry_get_var(const Entry* entry);
/*lint -sem(        entry_print, 1p == 1 && 2p == 1) */
extern void         entry_print(FILE* fp, const Entry* entry);
/*lint -sem(        entry_hash, 1p == 1) */
extern unsigned int entry_hash(const Entry* entry);

/* symbol.c
 */
/*lint -sem(        symbol_new, 1p == 1 && 3p == 1, @p == 1) */
extern Symbol*      symbol_new(const char* name, SymbolType type, Set* set);
extern void         symbol_exit(void);
/*lint -sem(        symbol_is_valid, 1p == 1) */
extern Bool         symbol_is_valid(const Symbol* symbol);
/*lint -sem(        symbol_lookup, 1p == 1, r_null) */
extern Symbol*      symbol_lookup(const char* name);
/*lint -sem(        symbol_has_entry, 1p == 1 && 2p == 1) */
extern Bool         symbol_has_entry(const Symbol* sym, const Tuple* tuple);
/*lint -sem(        symbol_lookup_entry, 1p == 1 && 2p == 1, @p == 1) */
extern const Entry* symbol_lookup_entry(const Symbol* sym, const Tuple* tuple);
/*lint -sem(        symbol_add_entry, 1p == 1 && 2p == 1) */
extern void         symbol_add_entry(Symbol* sym, Entry* entry);
/*lint -sem(        symbol_get_dim, 1p == 1, @n >= 0) */
extern int          symbol_get_dim(const Symbol* sym);
/*lint -sem(        symbol_get_iset, 1p == 1, @p == 1) */
extern const Set*   symbol_get_iset(const Symbol* sym);
/*lint -sem(        symbol_get_name, 1p == 1, @p) */
extern const char*  symbol_get_name(const Symbol* sym);
/*lint -sem(        symbol_get_type, 1p == 1) */
extern SymbolType   symbol_get_type(const Symbol* sym);
/*lint -sem(        symbol_get_numb, 1p == 1) */
extern double       symbol_get_numb(const Symbol* sym, int idx);
/*lint -sem(        symbol_get_strg, 1p == 1, @p) */
extern const char*  symbol_get_strg(const Symbol* sym, int idx);
/*lint -sem(        symbol_get_set, 1p == 1, @p == 1) */
extern Set*         symbol_get_set(const Symbol* sym, int idx);
/*lint -sem(        symbol_get_var, 1p == 1, @p == 1) */
extern Var*         symbol_get_var(const Symbol* sym, int idx);
/*lint -sem(        symbol_print, 1p == 1 && 2p == 1) */
extern void         symbol_print(FILE* fp, const Symbol* sym);
/*lint -sem(        symbol_print_all, 1p == 1) */
extern void         symbol_print_all(FILE* fp);
/*lint -sem(        symbol_print_bounds, 1p == 1) */
extern void         symbol_print_bounds(FILE* fp);

/* idxset.c
 */
/*lint -sem(        idxset_new, 1p == 1 && 2p == 1 && 3p == 1, @p == 1) */
extern IdxSet*      idxset_new(Tuple* tuple, Set* set, CodeNode* lexpr);
/*lint -sem(        idxset_free, 1p == 1) */
extern void         idxset_free(IdxSet* idxset);
/*lint -sem(        idxset_is_valid, 1p == 1) */
extern Bool         idxset_is_valid(const IdxSet* idxset);
/*lint -sem(        idxset_copy, 1p == 1, @p == 1) */
extern IdxSet*      idxset_copy(const IdxSet* source);
/*lint -sem(        idxset_get_lexpr, 1p == 1, @p == 1) */
extern CodeNode*    idxset_get_lexpr(const IdxSet* idxset);
/*lint -sem(        idxset_get_tuple, 1p == 1, @p == 1) */
extern Tuple*       idxset_get_tuple(const IdxSet* idxset);
/*lint -sem(        idxset_get_set, 1p == 1, @p == 1) */
extern Set*         idxset_get_set(const IdxSet* idxset);
/*lint -sem(        idxset_print, 1p == 1 && 2p == 1) */
extern void         idxset_print(FILE* fp, const IdxSet* idxset);

/* local.c
 */
extern void         local_drop_frame(void);
/*lint -sem(        local_lookup, 1p == 1, r_null) */
extern const Elem*  local_lookup(const char* name);
/*lint -sem(        local_install_tuple, 1p == 1 && 2p == 1) */
extern void         local_install_tuple(const Tuple* patt, const Tuple* vals);
/*lint -sem(        local_print_all, 1p == 1) */
extern void         local_print_all(FILE* fp);

/* term.c
 */
#define TERM_PRINT_SYMBOL  1
#define TERM_PRINT_INDEX   2

/*lint -sem(        term_new, 1n > 0, @p == 1) */
extern Term*        term_new(int size);
/*lint -sem(        term_add_elem, 1p == 1 && 2p == 1) */
extern void         term_add_elem(Term* term, const Entry* entry, double coeff);
/*lint -sem(        term_free, 1p == 1) */
extern void         term_free(Term* term);
/*lint -sem(        term_is_valid, 1p == 1) */
extern Bool         term_is_valid(const Term* term);
/*lint -sem(        term_copy, 1p == 1, @p == 1) */
extern Term*        term_copy(const Term* term);
/*lint -sem(        term_print, 1p == 1 && 2p == 1 && 3n >= 1) */
extern void         term_print(FILE* fp, const Term* term, int flag);
/*lint -sem(        term_add_term, 1p == 1 && 2p == 1, @p == 1) */
extern Term*        term_add_term(const Term* term_a, const Term* term_b);
/*lint -sem(        term_add_constant, 1p == 1) */
extern void         term_add_constant(Term* term, double value);
/*lint -sem(        term_mul_coeff, 1p == 1) */
extern void         term_mul_coeff(Term* term, double value);
/*lint -sem(        term_get_constant, 1p == 1) */
extern double       term_get_constant(const Term* term);
/*lint -sem(        term_negate, 1p == 1) */
extern void         term_negate(Term* term);
/*lint -sem(        term_to_objective, 1p == 1) */
extern void         term_to_objective(const Term* term);
/*lint -sem(        term_to_nzo, 1p == 1 && 2p == 1) */
extern void         term_to_nzo(const Term* term, Con* con);

/* rdefpar.c
 */
/*lint -sem(       rdef_new, 1p && 2p, @p == 1) */
extern RDef*       rdef_new(const char* filename, const char* template);
/*lint -sem(       rdef_free, 1p == 1) */
extern void        rdef_free(RDef* rdef);
/*lint -sem(       rdef_is_valid, 1p == 1) */
extern Bool        rdef_is_valid(const RDef* rdef);
/*lint -sem(       rdef_copy, 1p == 1, @p == 1) */
extern RDef*       rdef_copy(RDef* rdef);
/*lint -sem(       rdef_set_param, 1p == 1 && 2p == 1) */
extern void        rdef_set_param(RDef* rdef, const RPar* rpar);
/*lint -sem(       rdef_get_filename, 1p == 1, @p) */
extern const char* rdef_get_filename(const RDef* rdef);
/*lint -sem(       rdef_get_template, 1p == 1, @p) */
extern const char* rdef_get_template(const RDef* rdef);
/*lint -sem(       rdef_get_comment, 1p == 1, @p) */
extern const char* rdef_get_comment(const RDef* rdef);
/*lint -sem(       rdef_get_use, 1p == 1) */
extern int         rdef_get_use(const RDef* rdef);
/*lint -sem(       rdef_get_skip, 1p == 1) */
extern int         rdef_get_skip(const RDef* rdef);

/*lint -sem(       rpar_new_skip, @p == 1) */
extern RPar*       rpar_new_skip(int skip);
/*lint -sem(       rpar_new_use, @p == 1) */
extern RPar*       rpar_new_use(int use);
/*lint -sem(       rpar_new_comment, 1p, @p == 1) */
extern RPar*       rpar_new_comment(const char* comment);
/*lint -sem(       rpar_free, 1p == 1) */
extern void        rpar_free(RPar* rpar);
/*lint -sem(       rpar_is_valid, 1p == 1) */
extern Bool        rpar_is_valid(const RPar* rpar);
/*lint -sem(       rpar_copy, 1p == 1, @p == 1) */
extern RPar*       rpar_copy(const RPar* rpar);

/* lpstore.c
 */
#define LP_FLAG_CON_SCALE    1
#define LP_FLAG_CON_SEPAR    2

/*lint -sem(        lps_alloc, 1p) */
extern void         lps_alloc(const char* name);
extern void         lps_free(void);
extern void         lps_number(void);
/*lint -sem(        lps_getvar, 1p) */
extern Var*         lps_getvar(const char* name);
/*lint -sem(        lps_getcon, 1p) */
extern Con*         lps_getcon(const char* name);
/*lint -sem(        lps_getnzo, 1p == 1 && 2p == 1) */
extern Nzo*         lps_getnzo(const Con* con, const Var* var);
/*lint -sem(        lps_addvar, 1p, @p == 1) */
extern Var*         lps_addvar(const char* name, VarType type,
   double lower, double upper, int prio_up, int prio_down); 
/*lint -sem(        lps_addcon, 1p, @p == 1) */
extern Con*         lps_addcon(
   const char* name, ConType sense, double rhs, unsigned int flags);
/*lint -sem(        lps_addnzo, 1p == 1 && 2p == 1) */
extern void         lps_addnzo(Con* con, Var* var, double value);
/*lint -sem(        lps_delnzo, 1p == 1) */
extern void         lps_delnzo(Nzo* nzo);
extern void         lps_stat(void);
/*lint -sem(        lps_setval, 1p == 1) */
extern void         lps_setval(Nzo* nzo, double value);
/*lint -sem(        lps_getval, 1p == 1) */
extern double       lps_getval(const Nzo* nzo);
extern void         lps_setdir(LpDirect direct);
/*lint -sem(        lps_objname, 1p) */
extern void         lps_objname(const char* name);
/*lint -sem(        lps_getcost, 1p == 1) */
extern double       lps_getcost(const Var* var);
/*lint -sem(        lps_setcost, 1p == 1) */
extern void         lps_setcost(Var* var, double cost);
/*lint -sem(        lps_setlower, 1p == 1) */
extern void         lps_setlower(Var* var, double lower);
/*lint -sem(        lps_setupper, 1p == 1) */
extern void         lps_setupper(Var* var, double upper);
/*lint -sem(        lps_setsense, 1p == 1) */
extern void         lps_setsense(Con* con, ConType sense);
/*lint -sem(        lps_setrhs, 1p == 1) */
extern void         lps_setrhs(Con* con, double rhs);
/*lint -sem(        lps_getsense, 1p == 1) */
extern int          lps_getsense(const Con* con);
/*lint -sem(        lps_write, 1p == 1) */
extern void         lps_write(FILE* fp, LpForm form);
/*lint -sem(        lps_makename, 1p && 3p) */
extern void         lps_makename(
   char* target, int size, const char* name, int no);
/*lint -sem(        lps_transtable, 1p == 1) */
extern void         lps_transtable(FILE* fp);
/*lint -sem(        lps_orderfile, 1p == 1) */
extern void         lps_orderfile(FILE* fp);
extern void         lps_scale(void);

/* lpfwrite.c
 */
/*lint -sem(        lpf_write, 1p == 1 && 2p == 1) */
extern void         lpf_write(const Lps* lp, FILE* fp);

/* mpswrite.c
 */
/*lint -sem(        mps_write, 1p == 1 && 2p == 1) */
extern void         mps_write(const Lps* lp, FILE* fp);

/* ordwrite.c
 */
/*lint -sem(        ord_write, 1p == 1 && 2p == 1) */
extern void         ord_write(const Lps* lp, FILE* fp);

/* stmt.c
 */
/*lint -sem(        stmt_new, 2p && 3n >= 0 && 4p && 5p == 1, @p == 1) */
extern Stmt*        stmt_new(StmtType type, const char* filename, int lineno,
   const char* name, const char* text);
/*lint -sem(        stmt_free, 1p == 1) */
extern void         stmt_free(Stmt* stmt);
/*lint -sem(        stmt_is_valid, 1p == 1) */
extern Bool         stmt_is_valid(const Stmt* stmt);
/*lint -sem(        stmt_get_name, 1p == 1, @p) */
extern const char*  stmt_get_name(const Stmt* stmt);
/*lint -sem(        stmt_get_filename, 1p == 1, @p) */
extern const char*  stmt_get_filename(const Stmt* stmt);
/*lint -sem(        stmt_get_lineno, 1p == 1, @n > 0) */
extern int          stmt_get_lineno(const Stmt* stmt);
/*lint -sem(        stmt_get_filename, 1p == 1, @p) */
extern const char*  stmt_get_text(const Stmt* stmt);
/*lint -sem(        stmt_parse, 1p == 1) */
extern void         stmt_parse(Stmt* stmt);
/*lint -sem(        stmt_execute, 1p == 1) */
extern void         stmt_execute(const Stmt* stmt);
/*lint -sem(        stmt_print, 1p == 1 && 2p == 1) */
extern void         stmt_print(FILE* fp, const Stmt* stmt);

/* prog.c
 */
/*lint -sem(        prog_new, 1p) */
extern Prog*        prog_new(void);
/*lint -sem(        prog_free, 1p == 1) */
extern void         prog_free(Prog* prog);
/*lint -sem(        prog_is_valid, 1p == 1) */
extern Bool         prog_is_valid(const Prog* prog);
/*lint -sem(        prog_add_stmt, 1p == 1 && 2p == 1) */
extern void         prog_add_stmt(Prog* prog, Stmt* stmt);
/*lint -sem(        prog_print, 1p == 1 && 2p == 1) */
extern void         prog_print(FILE* fp, const Prog* prog);
/*lint -sem(        prog_execute, 1p == 1) */
extern void         prog_execute(const Prog* prog);

/* load.c
 */
/*lint -sem(        prog_load, 1p == 1 && 2p) */
extern void         prog_load(Prog* prog, const char* filename);

/* source.c
 */
extern void         show_source(FILE* fp, const char* text, int column);

/* mmlparse.y
 */
extern int          yyparse(void);

/* mmlscan.l
 */
/*lint -sem(        parse_stmt, 1p) */
extern void         parse_stmt(const Stmt* stmt);
extern const Stmt*  scan_get_stmt(void);
extern int          scan_get_column(void);
extern int          yylen(void);

#ifndef NDEBUG
#define SID unsigned int sid;
#define SID_set(p, id)  (p->sid = id)
#define SID_del(p)      (p->sid = 0xffffffff)
#define SID_ok(p, id)   (p->sid == id)
#else /* NDEBUG */
#define SID              /* */
#define SID_set(p, sid)  /* */
#define SID_del(p)       /* */
#define SID_ok(p, id)    TRUE
#endif /* NDEBUG */

#define DISPERSE(x) (1664525U * (x) + 1013904223U);

#endif /* _MME_H_ */
