#pragma ident "@(#) $Id: xlpglue.h,v 1.15 2006/08/22 20:11:09 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: xlpglue.h                                                     */
/*   Name....: Glue between numb/term and ratlp                              */
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
#ifndef _XLPGLUE_H_
#define _XLPGLUE_H_

#ifndef _BOOL_H_
#error "Need to include bool.h before xlpglue.h"
#endif

#ifndef _RATLPTYPES_H_
#error "Need to include ratlptypes.h before xlpglue.h"
#endif

#ifndef _MME_H_
#error "Need to include mme.h before xlpglue.h"
#endif

/*lint -sem(    xlp_alloc, nulterm(1), 1p) */
extern void     xlp_alloc(const char* name);
extern void     xlp_free(void);
extern void     xlp_stat(void);
extern void     xlp_scale(void);
/*lint -sem(    xlp_write, 1p == 1) */
extern void     xlp_write(FILE* fp, LpFormat format);
/*lint -sem(    xlp_transtable, 1p == 1) */
extern void     xlp_transtable(FILE* fp, LpFormat format);
/*lint -sem(    xlp_orderfile, 1p == 1) */
extern void     xlp_orderfile(FILE* fp, LpFormat format);
/*lint -sem(    xlp_mstfile, 1p == 1) */
extern void     xlp_mstfile(FILE* fp, LpFormat format);
/*lint -sem(    xlp_sosfile, 1p == 1) */
extern void     xlp_sosfile(FILE* fp, LpFormat format);
/*lint -sem(    xlp_conname, nulterm(1), 1p) */
extern Bool     xlp_conname_exists(const char* conname);
/*lint -sem(    xlp_addcon, nulterm(1), 1p && 3p == 1 && 4p == 1, @p == 1) */
extern Con*     xlp_addcon(const char* name, ConType type,
   const Numb* lhs, const Numb* rhs, unsigned int flags);
/*lint -sem(    xlp_addvar, nulterm(1), 1p && 3p == 1 && 4p == 1 && 5p == 1 && 6p == 1, @p == 1) */
extern Var*     xlp_addvar(const char* name, VarClass usevarclass,
   const Bound* lower, const Bound* upper, const Numb* priority, const Numb* startval);
/*lint -sem(    xlp_addsos, nulterm(1), 1p && 3p == 1, @p == 1) */
extern Sos*     xlp_addsos(const char* name, SosType type, const Numb* priority);
/*lint -sem(    xlp_addsse, nulterm(1), 1p && 3p == 1, @p == 1) */
extern void     xlp_addtosos(Sos* sos, Var* var, const Numb* weight);
/*lint -sem(    xlp_getclass, 1p == 1) */
extern VarClass xlp_getclass(const Var* var);
/*lint -sem(    xlp_getlower, 1p == 1, @p == 1) */
extern Bound*   xlp_getlower(const Var* var);
/*lint -sem(    xlp_getupper, 1p == 1, @p == 1) */
extern Bound*   xlp_getupper(const Var* var);
/*lint -sem(    xlp_objname, nulterm(1), 1p) */
extern void     xlp_objname(const char* name);
extern void     xlp_setdir(Bool minimize);
/*lint -sem(    xlp_addtonzo, 1p == 1 && 2p == 1 && 3p == 1) */
extern void     xlp_addtonzo(Var* var, Con* von, const Numb* numb);
/*lint -sem(    xlp_addtocost, 1p == 1 && 2p == 1) */
extern void     xlp_addtocost(Var* var, const Numb* cost);
extern Bool     xlp_presolve(void);
extern Bool     xlp_hassos(void);
extern void     xlp_setnamelen(int name_len);
#endif /* _XLPGLUE_H */








