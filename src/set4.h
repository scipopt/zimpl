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

union set
{
   SetHead*    head;
   SetList*    list;
   SetRange*   range;
   SetProd*    prod;
   SetMulti*   multi;
};

union set_iter
{
   SetListIter*    list;
   SetRangeIter*   range;
   SetProdIter*    prod;
   SetMultiIter*   multi;
};

struct set_vtab
{
   void (*set_free)  (Set set);
   int  (*set_lookup)(const Set set, const Tuple* tuple, int offset);
   void (*iter_init) (SetIter* iter, const Set set, const Tuple* pattern, int offset);
   Bool (*iter_next) (SetIter iter, const Set set, Tuple* tuple, int offset);
   void (*iter_exit) (SetIter iter);
   void (*iter_reset)(SetIter iter);
};


extern SetVTab* set_vtab_global;

#ifndef NDEBUG
extern SetVTab* set_get_vtab(void);
#if 0
{
   return set_vtab_global;
}
#endif
#else
#define set_get_vtab()  set_vtab_global
#endif

extern void setlist_init(SetVTab* vtab);
extern void setrange_init(SetVTab* vtab);
extern void setprod_init(SetVTab* vtab);
extern void setmulti_init(SetVTab* vtab);

extern const Elem* set_list_get_elem(SetList* sl, int idx);

