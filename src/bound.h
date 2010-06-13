/* $Id: bound.h,v 1.2 2010/06/13 12:37:40 bzfkocht Exp $ */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: bound.h                                                       */
/*   Name....: Bound value                                                   */
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
#ifndef _BOUND_H_
#define _BOUND_H_

#ifndef _NUMB_H_
#error "Need to include numb.h before bound.h"
#endif

enum bound_type
{
   BOUND_ERROR = 0, BOUND_VALUE, BOUND_INFTY, BOUND_MINUS_INFTY
};

typedef enum bound_type          BoundType;
typedef struct bound             Bound;

/*lint -sem(        bound_new, @p == 1) */
extern Bound*       bound_new(BoundType type, const Numb* value);
/*lint -sem(        bound_free, custodial(1), 1p == 1) */
extern void         bound_free(Bound* bound);
/*lint -sem(        bound_is_valid, 1p == 1) */
extern Bool         bound_is_valid(const Bound* bound);
/*lint -sem(        bound_copy, 1p == 1, @p == 1) */
extern Bound*       bound_copy(const Bound* source);
/*lint -sem(        bound_get_type, 1p == 1) */
extern BoundType    bound_get_type(const Bound* bound);
/*lint -sem(        bound_get_value, 1p == 1) */
extern const Numb*  bound_get_value(const Bound* bound);
/*lint -sem(        bound_print, 1p == 1 && 2p == 1) */
extern void         bound_print(FILE* fp, const Bound* bound);

#endif /* _BOUND_H_ */

