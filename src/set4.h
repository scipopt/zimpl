#pragma ident "@(#) $Id: set4.h,v 1.2 2004/04/12 19:17:27 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: set4.h                                                        */
/*   Name....: Set Functions                                                 */
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

#ifndef _SET4_H_
#define _SET4_H_

enum set_type {
   SET_ERROR   = 0,
   SET_EMPTY   = 1, /* dim = 0, Empty Set */
   SET_LIST    = 2, /* dim = 1, Explicit enumeration of members */
   SET_RANGE   = 3, /* dim = 1, Range: Start to End by Increment */
   SET_PROD    = 4, /* dim > 1, Cross Product of two or more sets */
   SET_MULTI   = 5, /* dim > 1, Multidimensional Subset */
   SET_UNION   = 6, /* dim > 1, Union of two sets */
   SET_INTER   = 7, /* dim > 1, Intersection of two sets */
   SET_MINUS   = 8, /* dim > 1, Subtraction of two sets */
   SET_SYMDIFF = 9  /* dim > 1, Symetric difference of two sets */
};

typedef enum set_type      SetType;
//typedef union set          Set;
typedef union set_iter     SetIter;
typedef struct set_vtab    SetVTab;

typedef struct set_head    SetHead;
typedef struct set_list    SetList;
typedef struct set_range   SetRange;
typedef struct set_prod    SetProd;
typedef struct set_multi   SetMulti;

typedef struct set_list_iter    SetListIter;
typedef struct set_range_iter   SetRangeIter;
typedef struct set_prod_iter    SetProdIter;
typedef struct set_multi_iter   SetMultiIter;

struct set_head
{
   int        refc;
   int        dim;
   int        members;
   SetType    type;
};

struct set_list
{
   SetHead head;   /** head.dim == 1 */
   int     size;
   Elem**  member; /** head.members gives the number */
   Hash*   hash;
   SID
};

struct set_range
{
   SetHead head;   /** head.dim == 1 */
   int     begin;
   int     end;
   int     step;
   SID
};

struct set_prod
{
   SetHead head;   /** head.dim > 1 */
   Set*    set_a;
   Set*    set_b;
   SID
};

union set
{
   SetHead    head;
   SetList    list;
   SetRange   range;
   SetProd    prod;
   //   SetMulti   multi;
};

struct set_list_iter
{
   int first;
   int last;
   int now;
   SID
};

struct set_range_iter
{
   int first;
   int last;
   int now;
   SID
};

struct set_prod_iter
{
   SetType  type_a;
   SetType  type_b;
   SetIter* iter_a;
   SetIter* iter_b;
   SID
};

union set_iter
{
   SetListIter    list;
   SetRangeIter   range;
   SetProdIter    prod;
   //   SetMultiIter   multi;
};

struct set_vtab
{
   void (*set_free)    (Set* set);
   Set* (*set_copy)    (const Set* set);
   Bool (*set_is_valid)(const Set* set);
   int  (*set_lookup_idx)(const Set* set, const Tuple* tuple, int offset);
   void (*iter_init) (SetIter** iter, const Set* set, const Tuple* pattern, int offset);
   Bool (*iter_next) (SetIter* iter, const Set* set, Tuple* tuple, int offset);
   void (*iter_exit) (SetIter* iter);
   void (*iter_reset)(SetIter* iter);
};


extern SetVTab* set_vtab_global;

#ifndef NDEBUG
extern SetVTab* set_get_vtab(void);
#else
#define set_get_vtab()  set_vtab_global
#endif

extern void setlist_init(SetVTab* vtab);
extern void setrange_init(SetVTab* vtab);
extern void setprod_init(SetVTab* vtab);
extern void setmulti_init(SetVTab* vtab);

extern Set set_new_from_list(const List* list);


/* setlist.c
 */
extern Set* set_list_new(int size, int flags);
extern int  set_list_add_elem(Set* set, const Elem* elem, SetCheckType check);
extern Set* set_list_new_from_elems(const List* list);
extern Set* set_list_new_from_tuples(const List* list);

extern const Elem* set_list_get_elem(const Set* set, int idx);

/* setrange.c
 */
extern Set* set_range_new(int begin, int end, int step);

/* set multi.c
 */
#if 0
extern Set set_multi_new_from_tuples(const List* list);

extern Set set_range_new(int begin, int end, int step);

extern Set set_prod_new(Set a, Set b);
#endif

#endif /* _SET4_H_ */







