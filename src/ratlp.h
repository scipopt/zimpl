#pragma ident "@(#) $Id: ratlp.h,v 1.4 2003/08/18 12:55:58 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: ratlp.h                                                       */
/*   Name....: Rational Number LP Storage Library                            */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2003 by Thorsten Koch <koch@zib.de>
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
#ifndef _RATLP_H_
#define _RATLP_H_

#ifndef __GMP_H__
#error "Need to include gmp.h before ratlp.h"
#endif
#ifndef _BOOL_H_
#error "Need to include bool.h before ratlp.h"
#endif

/* The reason for the extra types is that those do not need gmp.h
 */
#include "ratlptypes.h"

/* lpstore.c
 */
/*lint -sem(        lps_alloc, 1p && nulterm(1), @p == 1) */
extern Lps*         lps_alloc(const char* name);
/*lint -sem(        lps_free, 1p) */
extern void         lps_free(Lps* lp);
/*lint -sem(        lps_number, 1p == 1) */
extern void         lps_number(const Lps* lp);
/*lint -sem(        lps_getvar, 1p && 2p && nulterm(2)) */
extern Var*         lps_getvar(const Lps* lp, const char* name);
/*lint -sem(        lps_getcon, 1p && 2p && nulterm(2)) */
extern Con*         lps_getcon(const Lps* lp, const char* name);
/*lint -sem(        lps_getnzo, 1p == 1 && 2p == 1 && 3p == 1) */
extern Nzo*         lps_getnzo(const Lps* lp, const Con* con, const Var* var);
/*lint -sem(        lps_addvar, 1p == 1 && 2p && nulterm(2), @p == 1) */
extern Var*         lps_addvar(Lps* lp, const char* name); 
/*lint -sem(        lps_delvar, 1p == 1 && 2p == 1) */
extern void         lps_delvar(Lps* lp, Var* var);
/*lint -sem(        lps_addcon, 1p == 1 && 2p && nulterm(2), @p == 1) */
extern Con*         lps_addcon(Lps* lp, const char* name);
/*lint -sem(        lps_delcon, 1p == 1 && 2p == 1) */
extern void         lps_delcon(Lps* lp, Con* con);
/*lint -sem(        lps_addnzo, 1p == 1 && 2p == 1 && 3p == 1) */
extern void         lps_addnzo(Lps* lp, Con* con, Var* var, const mpq_t value);
/*lint -sem(        lps_delnzo, 1p == 1 && 2p == 2) */
extern void         lps_delnzo(Lps* lp, Nzo* nzo);
/*lint -sem(        lps_stat, 1p == 1) */
extern void         lps_stat(const Lps* lp);
/*lint -sem(        lps_setval, 1p == 1) */
extern void         lps_setval(Nzo* nzo, const mpq_t value);
/*lint -sem(        lps_getval, 1p == 1) */
extern void         lps_getval(const Nzo* nzo, mpq_t value);
/*lint -sem(        lps_stat, 1p == 1) */
extern void         lps_setdir(Lps* lp, LpDirect direct);
/*lint -sem(        lps_setprobname, 1p && 2p && nulterm(2)) */
extern void         lps_setprobname(Lps* lp, const char* name);
/*lint -sem(        lps_setobjname, 1p && 2p && nulterm(2)) */
extern void         lps_setobjname(Lps* lp, const char* name);
/*lint -sem(        lps_setrhsname, 1p && 2p && nulterm(2)) */
extern void         lps_setrhsname(Lps* lp, const char* name);
/*lint -sem(        lps_setbndname, 1p && 2p && nulterm(2)) */
extern void         lps_setbndname(Lps* lp, const char* name);
/*lint -sem(        lps_setrngname, 1p && 2p && nulterm(2)) */
extern void         lps_setrngname(Lps* lp, const char* name);
/*lint -sem(        lps_getcost, 1p == 1) */
extern void         lps_getcost(const Var* var, mpq_t cost);
/*lint -sem(        lps_setcost, 1p == 1) */
extern void         lps_setcost(Var* var, const mpq_t cost);
/*lint -sem(        lps_setlower, 1p == 1) */
extern void         lps_setlower(Var* var, const mpq_t lower);
/*lint -sem(        lps_setupper, 1p == 1) */
extern void         lps_setupper(Var* var, const mpq_t upper);
/*lint -sem(        lps_class, 1p == 1) */
extern VarClass     lps_class(const Var* var);
/*lint -sem(        lps_setclass, 1p == 1) */
extern void         lps_setclass(Var* var, VarClass class);
/*lint -sem(        lps_setlhs, 1p == 1) */
extern void         lps_setlhs(Con* con, const mpq_t lhs);
/*lint -sem(        lps_setrhs, 1p == 1) */
extern void         lps_setrhs(Con* con, const mpq_t rhs);
/*lint -sem(        lps_setcontype, 1p == 1) */
extern void         lps_setcontype(Con* con, ConType type);
/*lint -sem(        lps_contype, 1p == 1) */
extern ConType      lps_contype(const Con* con);
/*lint -sem(        lps_vartype, 1p == 1) */
extern VarType      lps_vartype(const Var* var);
/*lint -sem(        lps_getlhs, 1p == 1) */
extern void         lps_getlhs(const Con* con, mpq_t lhs);
/*lint -sem(        lps_getrhs, 1p == 1) */
extern void         lps_getrhs(const Con* con, mpq_t rhs);
/*lint -sem(        lps_setvartype, 1p == 1) */
extern void         lps_setvartype(Var* con, VarType type);
/*lint -sem(        lps_varstate, 1p == 1) */
extern VarState     lps_varstate(const Var* var);
/*lint -sem(        lps_setvarstate, 1p == 1) */
extern void         lps_setvarstate(Var* con, VarState state);
/*lint -sem(        lps_constate, 1p == 1) */
extern ConState     lps_constate(const Con* var);
/*lint -sem(        lps_setconstate, 1p == 1) */
extern void         lps_setconstate(Con* con, ConState state);
/*lint -sem(        lps_flags, 1p == 1) */
extern unsigned int lps_flags(const Con* con);
/*lint -sem(        lps_addflags, 1p == 1) */
extern void         lps_addflags(Con* con, unsigned int flags);
/*lint -sem(        lps_setscale, 1p == 1) */
extern void         lps_setscale(Con* con, const mpq_t scale);
/*lint -sem(        lps_setpriority, 1p == 1) */
extern void         lps_setpriority(Var* var, int priority);
/*lint -sem(        lps_setvalue, 1p == 1) */
extern void         lps_setvalue(Var* var, const mpq_t value);
/*lint -sem(        lps_setstartval, 1p == 1) */
extern void         lps_setstartval(Var* var, const mpq_t startval);
/*lint -sem(        lps_write, 1p == 1 && 2p == 1 && nulterm(4)) */
extern void         lps_write(const Lps* lp, FILE* fp, LpFormat format, const char* text);
/*lint -sem(        lps_transtable, 1p == 1 && 2p == 1 && 3n > 0 && 4p && nulterm(4)) */
extern void         lps_transtable(const Lps* lp, FILE* fp, int size, const char* head);
/*lint -sem(        lps_scale, 1p == 1) */
extern void         lps_scale(const Lps* lp);


/* ratmpsread.c
 */
/*lint -sem(        lps_readmps, 1p && nulterm(1) */
extern Lps*         lps_readmps(const char* filename);

/* ratlpfwrite.c
 */
/*lint -sem(        lpf_write, 1p == 1 && 2p == 1 && nulterm(3)) */
extern void         lpf_write(const Lps* lp, FILE* fp, const char* text);

/* ratmpswrite.c
 */
/*lint -sem(        lpf_write, 1p == 1 && 2p == 1 && nulterm(3)) */
extern void         mps_write(const Lps* lp, FILE* fp, const char* text);

/* ratordwrite.c
 */
/*lint -sem(        lps_ordwrite, 1p == 1 && 2p == 1) */
extern void         lps_orderfile(const Lps* lp, FILE* fp);

/* ratpresolve.c
 */
/*lint -sem(        lps_presolve, 1p == 1 && 2n >= 0) */
extern PSResult     lps_presolve(Lps* lp, int verbose_level);

#endif /* _RATLP_H_ */