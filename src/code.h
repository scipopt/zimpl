#ident "@(#) $Id: code.h,v 1.1 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: code.h                                                        */
/*   Name....: Code Functions                                                */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _CODE_H_
#define _CODE_H_

enum code_type
{
   CODE_ERR = 0, CODE_NUMB, CODE_STRG, CODE_NAME, CODE_TUPLE,
   CODE_SET, CODE_TERM, CODE_SYMBOL, CODE_BOOL, CODE_SIZE, CODE_INEQ,
   CODE_IDXSET, CODE_LIST, CODE_VOID, CODE_ENTRY, CODE_VARTYPE, CODE_INEQTYPE
};

#define Code_eval_child(node, no)      code_eval(code_get_child(node, no))

#define Code_eval_child_numb(n, i)     code_get_numb(Code_eval_child(n, i))
#define Code_eval_child_strg(n, i)     code_get_strg(Code_eval_child(n, i))
#define Code_eval_child_name(n, i)     code_get_name(Code_eval_child(n, i))
#define Code_eval_child_tuple(n, i)    code_get_tuple(Code_eval_child(n, i))
#define Code_eval_child_set(n, i)      code_get_set(Code_eval_child(n, i))
#define Code_eval_child_idxset(n, i)   code_get_idxset(Code_eval_child(n, i))
#define Code_eval_child_entry(n, i)    code_get_entry(Code_eval_child(n, i))
#define Code_eval_child_term(n, i)     code_get_term(Code_eval_child(n, i))
#define Code_eval_child_ineq(n, i)     code_get_ineq(Code_eval_child(n, i))
#define Code_eval_child_size(n, i)     code_get_size(Code_eval_child(n, i))
#define Code_eval_child_bool(n, i)     code_get_bool(Code_eval_child(n, i))
#define Code_eval_child_list(n, i)     code_get_list(Code_eval_child(n, i))
#define Code_eval_child_vartype(n, i)  code_get_vartype(Code_eval_child(n, i))
#define Code_eval_child_ineqtype(n, i) code_get_ineqtype(Code_eval_child(n, i))

/*lint -sem(        code_new_inst, 1p == 1, @p == 1) */
extern CodeNode*    code_new_inst(Inst inst, int childs, ...);
/*lint -sem(        code_new_numb, 1p == 1, @p == 1) */
extern CodeNode*    code_new_numb(double numb);
/*lint -sem(        code_new_strg, 1p, @p == 1) */
extern CodeNode*    code_new_strg(const char* strg);
/*lint -sem(        code_new_name, 1p, @p == 1) */
extern CodeNode*    code_new_name(const char* name);
/*lint -sem(        code_new_size, @p == 1) */
extern CodeNode*    code_new_size(int size);
/*lint -sem(        code_new_vartype, @p == 1) */
extern CodeNode*    code_new_vartype(VarType vartype);
/*lint -sem(        code_new_ineqtype, @p == 1) */
extern CodeNode*    code_new_ineqtype(IneqType ineqtype);
/*lint -sem(        code_free, 1p == 1) */
extern void         code_free(CodeNode* node);
/*lint -sem(        code_is_valid, 1p == 1) */
extern Bool         code_is_valid(const CodeNode* node);
/*lint -sem(        code_set_child, 1p == 1 && 2n >= 0 && 3p == 1) */
extern void         code_set_child(CodeNode* node, int idx, CodeNode* child);
/*lint -sem(        code_get_type, 1p == 1) */
extern CodeType     code_get_type(CodeNode* node);
/*lint -sem(        code_set_root, 1p == 1) */
extern void         code_set_root(CodeNode* node);
/*lint -sem(        code_get_root, @p == 1) */
extern CodeNode*    code_get_root(void);
/*lint -sem(        code_eval, 1p == 1, @p == 1) */
extern CodeNode*    code_eval(CodeNode* node);
/*lint -sem(        code_get_child, 1p == 1 && 2n >= 0, @p == 1) */
extern CodeNode*    code_get_child(CodeNode* node, int no);
/*lint -sem(        code_get_lineno, 1p == 1) */
extern int          code_get_lineno(const CodeNode* node);
/*lint -sem(        code_get_column, 1p == 1) */
extern int          code_get_column(const CodeNode* node);
/*lint -sem(        code_get_numb, 1p == 1) */
extern double       code_get_numb(CodeNode* node);
/*lint -sem(        code_get_strg, 1p == 1, @p) */
extern const char*  code_get_strg(CodeNode* node);
/*lint -sem(        code_get_name, 1p == 1, @p) */
extern const char*  code_get_name(CodeNode* node);
/*lint -sem(        code_get_tuple, 1p == 1, @p == 1) */
extern Tuple*       code_get_tuple(CodeNode* node);
/*lint -sem(        code_get_set, 1p == 1, @p == 1) */
extern Set*         code_get_set(CodeNode* node);
/*lint -sem(        code_get_idxset, 1p == 1, @p == 1) */
extern IdxSet*      code_get_idxset(CodeNode* node);
/*lint -sem(        code_get_entry, 1p == 1, @p == 1) */
extern Entry*       code_get_entry(CodeNode* node);
/*lint -sem(        code_get_term, 1p == 1, @p == 1) */
extern Term*        code_get_term(CodeNode* node);
/*lint -sem(        code_get_ineq, 1p == 1, @p == 1) */
extern Ineq*        code_get_ineq(CodeNode* node);
/*lint -sem(        code_get_size, 1p == 1) */
extern int          code_get_size(CodeNode* node);
/*lint -sem(        code_get_bool, 1p == 1) */
extern Bool         code_get_bool(CodeNode* node);
/*lint -sem(        code_get_list, 1p == 1, @p == 1) */
extern List*        code_get_list(CodeNode* node);
/*lint -sem(        code_get_vartype, 1p == 1) */
extern VarType      code_get_vartype(CodeNode* node);
/*lint -sem(        code_get_ineqtype, 1p == 1) */
extern IneqType     code_get_ineqtype(CodeNode* node);
/*lint -sem(        code_value_numb, 1p == 1) */
extern void         code_value_numb(CodeNode* node, double numb);
/*lint -sem(        code_value_strg, 1p == 1 && 2p) */
extern void         code_value_strg(CodeNode* node, const char* strg);
/*lint -sem(        code_value_name, 1p == 1 && 2p) */
extern void         code_value_name(CodeNode* node, const char* name);
/*lint -sem(        code_value_tuple, 1p == 1 && 2p == 1) */
extern void         code_value_tuple(CodeNode* node, Tuple* tuple);
/*lint -sem(        code_value_set, 1p == 1 && 2p == 1) */
extern void         code_value_set(CodeNode* node, Set* set);
/*lint -sem(        code_value_idxset, 1p == 1 && 2p == 1) */
extern void         code_value_idxset(CodeNode* node, IdxSet* idxset);
/*lint -sem(        code_value_entry, 1p == 1 && 2p == 1) */
extern void         code_value_entry(CodeNode* node, Entry* entry);
/*lint -sem(        code_value_term, 1p == 1 && 2p == 1) */
extern void         code_value_term(CodeNode* node, Term* term);
/*lint -sem(        code_value_ineq, 1p == 1 && 2p == 1) */
extern void         code_value_ineq(CodeNode* node, Ineq* ineq);
/*lint -sem(        code_value_bool, 1p == 1) */
extern void         code_value_bool(CodeNode* node, Bool bool);
/*lint -sem(        code_value_size, 1p == 1) */
extern void         code_value_size(CodeNode* node, int size);
/*lint -sem(        code_value_list, 1p == 1 && 2p == 1) */
extern void         code_value_list(CodeNode* node, List* list);
/*lint -sem(        code_value_vartype, 1p == 1) */
extern void         code_value_vartype(CodeNode* node, VarType vartype);
/*lint -sem(        code_value_ineqtype, 1p == 1) */
extern void         code_value_ineqtype(CodeNode* node, IneqType ineqtype);
/*lint -sem(        code_value_void, 1p == 1) */
extern void         code_value_void(CodeNode* node);

#endif /* _CODE_H_ */


