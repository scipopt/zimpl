#ident "@(#) $Id: code.h,v 1.8 2002/07/29 09:21:59 bzfkocht Exp $"
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

/*lint -sem(        code_new_inst, 1p == 1 && 2n >= 0, @p == 1) */
extern CodeNode*    code_new_inst(Inst inst, int childs, ...);
/*lint -sem(        code_new_numb, 1p == 1, @p == 1) */
extern CodeNode*    code_new_numb(double numb);
/*lint -sem(        code_new_strg, 1p && nulterm(1), @p == 1) */
extern CodeNode*    code_new_strg(const char* strg);
/*lint -sem(        code_new_name, 1p && nulterm(1), @p == 1) */
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
/*lint -sem(        code_get_type, 1p == 1) */
extern CodeType     code_get_type(const CodeNode* node);
/*lint -sem(        code_get_inst, 1p == 1, @p) */
extern Inst         code_get_inst(const CodeNode* node);
/*lint -sem(        code_set_root, 1p == 1) */
extern void         code_set_root(CodeNode* node);
/*lint -sem(        code_get_root, @p == 1) */
extern CodeNode*    code_get_root(void);
/*lint -sem(        code_set_child, 1p == 1 && 2n >= 0 && 3p == 1) */
extern void         code_set_child(CodeNode* node, int idx, CodeNode* child);
/*lint -sem(        code_errmsg, 1p == 1) */
extern void         code_errmsg(const CodeNode* node);
#if 0
/*lint -sem(        code_check_type, 1p == 1, @p == 1p) */
extern CodeNode*    code_check_type(CodeNode* node, CodeType expected);
#endif
/*lint -sem(        code_eval, 1p == 1, @p == 1p) */
extern CodeNode*    code_eval(CodeNode* node);
/*lint -sem(        code_get_child, 1p == 1 && 2n >= 0, @p == 1) */
extern CodeNode*    code_get_child(const CodeNode* node, int no);
/*lint -sem(        code_get_numb, 1p == 1) */
extern double       code_get_numb(CodeNode* node);
/*lint -sem(        code_get_strg, 1p == 1, @p && nulterm(@)) */
extern const char*  code_get_strg(CodeNode* node);
/*lint -sem(        code_get_name, 1p == 1, @p && nulterm(@)) */
extern const char*  code_get_name(CodeNode* node);
/*lint -sem(        code_get_tuple, 1p == 1, @p == 1) */
extern const Tuple* code_get_tuple(CodeNode* node);
/*lint -sem(        code_get_set, 1p == 1, @p == 1) */
extern const Set*   code_get_set(CodeNode* node);
/*lint -sem(        code_get_idxset, 1p == 1, @p == 1) */
extern const IdxSet* code_get_idxset(CodeNode* node);
/*lint -sem(        code_get_entry, 1p == 1, @p == 1) */
extern const Entry* code_get_entry(CodeNode* node);
/*lint -sem(        code_get_term, 1p == 1, @p == 1) */
extern const Term*  code_get_term(CodeNode* node);
/*lint -sem(        code_get_size, 1p == 1) */
extern int          code_get_size(CodeNode* node);
/*lint -sem(        code_get_bool, 1p == 1) */
extern Bool         code_get_bool(CodeNode* node);
/*lint -sem(        code_get_list, 1p == 1, @p == 1) */
extern const List*  code_get_list(CodeNode* node);
/*lint -sem(        code_get_vartype, 1p == 1) */
extern VarType      code_get_vartype(CodeNode* node);
/*lint -sem(        code_get_contype, 1p == 1) */
extern ConType      code_get_contype(CodeNode* node);
/*lint -sem(        code_get_rdef, 1p == 1, @p == 1) */
extern const RDef*  code_get_rdef(CodeNode* node);
/*lint -sem(        code_get_rpar, 1p == 1, @p == 1) */
extern const RPar*  code_get_rpar(CodeNode* node);
/*lint -sem(        code_get_bits, 1p == 1) */
extern unsigned int code_get_bits(CodeNode* node);
/*lint -sem(        code_value_numb, 1p == 1) */
extern void         code_value_numb(CodeNode* node, double numb);
/*lint -sem(        code_value_strg, 1p == 1 && 2p && nulterm(2)) */
extern void         code_value_strg(CodeNode* node, const char* strg);
/*lint -sem(        code_value_name, 1p == 1 && 2p && nulterm(2)) */
extern void         code_value_name(CodeNode* node, const char* name);
/*lint -sem(        code_value_tuple, 1p == 1 && 2p == 1) */
extern void         code_value_tuple(CodeNode* node, const Tuple* tuple);
/*lint -sem(        code_value_set, 1p == 1 && 2p == 1) */
extern void         code_value_set(CodeNode* node, const Set* set);
/*lint -sem(        code_value_idxset, 1p == 1 && 2p == 1) */
extern void         code_value_idxset(CodeNode* node, const IdxSet* idxset);
/*lint -sem(        code_value_entry, 1p == 1 && 2p == 1) */
extern void         code_value_entry(CodeNode* node, const Entry* entry);
/*lint -sem(        code_value_term, 1p == 1 && 2p == 1) */
extern void         code_value_term(CodeNode* node, const Term* term);
/*lint -sem(        code_value_bool, 1p == 1) */
extern void         code_value_bool(CodeNode* node, Bool bool);
/*lint -sem(        code_value_size, 1p == 1) */
extern void         code_value_size(CodeNode* node, int size);
/*lint -sem(        code_value_list, 1p == 1 && 2p == 1) */
extern void         code_value_list(CodeNode* node, const List* list);
/*lint -sem(        code_value_vartype, 1p == 1) */
extern void         code_value_vartype(CodeNode* node, VarType vartype);
/*lint -sem(        code_value_contype, 1p == 1) */
extern void         code_value_contype(CodeNode* node, ConType contype);
/*lint -sem(        code_value_rdef, 1p == 1 && 2p == 1) */
extern void         code_value_rdef(CodeNode* node, const RDef* rdef);
/*lint -sem(        code_value_rpar, 1p == 1 && 2p == 1) */
extern void         code_value_rpar(CodeNode* node, const RPar* rpar);
/*lint -sem(        code_value_bits, 1p == 1) */
extern void         code_value_bits(CodeNode* node, unsigned int bits);
/*lint -sem(        code_value_void, 1p == 1) */
extern void         code_value_void(CodeNode* node);

/*lint -sem(        code_eval_child, 1p == 1 && 2n >= 0, @p == 1p) */
extern CodeNode*    code_eval_child(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_numb, 1p == 1 && 2n >= 0) */
extern double       code_eval_child_numb(const CodeNode* node, int no);
/*lint -sem(        char* code_eval_child_strg, 1p == 1 && 2n >= 0,
                    @p && nulterm(@)) */
extern const char*  code_eval_child_strg(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_name, 1p == 1 && 2n >= 0,
                    @p && nulterm(@)) */
extern const char*  code_eval_child_name(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_tuple, 1p == 1 && 2n >= 0, @p == 1) */
extern const Tuple* code_eval_child_tuple(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_set, 1p == 1 && 2n >= 0, @p == 1) */
extern const Set*   code_eval_child_set(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_idxset, 1p == 1 && 2n >= 0, @p == 1) */
extern const IdxSet* code_eval_child_idxset(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_entry, 1p == 1 && 2n >= 0, @p == 1) */
extern const Entry* code_eval_child_entry(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_term, 1p == 1 && 2n >= 0, @p == 1) */
extern const Term*  code_eval_child_term(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_size, 1p == 1 && 2n >= 0, @n >= 0) */
extern int          code_eval_child_size(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_bool, 1p == 1 && 2n >= 0) */
extern Bool         code_eval_child_bool(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_list, 1p == 1 && 2n >= 0, @p == 1) */
extern const List*  code_eval_child_list(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_vartype, 1p == 1 && 2n >= 0) */
extern VarType      code_eval_child_vartype(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_contype, 1p == 1 && 2n >= 0) */
extern ConType      code_eval_child_contype(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_rdef, 1p == 1 && 2n >= 0, @p == 1) */
extern const RDef*  code_eval_child_rdef(const CodeNode* node, int no);
/*lint -sem(        code_eval_child_rpar, 1p == 1 && 2n >= 0, @p == 1) */
extern const RPar*  code_eval_child_rpar(const CodeNode* node, int no);
/*lint -sem(        int code_eval_child_bits, 1p == 1 && 2n >= 0) */
extern unsigned     int code_eval_child_bits(const CodeNode* node, int no);

#endif /* _CODE_H_ */







