#ident "@(#) $Id: inst.h,v 1.1 2001/01/30 19:14:10 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: inst.h                                                        */
/*   Name....: Instruction Functions                                         */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _INST_H_
#define _INST_H_

#define INST_NULL ((Inst)0)

/*lint -sem(i_bool_and, 1p == 1) */
extern void i_bool_and(CodeNode* self);
/*lint -sem(i_bool_eq, 1p == 1) */
extern void i_bool_eq(CodeNode* self);
/*lint -sem(i_bool_false, 1p == 1) */
extern void i_bool_false(CodeNode* self);
/*lint -sem(i_bool_ge, 1p == 1) */
extern void i_bool_ge(CodeNode* self);
/*lint -sem(i_bool_gt, 1p == 1) */
extern void i_bool_gt(CodeNode* self);
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
/*lint -sem(i_expr_mul, 1p == 1) */
extern void i_expr_mul(CodeNode* self);
/*lint -sem(i_expr_neg, 1p == 1) */
extern void i_expr_neg(CodeNode* self);
/*lint -sem(i_expr_sub, 1p == 1) */
extern void i_expr_sub(CodeNode* self);
/*lint -sem(i_forall, 1p == 1) */
extern void i_forall(CodeNode* self);
/*lint -sem(i_ineq_new, 1p == 1) */
extern void i_ineq_new(CodeNode* self);
/*lint -sem(i_idxset_new, 1p == 1) */
extern void i_idxset_new(CodeNode* self);
/*lint -sem(i_idxset_add, 1p == 1) */
extern void i_idxset_add(CodeNode* self);
/*lint -sem(i_idxset_set, 1p == 1) */
extern void i_idxset_set(CodeNode* self);
/*lint -sem(i_idxset_lexpr, 1p == 1) */
extern void i_idxset_lexpr(CodeNode* self);
/*lint -sem(i_local_deref, 1p == 1) */
extern void i_local_deref(CodeNode* self);
/*lint -sem(i_newsym_numb, 1p == 1) */
extern void i_newsym_numb(CodeNode* self);
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
/*lint -sem(i_set_new, 1p == 1) */
extern void i_set_new(CodeNode* self);
/*lint -sem(i_set_empty, 1p == 1) */
extern void i_set_empty(CodeNode* self);
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

#endif /* _INST_H_ */
