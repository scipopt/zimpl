#ident "@(#) $Id: code.h,v 1.5 2002/05/26 12:44:57 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: code.h                                                        */
/*   Name....: Code Functions                                                */
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

#ifndef _CODE_H_
#define _CODE_H_

#define Code_eval_child(node, no)      code_eval(code_get_child(node, no))

#define Code_eval_child_numb(n, i)     code_get_numb(Code_eval_child(n, i))
#define Code_eval_child_strg(n, i)     code_get_strg(Code_eval_child(n, i))
#define Code_eval_child_name(n, i)     code_get_name(Code_eval_child(n, i))
#define Code_eval_child_tuple(n, i)    code_get_tuple(Code_eval_child(n, i))
#define Code_eval_child_set(n, i)      code_get_set(Code_eval_child(n, i))
#define Code_eval_child_idxset(n, i)   code_get_idxset(Code_eval_child(n, i))
#define Code_eval_child_entry(n, i)    code_get_entry(Code_eval_child(n, i))
#define Code_eval_child_term(n, i)     code_get_term(Code_eval_child(n, i))
#define Code_eval_child_size(n, i)     code_get_size(Code_eval_child(n, i))
#define Code_eval_child_bool(n, i)     code_get_bool(Code_eval_child(n, i))
#define Code_eval_child_list(n, i)     code_get_list(Code_eval_child(n, i))
#define Code_eval_child_vartype(n, i)  code_get_vartype(Code_eval_child(n, i))
#define Code_eval_child_contype(n, i)  code_get_contype(Code_eval_child(n, i))
#define Code_eval_child_rdef(n, i)     code_get_rdef(Code_eval_child(n, i))
#define Code_eval_child_rpar(n, i)     code_get_rpar(Code_eval_child(n, i))
#define Code_eval_child_bits(n, i)     code_get_bits(Code_eval_child(n, i))

/*lint -sem(        code_new_inst, 1p == 1 && 2n >= 0, @p == 1) */
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
/*lint -sem(        code_new_contype, @p == 1) */
extern CodeNode*    code_new_contype(ConType contype);
/*lint -sem(        code_new_bits, @p == 1) */
extern CodeNode*    code_new_bits(unsigned int bits);
/*lint -sem(        code_free, 1p == 1) */
extern void         code_free(CodeNode* node);
/*lint -sem(        code_is_valid, 1p == 1) */
extern Bool         code_is_valid(const CodeNode* node);
/*lint -sem(        code_set_child, 1p == 1 && 2n >= 0 && 3p == 1) */
extern void         code_set_child(CodeNode* node, int idx, CodeNode* child);
/*lint -sem(        code_get_type, 1p == 1) */
extern CodeType     code_get_type(const CodeNode* node);
/*lint -sem(        code_set_root, 1p == 1) */
extern void         code_set_root(CodeNode* node);
/*lint -sem(        code_get_root, @p == 1) */
extern CodeNode*    code_get_root(void);
/*lint -sem(        code_eval, 1p == 1, @p == 1) */
extern CodeNode*    code_eval(CodeNode* node);
/*lint -sem(        code_get_child, 1p == 1 && 2n >= 0, @p == 1) */
extern CodeNode*    code_get_child(const CodeNode* node, int no);
/*lint -sem(        code_errmsg, 1p == 1) */
extern void         code_errmsg(const CodeNode* node);
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
/*lint -sem(        code_get_size, 1p == 1) */
extern int          code_get_size(CodeNode* node);
/*lint -sem(        code_get_bool, 1p == 1) */
extern Bool         code_get_bool(CodeNode* node);
/*lint -sem(        code_get_list, 1p == 1, @p == 1) */
extern List*        code_get_list(CodeNode* node);
/*lint -sem(        code_get_vartype, 1p == 1) */
extern VarType      code_get_vartype(CodeNode* node);
/*lint -sem(        code_get_contype, 1p == 1) */
extern ConType      code_get_contype(CodeNode* node);
/*lint -sem(        code_get_rdef, 1p == 1, @p == 1) */
extern RDef*        code_get_rdef(CodeNode* node);
/*lint -sem(        code_get_rpar, 1p == 1, @p == 1) */
extern RPar*        code_get_rpar(CodeNode* node);
/*lint -sem(        code_get_bits, 1p == 1) */
extern unsigned int code_get_bits(CodeNode* node);
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
extern void         code_value_idxset(CodeNode* node, const IdxSet* idxset);
/*lint -sem(        code_value_entry, 1p == 1 && 2p == 1) */
extern void         code_value_entry(CodeNode* node, Entry* entry);
/*lint -sem(        code_value_term, 1p == 1 && 2p == 1) */
extern void         code_value_term(CodeNode* node, const Term* term);
/*lint -sem(        code_value_bool, 1p == 1) */
extern void         code_value_bool(CodeNode* node, Bool bool);
/*lint -sem(        code_value_size, 1p == 1) */
extern void         code_value_size(CodeNode* node, int size);
/*lint -sem(        code_value_list, 1p == 1 && 2p == 1) */
extern void         code_value_list(CodeNode* node, List* list);
/*lint -sem(        code_value_vartype, 1p == 1) */
extern void         code_value_vartype(CodeNode* node, VarType vartype);
/*lint -sem(        code_value_contype, 1p == 1) */
extern void         code_value_contype(CodeNode* node, ConType contype);
/*lint -sem(        code_value_rdef, 1p == 1 && 2p == 1) */
extern void         code_value_rdef(CodeNode* node, RDef* rdef);
/*lint -sem(        code_value_rpar, 1p == 1 && 2p == 1) */
extern void         code_value_rpar(CodeNode* node, const RPar* rpar);
/*lint -sem(        code_value_bits, 1p == 1) */
extern void         code_value_bits(CodeNode* node, unsigned int bits);
/*lint -sem(        code_value_void, 1p == 1) */
extern void         code_value_void(CodeNode* node);

#endif /* _CODE_H_ */







