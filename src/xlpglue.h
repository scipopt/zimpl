#pragma ident "@(#) $Id: xlpglue.h,v 1.2 2003/07/16 08:48:03 bzfkocht Exp $"
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

extern void xlp_alloc(const char* name);
extern void xlp_free(void);
extern void xlp_scale(void);
extern void xlp_write(FILE* fp, LpFormat format);
extern void xlp_transtable(FILE* fp);
extern void xlp_orderfile(FILE* fp);
extern Bool xlp_conname_exists(const char* conname);
extern Con* xlp_addcon(const char* name, VarType type, const Numb* rhs, unsigned int flags);
extern Var* xlp_addvar(const char* name, VarClass usevarclass,
   const Bound* lower, const Bound* upper, const Numb* priority, const Numb* startval);
extern void xlp_objname(const char* name);
extern void xlp_setdir(Bool minimize);
extern void xlp_addtonzo(Var* var, Con* von, const Numb* numb);
extern void xlp_addtocost(Var* var, const Numb* cost);

#endif /* _XLPGLUE_H */
