/* $Id: rbt.h,v 1.1 2001/03/09 16:12:36 bzfkocht Exp $ */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   Datei...: rbt.h                                                         */
/*   Name....: Red-Black-Tree                                                */
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

#ifndef _RBT_H_
#define _RBT_H_

typedef struct rbt_header RBT;

#define RBT_STORE_REFERENCE   0

extern RBT*  rbt_init(unsigned int, int (*)(const void*, const void*));
extern void  rbt_exit(RBT*);
extern void  rbt_insert(RBT*, void* key, unsigned int);
extern void* rbt_search(const RBT*, const void* key);
extern void  rbt_apply(RBT*, void (*)(void*));

#endif /* _RBT_H_ */
