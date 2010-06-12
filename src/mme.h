#pragma ident "@(#) $Id: mme.h,v 1.92 2010/06/12 20:32:52 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mme.h                                                         */
/*   Name....: Mathematical Modelling Engine                                 */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2009 by Thorsten Koch <koch@zib.de>
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

#define ZIMPL_VERSION  300

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

typedef enum bound_type          BoundType;
typedef struct bound             Bound;
typedef enum symbol_type         SymbolType;
typedef struct symbol            Symbol;
typedef enum define_type         DefineType;
typedef struct define            Define;
typedef struct index_set         IdxSet;
typedef struct list_element      ListElem;
typedef struct list              List;
typedef struct local             Local;
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

#include "numb.h"
#include "elem.h"
#include "tuple.h"
#include "set.h"
#include "entry.h"
#include "list.h"
#include "term.h"

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

/* zimpllib.c
 */
extern int          verbose;
/*lint -function(exit,zpl_exit) */
extern void         zpl_exit(int retval);
/*lint -sem(        zpl_add_parameter, 1p && nulterm(1)) */
extern void         zpl_add_parameter(const char* def);
/*lint -sem(        zpl_var_print, 1p == 1 && 2p == 1) */
extern void         zpl_var_print(FILE* fp, const Var* var);
/*lint -sem(        zpl_print_banner, 1p == 1) */
extern void         zpl_print_banner(FILE* fp, Bool with_license);

/* strstore.c
 */
extern void         str_init(void);
extern void         str_exit(void);
/*lint -sem(        str_new, 1p && nulterm(1), @p == 1p && nulterm(@)) */
extern const char*  str_new(const char* s);
/*lint -sem(        str_hash, 1p)           */
extern unsigned int str_hash(const char* s);

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
/*lint -sem(        bound_print, 1p == 1 && 2p == 1) */
extern void         bound_print(FILE* fp, const Bound* bound);

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

/* rdefpar.c
 */
/*lint -sem(       rdef_new, nulterm(1), nulterm(2), 1p && 2p, @p == 1) */
extern RDef*       rdef_new(const char* filename, const char* pattern);
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
/*lint -sem(       rdef_get_pattern, 1p == 1, @p && nulterm(@)) */
extern const char* rdef_get_pattern(const RDef* rdef);
/*lint -sem(       rdef_get_comment, 1p == 1, @p && nulterm(@)) */
extern const char* rdef_get_comment(const RDef* rdef);
/*lint -sem(       rdef_get_match, 1p == 1) */
extern const char* rdef_get_match(const RDef* rdef);
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
/*lint -sem(       rpar_new_match, 1p && nulterm(1), @p == 1) */
extern RPar*       rpar_new_match(const char* match);
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
/*lint -sem(        prog_tostr, 1p == 1 && nulterm(2) && 3n > 0, nulterm(@)) */
extern char*        prog_tostr(const Prog* prog, const char* prefix, const char* title, int max_output_line_len);

/* load.c
 */
/*lint -sem(        prog_load, nulterm(3), 1p == 1) */
extern void         prog_load(Prog* prog, const char* cmd, const char* filename);

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
/*lint -function(   fgets(1), mio_gets(2)) */
/*lint -function(   fgets(2), mio_gets(3)) */
/*lint -function(   fgets(3), mio_gets(1)) */
/*lint -function(   fgets(r), mio_gets(r)) */
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

#define DISPERSE(x) (1664525U * (x) + 1013904223U)

#ifdef TRACE
#define Trace(fname) fprintf(stderr, "Trace: %s\n", fname);
#else
#define Trace(fname) /* */
#endif /* TRACE */

#ifdef __cplusplus
}
#endif

#endif /* _MME_H_ */
