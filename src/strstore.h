/* $Id: strstore.h,v 1.2 2010/06/13 12:37:41 bzfkocht Exp $ */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: strstore2.c                                                   */
/*   Name....: String Storage Functions                                      */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2010 by Thorsten Koch <koch@zib.de>
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
#ifndef _STRSTORE_H_
#define _STRSTORE_H_

extern void         str_init(void);
extern void         str_exit(void);
/*lint -sem(        str_new, 1p && nulterm(1), @p == 1p && nulterm(@)) */
extern const char*  str_new(const char* s);
/*lint -sem(        str_hash, 1p)           */
extern unsigned int str_hash(const char* s);

#endif /* _STRSTORE_H_ */
