#pragma ident "@(#) $Id: gmpmisc.h,v 1.1 2003/07/12 15:24:01 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: gmpmisc.c                                                     */
/*   Name....: miscellenious rational arithmetic functions                   */
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
#ifndef _GMPMISC_H_
#define _GMPMISC_H_

#ifndef __GMP_H__
#error "Need to include gmp.h before gmpmisc.h"
#endif
#ifndef _BOOL_H_
#error "Need to include bool.h before gmpmisc.h"
#endif

extern mpq_t const_zero;
extern mpq_t const_one;
extern mpq_t const_minus_one;

/*lint -sem(gmp_str2mpq, 1p && 2p && nulterm(2)) */
extern void gmp_str2mpq(mpq_t value, const char* num);
/*lint -sem(gmp_print, 1p == 1 && 2p ) */
extern void gmp_print_mpq(FILE* fp, const mpq_t qval);
extern void gmp_init(Bool verb);
extern void gmp_exit(void);

#endif /* _GMPMISC_H */
