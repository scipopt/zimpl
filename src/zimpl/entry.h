/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: entry.h                                                       */
/*   Name....: Symbol Table Entry Functions                                  */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2020 by Thorsten Koch <koch@zib.de>
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
#ifndef _ENTRY_H_
#define _ENTRY_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ENTRY_NULL ((Entry*)0)

//lint -sem(        entry_new_numb, 1p == 1, 2p == 1, @P >= malloc(1)) 
extern Entry*       entry_new_numb(const Tuple* tuple, const Numb* numb) expects_NONNULL returns_NONNULL;
//lint -sem(        entry_new_strg, 1p == 1, 2p, @P >= malloc(1)) 
extern Entry*       entry_new_strg(const Tuple* tuple, const char* strg) expects_NONNULL returns_NONNULL;
//lint -sem(        entry_new_set, 1p == 1, 2p == 1, @P >= malloc(1)) 
extern Entry*       entry_new_set(const Tuple* tuple, const Set* set) expects_NONNULL returns_NONNULL;
//lint -sem(        entry_new_var, 1p == 1, 2p == 1, @P >= malloc(1)) 
extern Entry*       entry_new_var(const Tuple* tuple, Var* var) expects_NONNULL returns_NONNULL;
//lint -sem(        entry_free, custodial(1), inout(1), 1p == 1) 
extern void         entry_free(Entry* entry) expects_NONNULL;
//lint -sem(        entry_is_valid, pure, 1p == 1) 
extern bool         entry_is_valid(const Entry* entry) expects_NONNULL is_PURE;
//lint -sem(        entry_copy, 1p == 1, @P >= malloc(1)) 
extern Entry*       entry_copy(const Entry* entry) expects_NONNULL returns_NONNULL;
//lint -sem(        entry_cmp, 1p == 1, 2p == 1) 
extern bool         entry_cmp(const Entry* entry, const Tuple* tuple) expects_NONNULL;
//lint -sem(        entry_get_type, pure, 1p == 1) 
extern SymbolType   entry_get_type(const Entry* entry) expects_NONNULL is_PURE;
//lint -sem(        entry_get_tuple, pure, 1p == 1, @p) 
extern const Tuple* entry_get_tuple(const Entry* entry) expects_NONNULL returns_NONNULL is_PURE;
//lint -sem(        entry_get_numb, pure, 1p == 1, @p) 
extern const Numb*  entry_get_numb(const Entry* entry) expects_NONNULL returns_NONNULL is_PURE;
//lint -sem(        entry_get_strg, pure, 1p == 1, @p) 
extern const char*  entry_get_strg(const Entry* entry) expects_NONNULL returns_NONNULL is_PURE;
//lint -sem(        entry_get_set, pure, 1p == 1, @p) 
extern const Set*   entry_get_set(const Entry* entry) expects_NONNULL returns_NONNULL is_PURE;
//lint -sem(        entry_get_var, pure, 1p == 1, @p) 
extern Var*         entry_get_var(const Entry* entry) expects_NONNULL is_PURE;
//lint -sem(        entry_print, 1p == 1, 2p == 1) 
extern void         entry_print(FILE* fp, const Entry* entry);

#ifdef __cplusplus
}
#endif
#endif // _ENTRY_H_ 

