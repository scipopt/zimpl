#ident "@(#) $Id: inst.h,v 1.6 2002/06/12 09:09:11 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: inst.h                                                        */
/*   Name....: Instruction Functions                                         */
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

#ifndef _INST_H_
#define _INST_H_

#define INST_NULL ((Inst)0)

/* inst.c
 */
/*lint -sem(i_bool_and, 1p == 1) */
extern void i_bool_and(CodeNode* self);
/*lint -sem(i_bool_eq, 1p == 1) */
extern void i_bool_eq(CodeNode* self);
/*lint -sem(i_bool_exists, 1p == 1) */
extern void i_bool_exists(CodeNode* self);
/*lint -sem(i_bool_false, 1p == 1) */
extern void i_bool_false(CodeNode* self);
/*lint -sem(i_bool_ge, 1p == 1) */
extern void i_bool_ge(CodeNode* self);
/*lint -sem(i_bool_gt, 1p == 1) */
extern void i_bool_gt(CodeNode* self);
/*lint -sem(i_bool_is_elem, 1p == 1) */
extern void i_bool_is_elem(CodeNode* self);
/*lint -sem(i_bool_le, 1p == 1) */
extern void i_bool_le(CodeNode* self);
/*lint -sem(i_bool_lt, 1p == 1) */
extern void i_bool_lt(CodeNode* self);
/*lint -sem(i_bool_ne, 1p == 1) */
extern void i_bool_ne(CodeNode* self);
/*lint -sem(i_bool_not, 1p == 1) */
extern void i_bool_not(CodeNode* self);
/*lint -sem(i_bool_or, 1p == 1) */
extern void i_bool_or(CodeNode* self);
/*lint -sem(i_bool_seq, 1p == 1) */
extern void i_bool_seq(CodeNode* self);
/*lint -sem(i_bool_sneq, 1p == 1) */
extern void i_bool_sneq(CodeNode* self);
/*lint -sem(i_bool_sseq, 1p == 1) */
extern void i_bool_sseq(CodeNode* self);
/*lint -sem(i_bool_subs, 1p == 1) */
extern void i_bool_subs(CodeNode* self);
/*lint -sem(i_bool_true, 1p == 1) */
extern void i_bool_true(CodeNode* self);
/*lint -sem(i_data_par, 1p == 1) */
extern void i_data_par(CodeNode* self);
/*lint -sem(i_elem_list_add, 1p == 1) */
extern void i_elem_list_add(CodeNode* self);
/*lint -sem(i_elem_list_new, 1p == 1) */
extern void i_elem_list_new(CodeNode* self);
/*lint -sem(i_entry, 1p == 1) */
extern void i_entry(CodeNode* self);
/*lint -sem(i_entry_list_add, 1p == 1) */
extern void i_entry_list_add(CodeNode* self);
/*lint -sem(i_entry_list_new, 1p == 1) */
extern void i_entry_list_new(CodeNode* self);
/*lint -sem(i_expr_add, 1p == 1) */
extern void i_expr_add(CodeNode* self);
/*lint -sem(i_expr_div, 1p == 1) */
extern void i_expr_div(CodeNode* self);
/*lint -sem(i_expr_if, 1p == 1) */
extern void i_expr_if(CodeNode* self);
/*lint -sem(i_expr_intdiv, 1p == 1) */
extern void i_expr_intdiv(CodeNode* self);
/*lint -sem(i_expr_sum, 1p == 1) */
extern void i_expr_sum(CodeNode* self);
/*lint -sem(i_expr_max, 1p == 1) */
extern void i_expr_max(CodeNode* self);
/*lint -sem(i_expr_min, 1p == 1) */
extern void i_expr_min(CodeNode* self);
/*lint -sem(i_expr_mul, 1p == 1) */
extern void i_expr_mul(CodeNode* self);
/*lint -sem(i_expr_mod, 1p == 1) */
extern void i_expr_mod(CodeNode* self);
/*lint -sem(i_expr_neg, 1p == 1) */
extern void i_expr_neg(CodeNode* self);
/*lint -sem(i_expr_pow, 1p == 1) */
extern void i_expr_pow(CodeNode* self);
/*lint -sem(i_expr_sub, 1p == 1) */
extern void i_expr_sub(CodeNode* self);
/*lint -sem(i_forall, 1p == 1) */
extern void i_forall(CodeNode* self);
/*lint -sem(i_idxset_new, 1p == 1) */
extern void i_idxset_new(CodeNode* self);
/*lint -sem(i_local_deref, 1p == 1) */
extern void i_local_deref(CodeNode* self);
/*lint -sem(i_newsym_para1, 1p == 1) */
extern void i_newsym_para1(CodeNode* self);
/*lint -sem(i_newsym_para2, 1p == 1) */
extern void i_newsym_para2(CodeNode* self);
/*lint -sem(i_newsym_set, 1p == 1) */
extern void i_newsym_set(CodeNode* self);
/*lint -sem(i_newsym_var, 1p == 1) */
extern void i_newsym_var(CodeNode* self);
/*lint -sem(i_nop, 1p == 1) */
extern void i_nop(CodeNode* self);
/*lint -sem(i_object_max, 1p == 1) */
extern void i_object_max(CodeNode* self);
/*lint -sem(i_object_min, 1p == 1) */
extern void i_object_min(CodeNode* self);
/*lint -sem(i_once, 1p == 1) */
extern void i_once(CodeNode* self);
/*lint -sem(i_set_cross, 1p == 1) */
extern void i_set_cross(CodeNode* self);
/*lint -sem(i_set_inter, 1p == 1) */
extern void i_set_inter(CodeNode* self);
/*lint -sem(i_set_new_tuple, 1p == 1) */
extern void i_set_new_tuple(CodeNode* self);
/*lint -sem(i_set_new_elem, 1p == 1) */
extern void i_set_new_elem(CodeNode* self);
/*lint -sem(i_set_minus, 1p == 1) */
extern void i_set_minus(CodeNode* self);
/*lint -sem(i_set_empty, 1p == 1) */
extern void i_set_empty(CodeNode* self);
/*lint -sem(i_set_range, 1p == 1) */
extern void i_set_range(CodeNode* self);
/*lint -sem(i_set_sdiff, 1p == 1) */
extern void i_set_sdiff(CodeNode* self);
/*lint -sem(i_set_union, 1p == 1) */
extern void i_set_union(CodeNode* self);
/*lint -sem(i_symbol_deref, 1p == 1) */
extern void i_symbol_deref(CodeNode* self);
/*lint -sem(i_term_coeff, 1p == 1) */
extern void i_term_coeff(CodeNode* self);
/*lint -sem(i_term_add, 1p == 1) */
extern void i_term_add(CodeNode* self);
/*lint -sem(i_term_sub, 1p == 1) */
extern void i_term_sub(CodeNode* self);
/*lint -sem(i_term_sum, 1p == 1) */
extern void i_term_sum(CodeNode* self);
/*lint -sem(i_tuple_new, 1p == 1) */
extern void i_tuple_new(CodeNode* self);
/*lint -sem(i_tuple_empty, 1p == 1) */
extern void i_tuple_empty(CodeNode* self);
/*lint -sem(i_tuple_list_add, 1p == 1) */
extern void i_tuple_list_add(CodeNode* self);
/*lint -sem(i_tuple_list_new, 1p == 1) */
extern void i_tuple_list_new(CodeNode* self);

/* iread.c
 */
/*lint -sem(i_read_new, 1p == 1) */
extern void i_read_new(CodeNode* self);
/*lint -sem(i_read_param, 1p == 1) */
extern void i_read_param(CodeNode* self);
/*lint -sem(i_read_comment, 1p == 1) */
extern void i_read_comment(CodeNode* self);
/*lint -sem(i_read_use, 1p == 1) */
extern void i_read_use(CodeNode* self);
/*lint -sem(i_read_skip, 1p == 1) */
extern void i_read_skip(CodeNode* self);
/*lint -sem(i_read, 1p == 1) */
extern void i_read(CodeNode* self);

#endif /* _INST_H_ */
