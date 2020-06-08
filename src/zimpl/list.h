/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: list.h                                                        */
/*   Name....: List Functions                                                */
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
#ifndef _LIST_H_
#define _LIST_H_

#ifndef _MME_H_
#error "Need to include mme.h before list.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LIST_NULL ((List*)0)

//lint -sem(        list_new_elem, 1p == 1, @P >= malloc(1)) 
extern List*        list_new_elem(const Elem* elem) expects_NONNULL returns_NONNULL;
//lint -sem(        list_new_tuple, 1p == 1, @P >= malloc(1)) 
extern List*        list_new_tuple(const Tuple* tuple) expects_NONNULL returns_NONNULL;
//lint -sem(        list_new_entry, 1p == 1, @P >= malloc(1)) 
extern List*        list_new_entry(const Entry* entry) expects_NONNULL returns_NONNULL;
//lint -sem(        list_new_list, 1p == 1, @P >= malloc(1)) 
extern List*        list_new_list(const List* list) expects_NONNULL returns_NONNULL;
//lint -sem(        list_free, custodial(1), inout(1), 1p == 1) 
extern void         list_free(List* list) expects_NONNULL;
//lint -sem(        list_is_valid, pure, 1p == 1) 
extern bool         list_is_valid(const List* list) expects_NONNULL is_PURE;
//lint -sem(        list_is_elemlist, pure, 1p == 1) 
extern bool         list_is_elemlist(const List* list) expects_NONNULL is_PURE;
//lint -sem(        list_is_entrylist, pure, 1p == 1) 
extern bool         list_is_entrylist(const List* list) expects_NONNULL is_PURE;
//lint -sem(        list_is_tuplelist, pure, 1p == 1) 
extern bool         list_is_tuplelist(const List* list) expects_NONNULL is_PURE;
//lint -sem(        list_copy, 1p == 1, @P >= malloc(1)) 
extern List*        list_copy(const List* list) expects_NONNULL returns_NONNULL;
//lint -sem(        list_add_list, inout(1), 1p == 1, 2p == 1) 
extern void         list_add_list(List* list, const List* ll) expects_NONNULL;
//lint -sem(        list_add_elem, inout(1), 1p == 1, 2p == 1) 
extern void         list_add_elem(List* list, const Elem* elem) expects_NONNULL;
//lint -sem(        list_add_tuple, inout(1), 1p == 1, 2p == 1) 
extern void         list_add_tuple(List* list, const Tuple* tuple) expects_NONNULL;
//lint -sem(        list_add_entry, inout(1), 1p == 1, 2p == 1) 
extern void         list_add_entry(List* list, const Entry* entry) expects_NONNULL;
//lint -sem(        list_insert_elem, inout(1), 1p == 1, 2p == 1) 
extern void         list_insert_elem(List* list, const Elem* elem) expects_NONNULL;
//lint -sem(        list_insert_tuple, inout(1), 1p == 1, 2p == 1) 
extern void         list_insert_tuple(List* list, const Tuple* tuple) expects_NONNULL;
//lint -sem(        list_insert_entry, inout(1), 1p == 1, 2p == 1) 
extern void         list_insert_entry(List* list, const Entry* entry) expects_NONNULL;
//lint -sem(        list_get_elems, pure, 1p == 1, chneg(@)) 
extern int          list_get_elems(const List* list) expects_NONNULL is_PURE;
//lint -sem(        list_get_elem, 1p == 1, 2p, r_null) 
extern const Elem*  list_get_elem(const List* list, ListElem** idxp) expects_NONNULL;
//lint -sem(        list_get_tuple, 1p == 1, 2p, r_null) 
extern const Tuple* list_get_tuple(const List* list, ListElem** idxp) expects_NONNULL;
//lint -sem(        list_get_entry, 1p == 1, 2p r_null) 
extern const Entry* list_get_entry(const List* list, ListElem** idxp) expects_NONNULL;
//lint -sem(        list_get_list, 1p == 1, 2p, r_null) 
extern const List*  list_get_list(const List* list, ListElem** idxp) expects_NONNULL;
//lint -sem(        list_print, inout(1), 1p == 1, 2p == 1) 
extern void         list_print(FILE* fp, const List* list) expects_NONNULL;

#ifdef __cplusplus
}
#endif
#endif // _LIST_H_ 
