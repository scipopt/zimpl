/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: hash.h                                                        */
/*   Name....: Hash Functions                                                */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2018 by Thorsten Koch <koch@zib.de>
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
#ifndef _HASH_H_
#define _HASH_H_

#ifndef _NUMB_H_
#error "Need to include numb.h before hash.h"
#endif
#ifndef _ELEM_H_
#error "Need to include elem.h before hash.h"
#endif
#ifndef _TUPLE_H_
#error "Need to include tuple.h before hash.h"
#endif
#ifndef _MME_H_
#error "Need to include mme.h before hash.h (Entry,Mono)"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum hash_type       { HASH_ERR = 0, HASH_TUPLE, HASH_ENTRY, HASH_ELEM_IDX, HASH_NUMB, HASH_MONO };

typedef enum hash_type           HashType;
typedef struct hash              Hash;

/*lint -sem(        hash_new, chneg(2), @P >= malloc(1)) */
extern Hash*        hash_new(HashType type, int size);
/*lint -sem(        hash_free, custodial(1), cleanup, inout(1), 1P) */
extern void         hash_free(Hash* hash);
/*lint -sem(        hash_add_tuple, inout(1), 1P >= 1, 2P >= 1) */
extern void         hash_add_tuple(Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_add_entry, inout(1), 1P >= 1, 2P >= 1) */
extern void         hash_add_entry(Hash* hash, const Entry* entry);
/*lint -sem(        hash_add_mono, inout(1), 1P >= 1, 2P >= 1) */
extern void         hash_add_mono(Hash* hash, const Mono* mono);
/*lint -sem(        hash_add_elem_idx, inout(1), 1P >= 1, 2p, chneg(3)) */
extern void         hash_add_elem_idx(Hash* hash, const Elem* elem, int idx);
/*lint -sem(        hash_add_numb, inout(1), 1P >= 1, 2P >= 1) */
extern void         hash_add_numb(Hash* hash, const Numb* numb);
/*lint -sem(        hash_has_tuple, pure, 1P >= 1, 2P >= 1) */
extern bool         hash_has_tuple(const Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_has_entry, pure, 1P >= 1, 2P >= 1) */
extern bool         hash_has_entry(const Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_has_numb, pure, 1P >= 1, 2P >= 1) */
extern bool         hash_has_numb(const Hash* hash, const Numb* numb);
/*lint -sem(        hash_lookup_entry, pure, 1P >= 1, 2P >= 1) */
extern const Entry* hash_lookup_entry(const Hash* hash, const Tuple* tuple);
/*lint -sem(        hash_lookup_mono, pure, 1P >= 1, 2P >= 1) */
extern const Mono*  hash_lookup_mono(const Hash* hash, const Mono* mono);
/*lint -sem(        hash_lookup_elem_idx, pure, 1P >= 1, 2P >= 1) */
extern int          hash_lookup_elem_idx(const Hash* hash, const Elem* elem);

#ifdef __cplusplus
}
#endif
#endif /* _HASH_H_ */
