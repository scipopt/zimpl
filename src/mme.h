#ident "@(#) $Id: mme.h,v 1.6 2001/01/30 19:14:10 thor Exp $"
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

typedef int                      Bool;
typedef enum element_type        ElemType;
typedef struct element           Elem;
typedef struct tuple             Tuple;
typedef struct set               Set;
typedef enum variable_type       VarType;
typedef struct variable          Var;
typedef struct entry             Entry;
typedef enum symbol_type         SymbolType;
typedef struct symbol            Symbol;
typedef struct index_set         IdxSet;
typedef struct term              Term;
typedef enum inequality_type     IneqType; 
typedef struct inequality        Ineq;
typedef struct local             Local;
typedef struct list_element      ListElem;
typedef struct list              List;
typedef enum hash_type           HashType;
typedef struct hash              Hash;

typedef enum statement_type  StmtType;
typedef struct statement     Stmt;
typedef struct program       Prog;

typedef enum code_type       CodeType;
typedef struct code_node     CodeNode;

typedef void               (*Inst)(CodeNode* self);

enum element_type    { ELEM_ERR = 0, ELEM_NUMB, ELEM_STRG, ELEM_NAME };
enum variable_type   { VAR_ERR = 0, VAR_CON, VAR_INT, VAR_BIN };
enum symbol_type     { SYM_ERR = 0, SYM_NUMB, SYM_STRG, SYM_SET, SYM_VAR };
enum inequality_type { INEQ_ERR = 0, INEQ_EQ, INEQ_LE, INEQ_GE };
enum hash_type       { HASH_ERR = 0, HASH_TUPLE, HASH_ENTRY };

enum statement_type
{
   STMT_ERR = 0, STMT_SET, STMT_PARAM, STMT_VAR, STMT_MIN, STMT_MAX,
   STMT_CONS, STMT_DATA
};

extern Bool          verbose;

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
extern void         hash_add_entry(Hash* hash, Entry* entry);
/*lint -sem(        hash_has_tuple, 1p == 1 && 2p == 1) */
extern Bool         hash_has_tuple(Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_has_entry, 1p == 1 && 2p == 1) */
extern Bool         hash_has_entry(Hash* hash, Tuple* tuple);
/*lint -sem(        hash_lookup_entry, 1p == 1 && 2p == 1, @p == 1) */
extern Entry*       hash_lookup_entry(Hash* hash, Tuple* tuple);

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
extern void         tuple_set_elem(Tuple* tuple, int idx, const Elem* elem);
/*lint -sem(        tuple_get_elem, 1p == 1 && 2n >= 0, @p == 1) */
extern const Elem*  tuple_get_elem(const Tuple* tuple, int idx);
/*lint -sem(        tuple_combine, 1p == 1 && 2p == 1, @p == 1) */
extern Tuple*       tuple_combine(const Tuple* ta, const Tuple* tb);
/*lint -sem(        tuple_print, 1p == 1 && 2p == 1) */
extern void         tuple_print(FILE* fp, const Tuple* tuple);
/*lint -sem(        tuple_hash, 1p == 1) */
extern unsigned int tuple_hash(const Tuple* tuple);

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
/*lint -sem(        set_print, 1p == 1 && 2p == 1) */
extern void         set_print(FILE* fp, const Set* set);
/*lint -sem(        set_range, @p == 1) */
extern Set*         set_range(double start, double end, double step);
/*lint -sem(        set_cross, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_cross(const Set* seta, const Set* setb);
/*lint -sem(        set_union, 1p == 1 && 2p == 1, @p == 1) */
extern Set*         set_union(const Set* seta, const Set* setb);

/* var.c
 */
/*lint -sem(        var_new, @p == 1) */
extern Var*         var_new(VarType type, double lower, double upper);
/*lint -sem(        var_free, 1p == 1) */
extern void         var_free(Var* var);
/*lint -sem(        var_is_valid, 1p == 1) */
extern Bool         var_is_valid(const Var* var);
/*lint -sem(        var_copy, 1p == 1, @p == 1) */
extern Var*         var_copy(Var* var);
/*lint -sem(        var_get_type, 1p == 1) */
extern VarType      var_get_type(const Var* var);
/*lint -sem(        var_get_lower, 1p == 1) */
extern double       var_get_lower(const Var* var);
/*lint -sem(        var_get_upper, 1p == 1) */
extern double       var_get_upper(const Var* var);
/*lint -sem(        var_print, 1p == 1 && 2p == 1) */
extern void         var_print(FILE* fp, const Var* var);

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
extern int          entry_get_index(Entry* entry);
/*lint -sem(        entry_get_numb, 1p == 1) */
extern double       entry_get_numb(const Entry* entry);
/*lint -sem(        entry_get_strg, 1p == 1, @p == 1) */
extern const char*  entry_get_strg(const Entry* entry);
/*lint -sem(        entry_get_set, 1p == 1, @p == 1) */
extern Set*         entry_get_set(Entry* entry);
/*lint -sem(        entry_get_var, 1p == 1, @p == 1) */
extern Var*         entry_get_var(Entry* entry);
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
extern Bool         symbol_has_entry(const Symbol* sym, Tuple* tuple);
/*lint -sem(        symbol_lookup_entry, 1p == 1 && 2p == 1, @p == 1) */
extern Entry*       symbol_lookup_entry(const Symbol* sym, Tuple* tuple);
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
extern Set*         symbol_get_set(Symbol* sym, int idx);
/*lint -sem(        symbol_get_var, 1p == 1, @p == 1) */
extern Var*         symbol_get_var(Symbol* sym, int idx);
/*lint -sem(        symbol_print, 1p == 1 && 2p == 1) */
extern void         symbol_print(FILE* fp, const Symbol* sym);
/*lint -sem(        symbol_print_all, 1p == 1) */
extern void         symbol_print_all(FILE* fp);
/*lint -sem(        symbol_print_bounds, 1p == 1) */
extern void         symbol_print_bounds(FILE* fp);

/* idxset.c
 */
/*lint -sem(        idxset_new, @p == 1) */
extern IdxSet*      idxset_new(void);
/*lint -sem(        idxset_free, 1p == 1) */
extern void         idxset_free(IdxSet* idxset);
/*lint -sem(        idxset_add_set, 1p == 1 && 2p == 1 && 3p == 1) */
extern void         idxset_add_set(IdxSet* idxset, Tuple* tuple, Set* set);
/*lint -sem(        idxset_is_valid, 1p == 1) */
extern Bool         idxset_is_valid(const IdxSet* idxset);
/*lint -sem(        idxset_copy, 1p == 1, @p == 1) */
extern IdxSet*      idxset_copy(const IdxSet* source);
/*lint -sem(        idxset_set_lexpr, 1p == 1 && 2p == 1) */
extern void         idxset_set_lexpr(IdxSet* idxset, CodeNode* addr);
/*lint -sem(        idxset_get_lexpr, 1p == 1, @p == 1) */
extern CodeNode*    idxset_get_lexpr(const IdxSet* idxset);
/*lint -sem(        idxset_get_sets, 1p == 1, @n >= 0) */
extern int          idxset_get_sets(const IdxSet* idxset);
/*lint -sem(        idxset_get_dim, 1p == 1, @n >= 0) */
extern int          idxset_get_dim(const IdxSet* idxset);
/*lint -sem(        idxset_get_tuple, 1p == 1 && 2n >= 0, @p == 1) */
extern Tuple*       idxset_get_tuple(const IdxSet* idxset, int set_nr);
/*lint -sem(        idxset_get_set, 1p == 1 && 2n >= 0, @p == 1) */
extern Set*         idxset_get_set(const IdxSet* idxset, int set_nr);
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

/*lint -sem(        term_new, @p == 1) */
extern Term*        term_new(void);
/*lint -sem(        term_add_elem, 1p == 1 && 2p == 1 && 3p == 1) */
extern void         term_add_elem(
   Term* term, const Symbol* sym, Entry* entry, double coeff);
/*lint -sem(        term_free, 1p == 1) */
extern void         term_free(Term* term);
/*lint -sem(        term_is_valid, 1p == 1) */
extern Bool         term_is_valid(const Term* term);
/*lint -sem(        term_copy, 1p == 1, @p == 1) */
extern Term*        term_copy(Term* term);
/*lint -sem(        term_print, 1p == 1 && 2p == 1 && 3n >= 1) */
extern void         term_print(FILE* fp, const Term* term, int flag);
/*lint -sem(        term_add_term, 1p == 1 && 2p == 1, @p == 1) */
extern Term*        term_add_term(Term* term_a, Term* term_b);
/*lint -sem(        term_add_constant, 1p == 1) */
extern void         term_add_constant(Term* term, double value);
/*lint -sem(        term_mul_coeff, 1p == 1) */
extern void         term_mul_coeff(Term* term, double value);
/*lint -sem(        term_get_constant, 1p == 1) */
extern double       term_get_constant(Term* term);
/*lint -sem(        term_negate, 1p == 1) */
extern void         term_negate(Term* term);

/* ineq.c
 */
/*lint -sem(        ineq_new, 2p == 1, @p == 1) */
extern Ineq*        ineq_new(IneqType type, Term* term, double rhs);
/*lint -sem(        ineq_free, 1p == 1) */
extern void         ineq_free(Ineq* ineq);
/*lint -sem(        ineq_is_valid, 1p == 1) */
extern Bool         ineq_is_valid(const Ineq* ineq);
/*lint -sem(        ineq_copy, 1p == 1, @p == 1) */
extern Ineq*        ineq_copy(Ineq* ineq);
/*lint -sem(        ineq_print, 1p == 1 && 2p == 1) */
extern void         ineq_print(FILE* fp, const Ineq* ineq);

/* stmt.c
 */
/*lint -sem(        stmt_new, 2n >= 0 && 3p && 4p == 1, @p == 1) */
extern Stmt*        stmt_new(StmtType type, int lineno, const char* name,
   const char* text);
/*lint -sem(        stmt_free, 1p == 1) */
extern void         stmt_free(Stmt* stmt);
/*lint -sem(        stmt_is_valid, 1p == 1) */
extern Bool         stmt_is_valid(const Stmt* stmt);
/*lint -sem(        stmt_get_name, 1p == 1, @p) */
extern const char*  stmt_get_name(const Stmt* stmt);
/*lint -sem(        stmt_parse, 1p == 1) */
extern void         stmt_parse(Stmt* stmt);
/*lint -sem(        stmt_execute, 1p == 1) */
extern void         stmt_execute(const Stmt* stmt);
/*lint -sem(        stmt_print, 1p == 1 && 2p == 1) */
extern void         stmt_print(FILE* fp, const Stmt* stmt);

/* prog.c
 */
/*lint -sem(        prog_new, 1p, @p == 1) */
extern Prog*        prog_new(const char* filename);
/*lint -sem(        prog_free, 1p == 1) */
extern void         prog_free(Prog* prog);
/*lint -sem(        prog_is_valid, 1p == 1) */
extern Bool         prog_is_valid(const Prog* prog);
/*lint -sem(        prog_add_stmt, 1p == 1 && 2p == 1) */
extern void         prog_add_stmt(Prog* prog, Stmt* stmt);
/*lint -sem(        prog_get_filename, 1p == 1, @p) */
extern const char*  prog_get_filename(const Prog* prog);
/*lint -sem(        prog_print, 1p == 1 && 2p == 1) */
extern void         prog_print(FILE* fp, const Prog* prog);
/*lint -sem(        prog_execute, 1p == 1) */
extern void         prog_execute(const Prog* prog);

/* load.c
 */
/*lint -sem(        prog_load, 1p == 1) */
extern void         prog_load(Prog* prog);

/* mmlparse.y
 */
extern int          yyparse(void);

/* mmlscan.l
 */
/*lint -sem(        parse_string, 1p && 2n >= 0) */
extern void         parse_string(const char* s, int lineno);
extern int          scan_get_lineno(void);
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

#ifndef NDEBUG
#define Trace(fname) if (verbose) fprintf(stderr, "Trace: %s\n", fname);
#else
#define Trace(fname) /* */
#endif

#define DISPERSE(x) (1664525U * (x) + 1013904223U);

