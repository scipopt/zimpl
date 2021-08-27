/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: xlpglue.h                                                     */
/*   Name....: Glue between numb/term and ratlp                              */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2003-2020 by Thorsten Koch <koch@zib.de>
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
#ifndef _XLPGLUE_H_
#define _XLPGLUE_H_

#ifndef _RATLPTYPES_H_
#error "Need to include ratlptypes.h before xlpglue.h"
#endif

#ifndef _MME_H_
#error "Need to include mme.h before xlpglue.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//lint -sem(    xlp_alloc, 1p, @P >= malloc(1)) 
extern Lps*     xlp_alloc(const char* name, bool need_startval, void* user_data) expects_NONNULL1 returns_NONNULL;
//lint -sem(    xlp_free, custodial(1), inout(1), 1p == 1) 
extern void     xlp_free(Lps* lp) expects_NONNULL;
//lint -sem(    xlp_conname_exists, 1p == 1, 2p) 
extern bool     xlp_conname_exists(const Lps* lp, const char* conname) expects_NONNULL is_PURE;
//lint -sem(    xlp_addcon_term, inout(1), 1p == 1, 2p, 4p == 1, 5p == 1, 7p == 1) 
extern bool     xlp_addcon_term(Lps* lp, const char* name, ConType type,
   const Numb* lhs, const Numb* rhs, unsigned int flags, const Term* term) expects_NONNULL;
//lint -sem(    xlp_addvar, inout(1), 1p == 1, 2p, 4p == 1, 5p == 1, 6p == 1, 7p == 1, @p == 1) 
extern Var*     xlp_addvar(Lps* lp, const char* name, VarClass usevarclass,
   const Bound* lower, const Bound* upper, const Numb* priority, const Numb* startval) expects_NONNULL returns_NONNULL;
//lint -sem(    xlp_addsos_term, inout(1), 1p == 1, 2p, 4p == 1, 5p == 1) 
extern int      xlp_addsos_term(Lps* lp, const char* name, SosType type,
   const Numb* priority, const Term* term) expects_NONNULL;
//lint -sem(    xlp_getvarname, 1p == 1, 2p == 1, @p) 
const char*     xlp_getvarname(const Lps* lp, const Var* var) expects_NONNULL returns_NONNULL is_PURE;
//lint -sem(    xlp_getclass, 1p == 1, 2p == 1) 
extern VarClass xlp_getclass(const Lps* lp, const Var* var) expects_NONNULL is_PURE;
//lint -sem(    xlp_getlower, 1p == 1, 2p == 1, @p == 1) 
extern Bound*   xlp_getlower(const Lps* lp, const Var* var) expects_NONNULL returns_NONNULL;
//lint -sem(    xlp_getupper, 1p == 1, 2p == 1, @p == 1) 
extern Bound*   xlp_getupper(const Lps* lp, const Var* var) expects_NONNULL returns_NONNULL;
//lint -sem(    xlp_setobj, inout(1), 1p == 1, 2p) 
extern bool     xlp_setobj(Lps* lp, const char* name, bool minimize) expects_NONNULL;
//lint -sem(    xlp_addtocost, inout(1), 1p == 1, inout(2), 2p == 1, 3p == 1) 
extern void     xlp_addtocost(Lps* lp, Var* var, const Numb* cost) expects_NONNULL;
//lint -sem(    xlp_addobjqme, inout(1), 1p == 1, inout(2), 2p == 1, inout(3), 3p == 1, 4p == 1) 
extern void     xlp_addobjqme(Lps* lp, Var* var1, Var* var2, const Numb* cost) expects_NONNULL;
//lint -sem(    xlp_addtermtoobj, inout(1), 1p == 1, 2p == 1) 
extern void     xlp_addtermtoobj(Lps* lp, const Term* term) expects_NONNULL;
   
#ifdef __cplusplus
}
#endif
#endif // _XLPGLUE_H 








