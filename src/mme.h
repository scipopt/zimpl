#ident "@(#) $Id: mme.h,v 1.3 2001/01/29 13:45:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mme.h                                                         */
/*   Name....: Mathematical Modelling Engine Functions                       */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Parameter werden nicht gefressen, wer es braucht kopiert es sich,
 * aber geliefert wird nur der pointer.
 *
 * ??? const Objekte sind nur pointer und brauchen nicht freigegeben zu werden,
 * alle nicht const Objekte muessen freigegeben werden.
 */
/* ueberlegung zum Speicherverhalten
 * in den Code koennen an dynamischen Daten nur
 * strings/Namen geschrieben werden.
 * Wir gehen davon aus, das diese vom uebergebenden Programm
 * verwaltet werden un loeschen sie nicht.
 */

/* Die Symbole werden extern angelegt. Dazu muss nur der Name, Typ und
 * die Dimension bekannt sein.
 * Spaeter werden dann Paare der Form <tuple>(wert) hinzugefuegt.
 *
 * Wobei noch zu ueberlegen ist, wie das mit den Mengen richtig zu
 * machen ist. Vermutlich sollte man in der Symboltabelle immer einen
 * eintrag anlegen und dann vom Symbol aus darauf verweisen. so kann
 * einfach ueberprueft werden, ob das Index tuple zulaessig ist.
 *
 * Eine Idee ist einen Pointer auf das Set des Symbols mit im Symbol
 * zu speichern, und das entsprechende Tuplel dann fuer den Entry zu nehmen.
 */

/* Uebelegung:
 *    forall { i in A, j in B : i > 5, j < 2, (i,j) in C }
 * ist equivalent zu
 *
 *    forall { i in A : i > 5 }
 *      forall { j in B : j < 2, (i,j) in C }
 *
 * Es waere auch
 *
 *    forall { i in A }
 *      forall { j in B : i > 5, j < 2, (i,j) in C }
 *
 * denkbar
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

enum code_type
{
   CODE_ERR = 0, CODE_NUMB, CODE_STRG, CODE_NAME, CODE_TUPLE,
   CODE_SET, CODE_TERM, CODE_SYMBOL, CODE_BOOL, CODE_SIZE, CODE_INEQ,
   CODE_IDXSET, CODE_LIST, CODE_VOID, CODE_ENTRY, CODE_VARTYPE, CODE_INEQTYPE
};

extern int          verbose;

/*lint -sem(str_new,  1p, @p == 1p) */
/*lint -sem(str_hash, 1p)           */
extern void         str_init(void);
extern void         str_exit(void);
extern const char*  str_new(const char* s);
extern unsigned int str_hash(const char* s);

/* list.c
 */
/*lint -sem(list_new_elem,  1p == 1,            @p == 1) */
/*lint -sem(list_new_tuple, 1p == 1,            @p == 1) */
/*lint -sem(list_new_entry, 1p == 1,            @p == 1) */
/*lint -sem(list_free,      1p == 1)                     */
/*lint -sem(list_is_valid,  1p == 1)                     */
/*lint -sem(list_copy,      1p == 1,            @P == 1) */
/*lint -sem(list_add_elem,  1p == 1 && 2p == 1)          */
/*lint -sem(list_add_tuple, 1p == 1 && 2p == 1)          */
/*lint -sem(list_add_entry, 1p == 1 && 2p == 1)          */
/*lint -sem(list_get_elems, 1p == 1,            @n >= 0) */
/*lint -sem(list_get_elem,  1p == 1,            @p == 1) */
/*lint -sem(list_get_tuple, 1p == 1,            @p == 1) */
/*lint -sem(list_get_entry, 1p == 1,            @p == 1) */
/*lint -sem(list_print,     1p == 1 && 2p == 1)          */

extern List*        list_new_elem(const Elem* elem);
extern List*        list_new_tuple(Tuple* tuple);
extern List*        list_new_entry(Entry* entry);
extern void         list_free(List* list);
extern int          list_is_valid(const List* list);
extern List*        list_copy(List* list);
extern void         list_add_elem(List* list, const Elem* elem);
extern void         list_add_tuple(List* list, Tuple* tuple);
extern void         list_add_entry(List* list, Entry* entry);
extern int          list_get_elems(const List* list);
extern const Elem*  list_get_elem(const List* list, ListElem** idxp);
extern Tuple*       list_get_tuple(const List* list, ListElem** idxp);
extern Entry*       list_get_entry(const List* list, ListElem** idxp);
extern void         list_print(FILE* fp, const List* list);

/* hash
 */
/*lint -sem(hash_new,                              @p == 1) */
/*lint -sem(hash_free,         1p == 1)                     */
/*lint -sem(hash_is_valid,     1p == 1)                     */
/*lint -sem(hash_add_tuple,    1p == 1 && 2p == 1)          */
/*lint -sem(hash_add_entry,    1p == 1 && 2p == 1)          */
/*lint -sem(hash_has_tuple,    1p == 1 && 2p == 1)          */
/*lint -sem(hash_has_entry,    1p == 1 && 2p == 1)          */
/*lint -sem(hash_lookup_entry, 1p == 1 && 2p == 1, @p == 1) */

extern Hash*        hash_new(HashType type);
extern void         hash_free(Hash* hash);
extern int          hash_is_valid(const Hash* hash);
extern void         hash_add_tuple(Hash* hash, const Tuple* tuple);
extern void         hash_add_entry(Hash* hash, Entry* entry);
extern int          hash_has_tuple(Hash* hash, const Tuple* tuple);
extern int          hash_has_entry(Hash* hash, Tuple* tuple);
extern Entry*       hash_lookup_entry(Hash* hash, Tuple* tuple);

/* element.c
 */
#define ELEM_NULL  ((Elem*)0)

/*lint -sem(elem_new_numb,                     @p == 1) */
/*lint -sem(elem_new_strg, 1p,                 @p == 1) */
/*lint -sem(elem_new_name, 1p,                 @p == 1) */
/*lint -sem(elem_is_valid, 1p == 1)                     */
/*lint -sem(elem_cmp,      1p == 1 && 2p == 1)          */
/*lint -sem(elem_get_type, 1p == 1)                     */
/*lint -sem(elem_get_numb, 1p == 1)                     */
/*lint -sem(elem_get_strg, 1p == 1,            @p)      */
/*lint -sem(elem_get_name, 1p == 1,            @p)      */
/*lint -sem(elem_print,    1p == 1 && 2p == 1)          */
/*lint -sem(elem_hash,     1p == 1)                     */

extern void         elem_init(void);
extern void         elem_exit(void);
extern const Elem*  elem_new_numb(double n);
extern const Elem*  elem_new_strg(const char* s);
extern const Elem*  elem_new_name(const char* s);
extern int          elem_is_valid(const Elem* elem);
extern int          elem_cmp(const Elem* elem_a, const Elem* elem_b);
extern ElemType     elem_get_type(const Elem* elem);
extern double       elem_get_numb(const Elem* elem);
extern const char*  elem_get_strg(const Elem* elem);
extern const char*  elem_get_name(const Elem* elem);
extern void         elem_print(FILE* fp, const Elem* elem);
extern unsigned int elem_hash(const Elem* elem);

/* tuple.c
 */
#define TUPLE_NULL ((Tuple*)0)

/*lint -sem(tuple_new,      1n >= 0,                       @p == 1) */
/*lint -sem(tuple_free,     1p == 1)                                */
/*lint -sem(tuple_is_valid, 1p == 1)                                */
/*lint -sem(tuple_copy,     1p == 1,                       @p == 1) */
/*lint -sem(tuple_cmp,      1p == 1 && 2p == 1)                     */
/*lint -sem(tuple_get_dim,  1p == 1,                       @n >= 0) */
/*lint -sem(tuple_set_elem, 1p == 1 && 2n >= 0 && 3p == 1)          */
/*lint -sem(tuple_get_elem, 1p == 1 && 2n >= 0,            @p == 1) */
/*lint -sem(tuple_combine,  1p == 1 && 2p == 1,            @p == 1) */
/*lint -sem(tuple_print,    1p == 1 && 2p == 1)                     */
/*lint -sem(tuple_hash,     1p == 1)                                */

extern Tuple*       tuple_new(int dim);
extern void         tuple_free(Tuple* tuple);
extern int          tuple_is_valid(const Tuple* tuple);
extern Tuple*       tuple_copy(Tuple* tuple);
extern int          tuple_cmp(const Tuple* tuple_a, const Tuple* tuple_b);
extern int          tuple_get_dim(const Tuple* tuple);
extern void         tuple_set_elem(Tuple* tuple, int idx, const Elem* elem);
extern const Elem*  tuple_get_elem(const Tuple* tuple, int idx);
extern Tuple*       tuple_combine(const Tuple* ta, const Tuple* tb);
extern void         tuple_print(FILE* fp, const Tuple* tuple);
extern unsigned int tuple_hash(const Tuple* tuple);

/* set.c
 */
#define SET_ADD_BEGIN  0
#define SET_ADD_END    1

/*lint -sem(set_new,        1n >= 0, @p == 1)                       */
/*lint -sem(set_free,       1p == 1)                                */
/*lint -sem(set_is_valid,   1p == 1)                                */
/*lint -sem(set_copy,       1p == 1,                       @p == 1) */
/*lint -sem(set_add_member, 1p == 1 && 2p == 1 && 3n >= 0, @p == 1) */
/*lint -sem(set_lookup,     1p == 1 && 2p == 1)                     */
/*lint -sem(set_match,      1p == 1 && 2p == 1 && 3p == 1, @p == 1) */
/*lint -sem(set_get_dim,    1p == 1,                       @n >= 0) */
/*lint -sem(set_print,      1p == 1 && 2p == 1)                     */
/*lint -sem(set_range,      @p == 1)                                */
/*lint -sem(set_cross,      1p == 1 && 2p == 1, @p == 1)            */
/*lint -sem(set_union,      1p == 1 && 2p == 1, @p == 1)            */

extern Set*         set_new(int dim);
extern void         set_free(Set* set);
extern int          set_is_valid(const Set* set);
extern Set*         set_copy(Set* set);
extern void         set_add_member(Set* set, Tuple* tuple, int where);
extern int          set_lookup(const Set* set, const Tuple* tuple);
extern Tuple*       set_match_next(const Set* set, const Tuple* pat, int* idx);
extern int          set_get_dim(const Set* set);
extern void         set_print(FILE* fp, const Set* set);
extern Set*         set_range(double start, double end, double step);
extern Set*         set_cross(const Set* seta, const Set* setb);
extern Set*         set_union(const Set* seta, const Set* setb);

/* var.c
 */
/*lint -sem(var_new,                @p == 1)   */
/*lint -sem(var_free,      1p == 1)            */
/*lint -sem(var_is_valid,  1p == 1)            */
/*lint -sem(var_copy,      1p == 1, @p == 1)   */
/*lint -sem(var_get_type,  1p == 1)            */
/*lint -sem(var_get_lower, 1p == 1)            */
/*lint -sem(var_get_upper, 1p == 1)            */
/*lint -sem(var_print,     1p == 1 && 2p == 1) */

extern Var*         var_new(VarType type, double lower, double upper);
extern void         var_free(Var* var);
extern int          var_is_valid(const Var* var);
extern Var*         var_copy(Var* var);
extern VarType      var_get_type(const Var* var);
extern double       var_get_lower(const Var* var);
extern double       var_get_upper(const Var* var);
extern void         var_print(FILE* fp, const Var* var);

/* entry.c
 */
#define ENTRY_NULL ((Entry*)0)

/*lint -sem(entry_new_numb,  1p == 1,            @p == 1) */
/*lint -sem(entry_new_strg,  1p == 1 && 2p,      @p == 1) */
/*lint -sem(entry_new_set,   1p == 1 && 2p == 1, @p == 1) */
/*lint -sem(entry_new_var,   1p == 1 && 2p == 1, @p == 1) */
/*lint -sem(entry_free,      1p == 1)                     */
/*lint -sem(entry_is_valid,  1p == 1)                     */
/*lint -sem(entry_copy,      1p == 1,            @p == 1) */
/*lint -sem(entry_cmp,       1p == 1 && 2p == 1)          */
/*lint -sem(entry_set_index, 1p == 1 && 2n >= 0)          */
/*lint -sem(entry_get_type,  1p == 1,            @n >= 0) */
/*lint -sem(entry_get_tuple, 1p == 1,            @p == 1) */
/*lint -sem(entry_get_index, 1p == 1,            @n >= 0) */
/*lint -sem(entry_get_numb,  1p == 1)                     */
/*lint -sem(entry_get_strg,  1p == 1,            @p == 1) */
/*lint -sem(entry_get_set,   1p == 1,            @p == 1) */
/*lint -sem(entry_get_var,   1p == 1,            @p == 1) */
/*lint -sem(entry_print,     1p == 1 && 2p == 1)          */
/*lint -sem(entry_hash,      1p == 1)                     */

extern Entry*       entry_new_numb(Tuple* tuple, double numb);
extern Entry*       entry_new_strg(Tuple* tuple, const char* strg);
extern Entry*       entry_new_set(Tuple* tuple, Set* set);
extern Entry*       entry_new_var(Tuple* tuple, Var* var);
extern void         entry_free(Entry* entry);
extern int          entry_is_valid(const Entry* entry);
extern Entry*       entry_copy(Entry* entry);
extern int          entry_cmp(const Entry* entry, const Tuple* tuple);
extern void         entry_set_index(Entry* entry, int idx);
extern SymbolType   entry_get_type(const Entry* entry);
extern Tuple*       entry_get_tuple(const Entry* entry);
extern int          entry_get_index(Entry* entry);
extern double       entry_get_numb(const Entry* entry);
extern const char*  entry_get_strg(const Entry* entry);
extern Set*         entry_get_set(Entry* entry);
extern Var*         entry_get_var(Entry* entry);
extern void         entry_print(FILE* fp, const Entry* entry);
extern unsigned int entry_hash(const Entry* entry);

/* symbol.c
 */
/*lint -sem(symbol_new,          1p == 1 && 3p == 1, @p == 1) */
/*lint -sem(symbol_is_valid,     1p == 1)                     */
/*lint -sem(symbol_lookup,       1p == 1,            r_null)  */
/*lint -sem(symbol_has_entry,    1p == 1 && 2p == 1)          */
/*lint -sem(symbol_lookup_entry, 1p == 1 && 2p == 1, @p == 1) */
/*lint -sem(symbol_add_entry,    1p == 1 && 2p == 1)          */
/*lint -sem(symbol_get_dim,      1p == 1,            @n >= 0) */
/*lint -sem(symbol_get_iset,     1p == 1,            @p == 1) */
/*lint -sem(symbol_get_name,     1p == 1,            @p)      */
/*lint -sem(symbol_get_type,     1p == 1)                     */
/*lint -sem(symbol_get_numb,     1p == 1)                     */
/*lint -sem(symbol_get_strg,     1p == 1,            @p)      */
/*lint -sem(symbol_get_set,      1p == 1,            @p == 1) */
/*lint -sem(symbol_get_var,      1p == 1,            @p == 1) */
/*lint -sem(symbol_print,        1p == 1 && 2p == 1)          */
/*lint -sem(symbol_print_all,    1p == 1)                     */
/*lint -sem(symbol_print_bounds, 1p == 1)                     */

extern Symbol*      symbol_new(const char* name, SymbolType type, Set* set);
extern void         symbol_exit(void);
extern int          symbol_is_valid(const Symbol* symbol);
extern Symbol*      symbol_lookup(const char* name);
extern int          symbol_has_entry(const Symbol* sym, Tuple* tuple);
extern Entry*       symbol_lookup_entry(const Symbol* sym, Tuple* tuple);
extern void         symbol_add_entry(Symbol* sym, Entry* entry);
extern int          symbol_get_dim(const Symbol* sym);
extern const Set*   symbol_get_iset(const Symbol* sym);
extern const char*  symbol_get_name(const Symbol* sym);
extern SymbolType   symbol_get_type(const Symbol* sym);
extern double       symbol_get_numb(const Symbol* sym, int idx);
extern const char*  symbol_get_strg(const Symbol* sym, int idx);
extern Set*         symbol_get_set(Symbol* sym, int idx);
extern Var*         symbol_get_var(Symbol* sym, int idx);
extern void         symbol_print(FILE* fp, const Symbol* sym);
extern void         symbol_print_all(FILE* fp);
extern void         symbol_print_bounds(FILE* fp);

/* idxset.c
 */
/*lint -sem(idxset_new,       @p == 1)                                */
/*lint -sem(idxset_free,      1p == 1)                                */
/*lint -sem(idxset_add_set,   1p == 1 && 2p == 1 && 3p == 1)          */
/*lint -sem(idxset_is_valid,  1p == 1)                                */
/*lint -sem(idxset_copy,      1p == 1,                       @p == 1) */
/*lint -sem(idxset_set_lexpr, 1p == 1 && 2p == 1)                     */
/*lint -sem(idxset_get_lexpr, 1p == 1,                       @p == 1) */
/*lint -sem(idxset_get_sets,  1p == 1,                       @n >= 0) */
/*lint -sem(idxset_get_dim,   1p == 1,                       @n >= 0) */
/*lint -sem(idxset_get_tuple, 1p == 1 && 2n >= 0,            @p == 1) */
/*lint -sem(idxset_get_set,   1p == 1 && 2n >= 0,            @p == 1) */
/*lint -sem(idxset_print,     1p == 1 && 2p == 1)                     */

extern IdxSet*      idxset_new(void);
extern void         idxset_free(IdxSet* idxset);
extern void         idxset_add_set(IdxSet* idxset, Tuple* tuple, Set* set);
extern int          idxset_is_valid(const IdxSet* idxset);
extern IdxSet*      idxset_copy(const IdxSet* source);
extern void         idxset_set_lexpr(IdxSet* idxset, CodeNode* addr);
extern CodeNode*    idxset_get_lexpr(const IdxSet* idxset);
extern int          idxset_get_sets(const IdxSet* idxset);
extern int          idxset_get_dim(const IdxSet* idxset);
extern Tuple*       idxset_get_tuple(const IdxSet* idxset, int set_nr);
extern Set*         idxset_get_set(const IdxSet* idxset, int set_nr);
extern void         idxset_print(FILE* fp, const IdxSet* idxset);

/* local.c
 */
/*lint -sem(local_lookup,        1p == 1,            r_null) */
/*lint -sem(local_install_tuple, 1p == 1 && 2p == 1)         */
/*lint -sem(local_print_all,     1p == 1)                    */

extern void         local_drop_frame(void);
extern const Elem*  local_lookup(const char* name);
extern void         local_install_tuple(const Tuple* patt, const Tuple* vals);
extern void         local_print_all(FILE* fp);

/* term.c
 */
#define TERM_PRINT_SYMBOL  1
#define TERM_PRINT_INDEX   2

/*lint -sem(term_new,                                         @p == 1) */
/*lint -sem(term_add_elem,     1p == 1 && 2p == 1 && 3p == 1)          */
/*lint -sem(term_free,         1p == 1)                                */
/*lint -sem(term_is_valid,     1p == 1)                                */
/*lint -sem(term_copy,         1p == 1,                       @p == 1) */
/*lint -sem(term_print,        1p == 1 && 2p == 1 && 3n >= 1)          */
/*lint -sem(term_add_term,     1p == 1 && 2p == 1,            @p == 1) */
/*lint -sem(term_add_constant, 1p == 1)                                */
/*lint -sem(term_mul_coeff,    1p == 1)                                */
/*lint -sem(term_get_constant, 1p == 1)                                */
/*lint -sem(term_negate,       1p == 1)                                */

extern Term*        term_new(void);
extern void         term_add_elem(
   Term* term, const Symbol* sym, Entry* entry, double coeff);
extern void         term_free(Term* term);
extern int          term_is_valid(const Term* term);
extern Term*        term_copy(Term* term);
extern void         term_print(FILE* fp, const Term* term, int flag);
extern Term*        term_add_term(Term* term_a, Term* term_b);
extern void         term_add_constant(Term* term, double value);
extern void         term_mul_coeff(Term* term, double value);
extern double       term_get_constant(Term* term);
extern void         term_negate(Term* term);

/* ineq.c
 */
/*lint -sem(ineq_new,      2p == 1,            @p == 1) */
/*lint -sem(ineq_free,     1p == 1)                     */
/*lint -sem(ineq_is_valid, 1p == 1)                     */
/*lint -sem(ineq_copy,     1p == 1,            @p == 1) */
/*lint -sem(ineq_print,    1p == 1 && 2p == 1)          */

extern Ineq*        ineq_new(IneqType type, Term* term, double rhs);
extern void         ineq_free(Ineq* ineq);
extern int          ineq_is_valid(const Ineq* ineq);
extern Ineq*        ineq_copy(Ineq* ineq);
extern void         ineq_print(FILE* fp, const Ineq* ineq);

/* stmt.c
 */
/*lint -sem(stmt_new,      2n >= 0 && 3p && 4p == 1, @p == 1) */
/*lint -sem(stmt_free,     1p == 1)                           */
/*lint -sem(stmt_is_valid, 1p == 1)                           */
/*lint -sem(stmt_get_name, 1p == 1,                  @p)      */
/*lint -sem(stmt_parse,    1p == 1)                           */
/*lint -sem(stmt_execute,  1p == 1)                           */
/*lint -sem(stmt_print,    1p == 1 && 2p == 1)                */

extern Stmt*        stmt_new(StmtType type, int lineno, const char* name,
   const char* text);
extern void         stmt_free(Stmt* stmt);
extern int          stmt_is_valid(const Stmt* stmt);
extern const char*  stmt_get_name(const Stmt* stmt);
extern int          stmt_parse(Stmt* stmt);
extern int          stmt_execute(const Stmt* stmt);
extern void         stmt_print(FILE* fp, const Stmt* stmt);

/* prog.c
 */
/*lint -sem(prog_new,          1p,                 @p == 1) */
/*lint -sem(prog_free,         1p == 1)                     */
/*lint -sem(prog_is_valid,     1p == 1)                     */
/*lint -sem(prog_add_stmt,     1p == 1 && 2p == 1)          */
/*lint -sem(prog_get_filename, 1p == 1,            @p)      */
/*lint -sem(prog_print,        1p == 1 && 2p == 1)          */
/*lint -sem(prog_execute,      1p == 1)                     */

extern Prog*        prog_new(const char* filename);
extern void         prog_free(Prog* prog);
extern int          prog_is_valid(const Prog* prog);
extern void         prog_add_stmt(Prog* prog, Stmt* stmt);
extern const char*  prog_get_filename(const Prog* prog);
extern void         prog_print(FILE* fp, const Prog* prog);
extern int          prog_execute(const Prog* prog);

/* load.c
 */
/*lint -sem(prog_load, 1p == 1) */

extern int          prog_load(Prog* prog);

/* code2.c
 */
/*lint -sem(code_new_inst,       1p == 1,                       @p == 1) */
/*lint -sem(code_new_numb,       1p == 1,                       @p == 1) */
/*lint -sem(code_new_strg,       1p,                            @p == 1) */
/*lint -sem(code_new_name,       1p,                            @p == 1) */
/*lint -sem(code_new_size,                                      @p == 1) */
/*lint -sem(code_new_vartype,                                   @p == 1) */
/*lint -sem(code_new_ineqtype,                                  @p == 1) */
/*lint -sem(code_free,           1p == 1)                                */
/*lint -sem(code_is_valid,       1p == 1)                                */
/*lint -sem(code_set_child,      1p == 1 && 2n >= 0 && 3p == 1)          */
/*lint -sem(code_get_type,       1p == 1)                                */
/*lint -sem(code_set_root,       1p == 1)                                */
/*lint -sem(code_get_root,                                      @p == 1) */
/*lint -sem(code_eval,           1p == 1)                                */
/*lint -sem(code_execute,        1p == 1)                                */
/*lint -sem(code_get_child,      1p == 1 && 2n >= 0,            @p == 1) */
/*lint -sem(code_get_lineno,     1p == 1)                                */
/*lint -sem(code_get_column,     1p == 1)                                */
/*lint -sem(code_get_numb,       1p == 1)                                */
/*lint -sem(code_get_strg,       1p == 1,                       @p)      */
/*lint -sem(code_get_name,       1p == 1,                       @p)      */
/*lint -sem(code_get_tuple,      1p == 1,                       @p == 1) */
/*lint -sem(code_get_set,        1p == 1,                       @p == 1) */
/*lint -sem(code_get_idxset,     1p == 1,                       @p == 1) */
/*lint -sem(code_get_entry,      1p == 1,                       @p == 1) */
/*lint -sem(code_get_term,       1p == 1,                       @p == 1) */
/*lint -sem(code_get_ineq,       1p == 1,                       @p == 1) */
/*lint -sem(code_get_size,       1p == 1)                                */
/*lint -sem(code_get_bool,       1p == 1)                                */
/*lint -sem(code_get_list,       1p == 1,                       @p == 1) */
/*lint -sem(code_get_vartype,    1p == 1)                                */
/*lint -sem(code_get_ineqtype,   1p == 1)                                */
/*lint -sem(code_value_numb,     1p == 1)                                */
/*lint -sem(code_value_strg,     1p == 1 && 2p)                          */
/*lint -sem(code_value_name,     1p == 1 && 2p)                          */
/*lint -sem(code_value_tuple,    1p == 1 && 2p == 1)                     */
/*lint -sem(code_value_set,      1p == 1 && 2p == 1)                     */
/*lint -sem(code_value_idxset,   1p == 1 && 2p == 1)                     */
/*lint -sem(code_value_entry,    1p == 1 && 2p == 1)                     */
/*lint -sem(code_value_term,     1p == 1 && 2p == 1)                     */
/*lint -sem(code_value_ineq,     1p == 1 && 2p == 1)                     */
/*lint -sem(code_value_bool,     1p == 1)                                */
/*lint -sem(code_value_size,     1p == 1)                                */
/*lint -sem(code_value_list,     1p == 1 && 2p == 1)                     */
/*lint -sem(code_value_vartype,  1p == 1)                                */
/*lint -sem(code_value_ineqtype, 1p == 1)                                */
/*lint -sem(code_value_void,     1p == 1)                                */

extern CodeNode*    code_new_inst(Inst inst, int childs, ...);
extern CodeNode*    code_new_numb(double numb);
extern CodeNode*    code_new_strg(const char* strg);
extern CodeNode*    code_new_name(const char* name);
extern CodeNode*    code_new_size(int size);
extern CodeNode*    code_new_vartype(VarType vartype);
extern CodeNode*    code_new_ineqtype(IneqType ineqtype);
extern void         code_free(CodeNode* node);
extern int          code_is_valid(const CodeNode* node);
extern void         code_set_child(CodeNode* node, int idx, CodeNode* child);
extern CodeType     code_get_type(CodeNode* node);
extern void         code_set_root(CodeNode* node);
extern CodeNode*    code_get_root(void);
extern CodeType     code_eval(CodeNode* node);
extern void         code_execute(CodeNode* node);
extern CodeNode*    code_get_child(CodeNode* node, int no);
extern int          code_get_lineno(const CodeNode* node);
extern int          code_get_column(const CodeNode* node);
extern double       code_get_numb(CodeNode* node);
extern const char*  code_get_strg(CodeNode* node);
extern const char*  code_get_name(CodeNode* node);
extern Tuple*       code_get_tuple(CodeNode* node);
extern Set*         code_get_set(CodeNode* node);
extern IdxSet*      code_get_idxset(CodeNode* node);
extern Entry*       code_get_entry(CodeNode* node);
extern Term*        code_get_term(CodeNode* node);
extern Ineq*        code_get_ineq(CodeNode* node);
extern int          code_get_size(CodeNode* node);
extern int          code_get_bool(CodeNode* node);
extern List*        code_get_list(CodeNode* node);
extern VarType      code_get_vartype(CodeNode* node);
extern IneqType     code_get_ineqtype(CodeNode* node);
extern void         code_value_numb(CodeNode* node, double numb);
extern void         code_value_strg(CodeNode* node, const char* strg);
extern void         code_value_name(CodeNode* node, const char* name);
extern void         code_value_tuple(CodeNode* node, Tuple* tuple);
extern void         code_value_set(CodeNode* node, Set* set);
extern void         code_value_idxset(CodeNode* node, IdxSet* idxset);
extern void         code_value_entry(CodeNode* node, Entry* entry);
extern void         code_value_term(CodeNode* node, Term* term);
extern void         code_value_ineq(CodeNode* node, Ineq* ineq);
extern void         code_value_bool(CodeNode* node, int bool);
extern void         code_value_size(CodeNode* node, int size);
extern void         code_value_list(CodeNode* node, List* list);
extern void         code_value_vartype(CodeNode* node, VarType vartype);
extern void         code_value_ineqtype(CodeNode* node, IneqType ineqtype);
extern void         code_value_void(CodeNode* node);

/* inst.c
 */
/*lint -sem(i_and,            1p == 1) */
/*lint -sem(i_expr_add,       1p == 1) */
/*lint -sem(i_data_par,       1p == 1) */
/*lint -sem(i_expr_div,       1p == 1) */
/*lint -sem(i_elem_list_add,  1p == 1) */
/*lint -sem(i_elem_list_new,  1p == 1) */
/*lint -sem(i_entry,          1p == 1) */
/*lint -sem(i_entry_list_add, 1p == 1) */
/*lint -sem(i_entry_list_new, 1p == 1) */
/*lint -sem(i_eq,             1p == 1) */
/*lint -sem(i_false,          1p == 1) */
/*lint -sem(i_forall,         1p == 1) */
/*lint -sem(i_ineq_new,       1p == 1) */
/*lint -sem(i_idxset_new,     1p == 1) */
/*lint -sem(i_idxset_add,     1p == 1) */
/*lint -sem(i_idxset_set,     1p == 1) */
/*lint -sem(i_idxset_lexpr,   1p == 1) */
/*lint -sem(i_local_deref,    1p == 1) */
/*lint -sem(i_expr_mul,       1p == 1) */
/*lint -sem(i_newsym_numb,    1p == 1) */
/*lint -sem(i_newsym_set,     1p == 1) */
/*lint -sem(i_newsym_var,     1p == 1) */
/*lint -sem(i_nop,            1p == 1) */
/*lint -sem(i_not,            1p == 1) */
/*lint -sem(i_numb_size,      1p == 1) */
/*lint -sem(i_object_min,     1p == 1) */
/*lint -sem(i_once,           1p == 1) */
/*lint -sem(i_or,             1p == 1) */
/*lint -sem(i_set_cross,      1p == 1) */
/*lint -sem(i_set_new,        1p == 1) */
/*lint -sem(i_set_dim,        1p == 1) */
/*lint -sem(i_set_empty,      1p == 1) */
/*lint -sem(i_set_union,      1p == 1) */
/*lint -sem(i_expr_sub,       1p == 1) */
/*lint -sem(i_symbol_deref,   1p == 1) */
/*lint -sem(i_term_coeff,     1p == 1) */
/*lint -sem(i_term_const,     1p == 1) */
/*lint -sem(i_term_add,       1p == 1) */
/*lint -sem(i_term_sub,       1p == 1) */
/*lint -sem(i_term_sum,       1p == 1) */
/*lint -sem(i_true,           1p == 1) */
/*lint -sem(i_tuple_new,      1p == 1) */
/*lint -sem(i_tuple_empty,    1p == 1) */
/*lint -sem(i_tuple_list_add, 1p == 1) */
/*lint -sem(i_tuple_list_new, 1p == 1) */

extern void i_and(CodeNode* self);
extern void i_expr_add(CodeNode* self);
extern void i_data_par(CodeNode* self);
extern void i_expr_div(CodeNode* self);
extern void i_elem_list_add(CodeNode* self);
extern void i_elem_list_new(CodeNode* self);
extern void i_entry(CodeNode* self);
extern void i_entry_list_add(CodeNode* self);
extern void i_entry_list_new(CodeNode* self);
extern void i_eq(CodeNode* self);
extern void i_false(CodeNode* self);
extern void i_forall(CodeNode* self);
extern void i_ineq_new(CodeNode* self);
extern void i_idxset_new(CodeNode* self);
extern void i_idxset_add(CodeNode* self);
extern void i_idxset_set(CodeNode* self);
extern void i_idxset_lexpr(CodeNode* self);
extern void i_local_deref(CodeNode* self);
extern void i_expr_mul(CodeNode* self);
extern void i_newsym_numb(CodeNode* self);
extern void i_newsym_set(CodeNode* self);
extern void i_newsym_var(CodeNode* self);
extern void i_nop(CodeNode* self);
extern void i_not(CodeNode* self);
extern void i_numb_size(CodeNode* self);
extern void i_object_min(CodeNode* self);
extern void i_once(CodeNode* self);
extern void i_or(CodeNode* self);
extern void i_set_cross(CodeNode* self);
extern void i_set_new(CodeNode* self);
extern void i_set_dim(CodeNode* self);
extern void i_set_empty(CodeNode* self);
extern void i_set_union(CodeNode* self);
extern void i_expr_sub(CodeNode* self);
extern void i_symbol_deref(CodeNode* self);
extern void i_term_coeff(CodeNode* self);
extern void i_term_const(CodeNode* self);
extern void i_term_add(CodeNode* self);
extern void i_term_sub(CodeNode* self);
extern void i_term_sum(CodeNode* self);
extern void i_true(CodeNode* self);
extern void i_tuple_new(CodeNode* self);
extern void i_tuple_empty(CodeNode* self);
extern void i_tuple_list_add(CodeNode* self);
extern void i_tuple_list_new(CodeNode* self);

/* mmlparse.y
 */
extern int yyparse(void);

/* mmlscan.l
 */
/*lint -sem(parse_string, 1p && 2n >= 0) */

extern int yylen(void);
extern int parse_string(const char* s, int lineno);
extern int scan_get_lineno(void);
extern int scan_get_column(void);

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

