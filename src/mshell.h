#ident "@(#) $Id: mshell.h,v 1.2 2001/05/06 11:43:21 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mshell.h                                                      */
/*   Name....: Memory Allocation Shell                                       */
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

#ifndef _MSHELL_H_
#define _MSHELL_H_  

#if !defined(NO_MSHELL) && !defined(NDEBUG)

extern size_t mem_used(void);
extern void   mem_maximum(FILE*);
extern void   mem_display(FILE*);
extern void   mem_check(const char*, const int);
extern void*  mem_malloc(size_t, const char*, const int);
extern void*  mem_calloc(size_t, size_t, const char*, const int);
extern void*  mem_realloc(void*, size_t, const char*, const int);
extern char*  mem_strdup(const char*, const char*, const int);
extern void   mem_free(void*, const char*, const int);
extern void   mem_xhide(void*, const char*, const int);

#ifndef __MSHELL__
/*lint -e652*/
#ifdef strdup
#undef strdup
#endif

#define malloc(a)       mem_malloc((a), __FILE__, __LINE__)
#define calloc(a, b)    mem_calloc((a), (b), __FILE__, __LINE__)
#define realloc(a, b)   mem_realloc((a), (b), __FILE__, __LINE__)
#define strdup(a)       mem_strdup((a), __FILE__, __LINE__)
#define free(a)         mem_free((a), __FILE__, __LINE__) 
#define mem_hide(a)     mem_xhide(a, __FILE__, __LINE__)
/*lint +e652*/

#endif /* __MSHELL__ */
#else /* !NO_MSHELL && !NDEBUG */

#define mem_maximum(a)  /**/
#define mem_display(a)  /**/
#define mem_check(a, b) /**/
#define mem_hide(a)     /**/

#endif /* !NO_MSHELL && !NDEBUG */
#endif /* !_MSHELL_H_ */

