/* $Id: heap.h,v 1.2 2010/06/13 12:37:40 bzfkocht Exp $ */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: heap.h                                                        */
/*   Name....: Heap Functions                                                */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2006-2010 by Thorsten Koch <koch@zib.de>
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
#ifndef _HEAP_H_
#define _HEAP_H_

#ifndef _BOOL_H_
#error "Need to include bool.h before heaph.h"
#endif
#ifndef _ENTRY_H_
#error "Need to include entry.h before heap.h"
#endif

union heap_data
{
   Entry* entry;
};

typedef struct heap              Heap;
typedef union heap_data          HeapData;
typedef int                    (*HeapCmp)(HeapData, HeapData);

/*lint -sem(        heap_new_entry, 1n > 0 && 2p == 1, @p == 1) */
extern Heap*        heap_new_entry(int size, HeapCmp entry_cmp);
/*lint -sem(        heap_free, 1p == 1) */
extern void         heap_free(Heap* heap);
/*lint -sem(        heap_is_valid, 1p == 1) */
extern Bool         heap_is_valid(const Heap* heap);
/*lint -sem(        heap_push_entry, 1p == 1 && 2p == 1) */
extern void         heap_push_entry(Heap* heap, Entry* entry);
/*lint -sem(        heap_pop_entry, 1p == 1, @p == 1) */
extern Entry*       heap_pop_entry(Heap* heap);
/*lint -sem(        heap_top_entry, 1p == 1, @p == 1) */
extern const Entry* heap_top_entry(const Heap* heap);
/*lint -sem(        heap_is_full, 1p == 1) */
extern Bool         heap_is_full(const Heap* heap);
/*lint -sem(        heap_is_empty, 1p == 1) */
extern Bool         heap_is_empty(const Heap* heap);

#endif /* _HEAP_H_ */
