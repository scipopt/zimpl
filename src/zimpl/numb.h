/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: numb.c                                                        */
/*   Name....: Number Functions                                              */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2018 by Thorsten Koch <koch@zib.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _NUMB_H_
#define _NUMB_H_

typedef struct number            Numb;

 /* numbgmp.c
 */
extern void         numb_init(bool with_management);
extern void         numb_exit(void);
/*lint -sem(        numb_new, @P >= malloc(1)) */
extern Numb*        numb_new(void);
/*lint -sem(        numb_new_ascii, nulterm(1), 1P >= 1, @P >= malloc(1)) */
extern Numb*        numb_new_ascii(const char* val);
/*lint -sem(        numb_new_integer, @P >= malloc(1)) */
extern Numb*        numb_new_integer(int val);

/*lint -sem(        numb_free, custodial(1), inout(1), cleanup, 1P >= 1) */
extern void         numb_free(Numb* numb);
/*lint -sem(        numb_is_valid, pure, 1P >= 1) */
extern bool         numb_is_valid(const Numb* numb);

/*lint -sem(        numb_copy, 1p == 1, @P >= malloc(1)) */
extern Numb*        numb_copy(const Numb* source);
/*lint -sem(        numb_equal, pure, 1P >= 1, 2P >= 1) */
extern bool         numb_equal(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_cmp, pure, 1P >= 1, 2P >= 1) */
extern int          numb_cmp(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_set, inout(1), 1P >= 1, 2P >= 1) */
extern void         numb_set(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_add, inout(1), 1P >= 1, 2P >= 1) */
extern void         numb_add(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_add, 1P >= 1, 2p, @P >= malloc(1)) */
extern Numb*        numb_new_add(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_sub, inout(1), 1P >= 1, 2P >= 1) */
extern void         numb_sub(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_sub, 1P >= 1, 2p, @P >= malloc(1)) */
extern Numb*        numb_new_sub(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_mul, inout(1), 1P >= 1, 2P >= 1) */
extern void         numb_mul(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_mul, 1P >= 1, 2p, @P >= malloc(1)) */
extern Numb*        numb_new_mul(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_div, inout(1), 1P >= 1, 2P >= 1) */
extern void         numb_div(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_div, 1P >= 1, 2p, @P >= malloc(1)) */
extern Numb*        numb_new_div(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_intdiv, inout(1), 1P >= 1, 2P >= 1) */
extern void         numb_intdiv(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_intdiv, 1P >= 1, 2p, @P >= malloc(1)) */
extern Numb*        numb_new_intdiv(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_pow, 1P >= 1, chneg(2), @P >= malloc(1)) */
extern Numb*        numb_new_pow(const Numb* base, int expo);
/*lint -sem(        numb_new_fac, chneg(1), @P >= malloc(1)) */
extern Numb*        numb_new_fac(int n);
/*lint -sem(        numb_mod, inout(1), 1P >= 1, 2P >= 1) */
extern void         numb_mod(Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_new_mod, 1P >= 1, 2p, @P >= malloc(1)) */
extern Numb*        numb_new_mod(const Numb* numb_a, const Numb* numb_b);
/*lint -sem(        numb_neg, inout(1), 1P >= 1) */
extern void         numb_neg(Numb* numb);
/*lint -sem(        numb_abs, inout(1), 1P >= 1) */
extern void         numb_abs(Numb* numb);
/*lint -sem(        numb_sgn, inout(1), 1P >= 1) */
extern void         numb_sgn(Numb* numb);
/*lint -sem(        numb_get_sgn, 1P >= 1, @n >= -1 && @n <= 1) */
extern int          numb_get_sgn(const Numb* numb);
/*lint -sem(        numb_round, inout(1), 1P >= 1) */
extern void         numb_round(Numb* numb);
/*lint -sem(        numb_ceil, inout(1), 1P >= 1) */
extern void         numb_ceil(Numb* numb);
/*lint -sem(        numb_floor, inout(1), 1P >= 1) */
extern void         numb_floor(Numb* numb);
/*lint -sem(        numb_new_log, 1P >= 1, @P >= malloc(1) || @P == 0) */
extern Numb*        numb_new_log(const Numb* numb);
/*lint -sem(        numb_new_sqrt, 1P >= 1, @P >= malloc(1) || @P == 0) */
extern Numb*        numb_new_sqrt(const Numb* numb);
/*lint -sem(        numb_new_exp, 1P >= 1, @P >= malloc(1)) */
extern Numb*        numb_new_exp(const Numb* numb);
/*lint -sem(        numb_new_ln, 1P >= 1, @P >= malloc(1) || @P == 0) */
extern Numb*        numb_new_ln(const Numb* numb);
/*lint -sem(        numb_new_rand, 1P >= 1, 2p, @P >= malloc(1)) */
extern Numb*        numb_new_rand(const Numb* mini, const Numb* maxi);
/*lint -sem(        numb_todbl, pure, 1P >= 1) */
extern double       numb_todbl(const Numb* numb);
/*lint -sem(        numb_print, 1P >= 1, 2P >= 1) */
extern void         numb_print(FILE* fp, const Numb* numb);
/*lint -sem(        numb_hash, pure, 1P >= 1) */
extern unsigned int numb_hash(const Numb* numb);
/*lint -sem(        numb_tostr, 1P >= 1, @p) */
extern char*        numb_tostr(const Numb* numb);
/*lint -sem(        numb_zero, @P >= 1) */
extern const Numb*  numb_zero(void);
/*lint -sem(        numb_one, @P >= 1) */
extern const Numb*  numb_one(void);
/*lint -sem(        numb_minusone, @P >= 1) */
extern const Numb*  numb_minusone(void);
/*lint -sem(        numb_unknown, @P >= 1) */
extern const Numb*  numb_unknown(void);
/*lint -sem(        numb_is_int, pure, 1P >= 1) */
extern bool         numb_is_int(const Numb* numb);
/*lint -sem(        numb_toint, pure, 1P >= 1) */
extern int          numb_toint(const Numb* numb);
/*lint -sem(        numb_is_number, 1P >= 1) */
extern bool         numb_is_number(const char *s);

#ifdef __GMP_H__
/*lint -sem(        numb_new_mpq, @P >= malloc(1)) */
extern Numb*        numb_new_mpq(const mpq_t val);
/*lint -sem(        numb_new_mpq, 1P >= 1) */
extern void         numb_get_mpq(const Numb* numb, mpq_t value);
#endif /* __GMP_H__ */

#endif /* _NUMB_H_ */
