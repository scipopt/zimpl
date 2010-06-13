/* $Id: zimpllib.h,v 1.9 2010/06/13 12:37:41 bzfkocht Exp $ */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: zimpllib.h                                                    */
/*   Name....: Zimpl library                                                 */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2005-2010 by Thorsten Koch <koch@zib.de>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _ZIMPLLIB_H_
#define _ZIMPLLIB_H_

#ifndef _BOOL_H_
#error "Need to include bool.h before zimpllib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*lint -sem(        zpl_add_parameter, 1p && nulterm(1)) */
extern void         zpl_add_parameter(const char* def);
/*lint -sem(        zpl_var_print, 1p == 1 && 2p == 1) */
extern void         zpl_var_print(FILE* fp, const Var* var);
/*lint -sem(        zpl_print_banner, 1p == 1) */
extern void         zpl_print_banner(FILE* fp, Bool with_license);

/*lint -sem(zpl_read, nulterm(1)) */
extern Bool         zpl_read(const char* filename, Bool with_management);
/*lint -sem(zpl_read_with_args, 1n > 0 && 2p) */
extern Bool         zpl_read_with_args(char** argv, int argc, Bool with_management);

#ifdef __cplusplus
}
#endif

#endif /* _ZIMPLLIB_H_ */
