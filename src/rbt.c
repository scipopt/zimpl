#ident "@(#) $Id: rbt.c,v 1.4 2002/08/18 14:58:01 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   Datei...: rbt.c                                                         */
/*   Name....: Red-Black-Tree Routines                                       */
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
 *
 *
 *       A description of the following algorithm can be found in
 *
 *                   Robert Sedgewick: ALGORITHMS
 *
 *                second edition, 1989, Addison-Wesley
 *
 *                    Chapter 15: Balanced Trees
 *
 *
 * Red-Black Trees can be defined as follows:
 *
 * 1. Every Node is either red or black.
 * 2. The root is black.
 * 3. The leafes are black.
 * 4. Every red node has a black parent.
 * 5. Every direct path from a node to a leaf must contain the same
 *    number of black nodes.
 *
 * This implementation employes two dummy nodes. One start node, which is
 * the parent of the root (the root can change) and an end node, which is
 * the universal leaf.
 * It is assumed that the start nodes has the lowest possible key value
 * and the end node hat the highest possible key value.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mshell.h"
#include "portab.h"

#define BLACK_NODE  (Bool)0
#define RED_NODE    (Bool)1

struct rbt_store                 /* node block storage                  */
{
   unsigned int      size;       /* Number of nodes in a block          */
   struct rbt_node*  begin;      /* Pointer to start of block           */
   struct rbt_store* next;       /* Pointer to next storage             */
};

struct rbt_node                  /* tree node                           */
{
   void*            key;         /* Pointer to keyvalue                 */
   struct rbt_node* left;        /* Left child (left->key <= key)       */
   struct rbt_node* right;       /* right child (right->key >= key)     */
   unsigned int     size  : 31;  /* size of keyvalue (may be 0)         */
   unsigned int     color : 1;   /* color of node (BLACK or RED)        */
};

#define MAX_SIZE     0x7fffffff  /* = 2**31 - 1                         */
                                 /* (beware of signed vs. unsigned.)    */
                                 
struct rbt_header                /* rbt header structure                */
{
   unsigned int      count;      /* number of real nodes                */
   unsigned int      store_size; /* number of nodes in a storage block  */
   struct rbt_node*  next;       /* the next free node (can be NULL)    */
   struct rbt_node   start;      /* the first node                      */
   struct rbt_node   stop;       /* THE leaf node for all leafes        */
   struct rbt_store* store;      /* Pointer to storage (linked list)    */ 
   int             (*cmp)(const void*, const void*); 
};

#include "rbt.h"

#if TRACE                         /* Routines only used for tracing purposes */
/*---------------------------------------------------------------------------*/
/*--- Name     : Printout RBT                                             ---*/
/*--- Function : Traverses the RBT and prints the node information.       ---*/
/*--- Parameter: Pointer to RBT, node, depth.                             ---*/
/*--- Returns  : Nothing.                                                 ---*/
/*---------------------------------------------------------------------------*/
static void rbt_printout(
   const RBT*             rbt,
   const struct rbt_node* x,
   int                    depth)
{
   assert(rbt   != NULL);
   assert(x     != NULL);
   assert(depth >= 0);
   
   if (x != &rbt->stop)
   {
      fprintf(stderr, "Depth: %3d, %s, [%5d], size=%d, left=[%5d], right=[%5d]\n",
         depth,
         x->color == BLACK_NODE ? "Black" : "Red  ",
         *(int*)x->key,
         x->size,
         x->left  == &rbt->stop ? -2 : *(int*)x->left->key,
         x->right == &rbt->stop ? -2 : *(int*)x->right->key);
         
      rbt_printout(rbt, x->left, depth + 1);
      rbt_printout(rbt, x->right, depth + 1);
   }
}   

/*---------------------------------------------------------------------------*/
/*--- Name     : Print RBT                                                ---*/
/*--- Function : Prints the contents of the RBT including internal info.  ---*/
/*--- Parameter: Pointer to RBT.                                          ---*/
/*--- Returns  : Nothing.                                                 ---*/
/*---------------------------------------------------------------------------*/
static void rbt_print(
   const RBT* rbt)
{
   assert(rbt != NULL);

   fprintf(stderr, "----------------- RBT ---------------------\n");
   rbt_printout(rbt, &rbt->start, 0);
   fprintf(stderr, "-------------------------------------------\n\n");
}   
#endif /* TRACE */

#ifndef NDEBUG                /* Routines only availabe in debugging version */
/*---------------------------------------------------------------------------*/
/*--- Name     : Checkout RBT                                             ---*/
/*--- Function : Traverses the RBT and checks consistence.                ---*/
/*--- Parameter: Pointer to RBT, node, parent, pointer to count nodes.    ---*/
/*--- Returns  : True if ok, False otherwise.                             ---*/
/*---------------------------------------------------------------------------*/
static Bool rbt_checkout(
   const RBT*    rbt,
   const struct  rbt_node* xp,
   const struct  rbt_node* x,
   unsigned int* count,
   unsigned int* blacks)
{
   unsigned int blacks_left  = 0;
   unsigned int blacks_right = 0;
   
   assert(rbt    != NULL);
   assert(xp     != NULL);
   assert(x      != NULL);
   assert(count  != NULL);
   assert(blacks != NULL);
   
   if (x != &rbt->stop)
   {
      (*count)++;

      if ((x->color == RED_NODE) && (xp->color == RED_NODE))
         return FALSE;
         
      if (!rbt_checkout(rbt, x, x->left, count, &blacks_left))
         return FALSE;
         
      if (!rbt_checkout(rbt, x, x->right, count, &blacks_right))
         return FALSE;

      if (blacks_left != blacks_right)
         return FALSE;

      *blacks += blacks_left;
   
      if (x->color == BLACK_NODE)
         (*blacks)++;
   }
   return TRUE;
}   

/*---------------------------------------------------------------------------*/
/*--- Name     : Is RBT Valid ?                                           ---*/
/*--- Function : Checks the RBT structure for consistence / correctness.  ---*/
/*--- Parameter: Pointer to RBT.                                          ---*/
/*--- Returns  : True if ok, False otherwise.                             ---*/
/*---------------------------------------------------------------------------*/
/* Routine is exported, so validation routines for data structures using 
 * RBT's can explicitly check if the RBT is ok.
 * Be aware that this routine is not available if NDEBUG is defined !
 */
static Bool rbt_valid(
   const RBT* rbt)
{
   unsigned int count  = 0;
   unsigned int blacks = 0;
   
   mem_check(__FILE__, __LINE__);

   if (rbt == NULL)
      fprintf(stderr, "rbt_valid: rbt == NULL\n");

   else if (rbt->start.color != BLACK_NODE)
      fprintf(stderr, "rbt_valid: rbt->start.color != BLACK_NODE\n");

   else if (rbt->start.right->color != BLACK_NODE)
      fprintf(stderr, "rbt_valid: rbt->start.right->color != BLACK_NODE\n");

   else if (rbt->start.left != &rbt->stop)
      fprintf(stderr, "rbt_valid: rbt->start.left != &rbt->stop\n");

   else if ((rbt->stop.left != &rbt->stop) || (rbt->stop.right != &rbt->stop))
      fprintf(stderr, "rbt_valid: rbt->stop.left/right != &rbt->stop\n");

   else if (rbt->stop.color != BLACK_NODE)
      fprintf(stderr, "rbt_valid: rbt->stop.color != BLACK_NODE\n");

   else if (rbt->cmp == NULL)
      fprintf(stderr, "rbt_valid: rbt->cmp == NULL\n");

   else if (!rbt_checkout(rbt, &rbt->start, rbt->start.right, &count, &blacks))
      fprintf(stderr, "rbt_valid: checkout failed\n");

   else if (count != rbt->count)
      fprintf(stderr,
         "rbt_valid: rbt->count=%d, count=%d\n", rbt->count, count);
   else   
      return TRUE;
   
   return FALSE;
}
#endif /* !NDEBUG */

/*---------------------------------------------------------------------------*/
/*--- Name     : Allocate Block Storage                                   ---*/
/*--- Function : Allocates memory to hold rbt->store_size nodes.          ---*/
/*--- Parameter: Pointer to RBT.                                          ---*/
/*--- Returns  : Nothing.                                                 ---*/
/*---------------------------------------------------------------------------*/
static void rbt_storage(
   RBT* rbt)
{
   struct rbt_store* store;
   struct rbt_node*  nodes;
   unsigned int      i;
         
   assert(rbt       != NULL);
   assert(rbt->next == NULL); 

   store = malloc(sizeof(*store));

   assert(store != NULL);

   nodes = malloc(rbt->store_size * sizeof(*nodes));
   
   assert(nodes != NULL);
   
   for(i = 0; i < rbt->store_size; i++)
   {
      /* Attention; &nodes[rbt->storage_size] is not a valid address !
       * We correct this after the loop.
       */
      nodes[i].left  = &nodes[i + 1];
      nodes[i].key   = NULL;
      nodes[i].right = NULL;
      nodes[i].size  = 0;
      nodes[i].color = BLACK_NODE;
   }
   /* Correction for the last element.
    */
   nodes[i - 1].left  = NULL;

   store->size  = rbt->store_size;
   store->begin = &nodes[0];
   store->next  = rbt->store;
   rbt->store   = store;
   rbt->next    = store->begin;
}
      
/*---------------------------------------------------------------------------*/
/*--- Name     : Allocate RBT Node.                                       ---*/
/*--- Function : Provides a new node.                                     ---*/
/*--- Parameter: Pointer to RBT and to new keyvalue and it's size.        ---*/
/*---            (if size == 0 then only the pointer self is stored.)     ---*/
/*--- Returns  : Pointer to new node.                                     ---*/
/*---------------------------------------------------------------------------*/
static struct rbt_node* rbt_alloc(
   RBT*         rbt,
   void*        key,
   unsigned int size)
{
   struct rbt_node* x;
   void*            t;
   
   assert(rbt  != NULL);
   assert(key  != NULL);
   assert(size <  MAX_SIZE);

   if (rbt->next == NULL)
      rbt_storage(rbt);

   /* Get a free node
    */
   x = rbt->next;

   assert(x != NULL);

   rbt->next = x->left;
   x->size   = size;
   x->color  = BLACK_NODE;
   x->left   = &rbt->stop;
   x->right  = &rbt->stop;

   if (size == 0)
      x->key = key;
   else
   {
      /* Copy and store the keyvalue
       */
      t = malloc(size);

      assert(t != NULL);

      x->key = memcpy(t, key, size);
   }
   rbt->count++;
   
   return x;
}   

/*---------------------------------------------------------------------------*/
/*--- Name     : Initialise RBT.                                          ---*/
/*--- Function : Initialises a new RBT.                                   ---*/
/*--- Parameter: Number of nodes to allocate storage for at a time,       ---*/
/*---            Comparisonfunction.                                      ---*/
/*--- Returns  : Pointer to RBT.                                          ---*/
/*---------------------------------------------------------------------------*/
RBT* rbt_init(
   unsigned int store_size,
   int        (*cmp)(const void*, const void*))
{
   RBT* rbt;

   rbt = malloc(sizeof(*rbt));

   assert(rbt != NULL);

   rbt->count       = 0;
   rbt->stop.key    = NULL;
   rbt->stop.left   = &rbt->stop;
   rbt->stop.right  = &rbt->stop;
   rbt->stop.size   = 0;
   rbt->stop.color  = BLACK_NODE;
   rbt->start.key   = NULL;
   rbt->start.left  = &rbt->stop;
   rbt->start.right = &rbt->stop;
   rbt->start.size  = 0;
   rbt->start.color = BLACK_NODE;
   rbt->store_size  = store_size;
   rbt->cmp         = cmp;
   rbt->next        = NULL;
   rbt->store       = NULL;
   
   assert(rbt_valid(rbt));

   return rbt;
}   

/*---------------------------------------------------------------------------*/
/*--- Name     : Exit RBT.                                                ---*/
/*--- Function : Destroys a RBT and frees all kept memory.                ---*/
/*--- Parameter: Pointer to RBT.                                          ---*/
/*--- Returns  : Nothing.                                                 ---*/
/*---------------------------------------------------------------------------*/
void rbt_exit(
   RBT* rbt)
{
   struct rbt_store* store;
   struct rbt_store* next;
   unsigned int      i;
   
   assert(rbt_valid(rbt));
   
   for(store = rbt->store; store != NULL; store = next)
   {
      next = store->next;

      /* Free copied keyvalues.
       */
      for(i = 0; i < store->size; i++)
         if ((store->begin[i].size > 0) && (store->begin[i].key != NULL))
            free((void*)store->begin[i].key);

      free(store->begin);
      free(store);
   }
   free(rbt);
}

/*---------------------------------------------------------------------------*/
/*--- Name     : Rotate two Nodes.                                        ---*/
/*--- Function : Switches (rotates) two nodes in the tree.                ---*/
/*--- Parameter: Pointer to RBT, Keyvalue and parent of the two nodes.    ---*/
/*--- Returns  : Pointer to the deepest of the two nodes after the switch.---*/
/*---------------------------------------------------------------------------*/
static struct rbt_node* rbt_rotate(
   const RBT*       rbt,
   const void*      key,
   struct rbt_node* y)
{
   struct rbt_node* yc;
   struct rbt_node* ycc;
   int              key_ykey;
   
   assert(rbt != NULL);
   assert(key != NULL);
   assert(y   != NULL);
   
   key_ykey = ((y != &rbt->start) && ((*rbt->cmp)(key, y->key) < 0));
   yc       = (key_ykey) ? y->left : y->right;

   if ((yc == &rbt->stop) || (*rbt->cmp)(key, yc->key) < 0)
   {
      ycc        = yc->left;
      yc->left   = ycc->right;
      ycc->right = yc;
   }
   else
   {
      ycc        = yc->right;
      yc->right  = ycc->left;
      ycc->left  = yc; 
   }
   if (key_ykey)
      y->left  = ycc;
   else
      y->right = ycc;

   return ycc;
}   

/*---------------------------------------------------------------------------*/
/*--- Name     : Split a Node.                                            ---*/
/*--- Function : Splits a virtual 4-node (see article).                   ---*/
/*--- Parameter: Pointer to RBT, keyvalue and a big family.               ---*/
/*--- Returns  : Pointer to the deepest (youngest) node.                  ---*/
/*---------------------------------------------------------------------------*/
static struct rbt_node* rbt_split(
   const RBT*       rbt,
   const void*      key,
   struct rbt_node* xppp,
   struct rbt_node* xpp,
   const struct rbt_node* xp,
   struct rbt_node* x)
{
   Bool key_xppkey;
   Bool key_xpkey;
   
   assert(rbt      != NULL);
   assert(key      != NULL);
   assert(xppp     != NULL);
   assert(xpp      != NULL);
   assert(xp       != NULL);
   assert(x        != NULL);
   assert(x->color == BLACK_NODE);

   /* Easy case: we have a 2- or a suitable oriented 3-node.
    */   
   x->color        = RED_NODE;
   x->left->color  = BLACK_NODE;
   x->right->color = BLACK_NODE;

   /* Everything ok ?
    * Only one red node in sequence is allowed.
     */
   if (xp->color == RED_NODE)
   {
      assert((xpp->color == BLACK_NODE) || (xpp == xp));
      
      xpp->color = RED_NODE;

      /* Since the following are boolean expressions, conditional could be
       * ommitted, but since in C a boolean true is any non zero value and
       * the two values are to be compared, it seems safer to assign a well
       * defined value. An optimizing compiler should be able to generate
       * the same code anyway.
       */
      key_xppkey = ((xpp != &rbt->start) && ((*rbt->cmp)(key, xpp->key) < 0)) ? TRUE : FALSE;
      key_xpkey  = ((xp  != &rbt->start) && ((*rbt->cmp)(key, xp->key)  < 0)) ? TRUE : FALSE;

      if (key_xppkey != key_xpkey)
      {
         (void)rbt_rotate(rbt, key, xpp);
      }
      x = rbt_rotate(rbt, key, xppp);

      x->color = BLACK_NODE;
   }
   /* The root is ever black. If we got it red, correct this.
    * This way the root stays up !
    */
   rbt->start.right->color = BLACK_NODE;

   return x;
}

/*---------------------------------------------------------------------------*/
/*--- Name     : Insert a Node.                                           ---*/
/*--- Function : Adds a new node to the tree.                             ---*/
/*--- Parameter: Pointer to RBT, new keyvalue and it's size.              ---*/
/*---            (if size == 0 then only the keyvalue pointer is stored.) ---*/
/*--- Returns  : Nothing.                                                 ---*/
/*---------------------------------------------------------------------------*/
void rbt_insert(
   RBT*         rbt,
   void*        key,
   unsigned int size)
{
   struct rbt_node* x;
   struct rbt_node* xp;
   struct rbt_node* xpp;
   struct rbt_node* xppp;

   assert(rbt_valid(rbt));
   assert(key != NULL);
   
   x   = &rbt->start;
   xp  = &rbt->start;
   xpp = &rbt->start;
   
   /* All the way down...
    */
   do
   {
      /* Remeber our way down and go on left or right.
       */
      xppp = xpp;
      xpp  = xp;
      xp   = x;
      x    = ((x != &rbt->start) && ((*rbt->cmp)(key, x->key)) < 0) ? x->left : x->right;

      /* Got a 4-node ? Split it !
       */
      if ((x->left->color == RED_NODE) && (x->right->color == RED_NODE))
         x = rbt_split(rbt, key, xppp, xpp, xp, x);

      assert(x != NULL);
   }
   while(x != &rbt->stop);

   /* Ok, now we insert a new node.
    */
   x = rbt_alloc(rbt, key, size);

   assert(x != NULL);

   /* Did we sit to the left or the right side of the father ? 
    */
   if ((xp != &rbt->start) && ((*rbt->cmp)(key, xp->key) < 0))
      xp->left = x;
   else
      xp->right = x;

   /* The start node has the lowest possible value, so only the right
    * branch can be taken from there.
    */
   assert((xp != &rbt->start) || (x == xp->right));

   /* We look at a new node as the middle of a (virtual) 4-node.
    * Then we split this node. This operation pushes the keyvalue
    * in the middle up.
    */
   (void)rbt_split(rbt, key, xppp, xpp, xp, x);

   assert(rbt_valid(rbt));
}   

/*---------------------------------------------------------------------------*/
/*--- Name     : Search a Key.                                            ---*/
/*--- Function : Searches a keyvalue in the tree.                         ---*/
/*--- Parameter: Pointer to RBT and keyvalue.                             ---*/
/*--- Returns  : Pointer to Keyvalue or NULL.                             ---*/
/*---------------------------------------------------------------------------*/
void* rbt_search(
   const RBT*  rbt,
   const void* key)
{
   struct rbt_node* x;
   
   assert(rbt_valid(rbt));
   assert(key != NULL);

   x = rbt->start.right;

   while((x != &rbt->stop) && ((*rbt->cmp)(key, x->key) != 0))
      x = ((*rbt->cmp)(key, x->key) < 0) ? x->left : x->right;

   return (x != &rbt->stop) ? x->key : NULL;
}   
 
/*---------------------------------------------------------------------------*/
/*--- Name     : Traverses Tree.                                          ---*/
/*--- Function : Traverse the tree in key order and applys a function.    ---*/
/*--- Parameter: Pointer to RBT, Start node, function to apply            ---*/
/*--- Returns  : Nothing.                                                 ---*/
/*---------------------------------------------------------------------------*/
static void rbt_traverse(
   RBT*                   rbt,
   const struct rbt_node* x,
   void                 (*apply)(void*))
{
   assert(rbt_valid(rbt));
   assert(x     != NULL);
   assert(apply != NULL);
   
   if (x != &rbt->stop)
   {
      rbt_traverse(rbt, x->left, apply);
      
      (*apply)(x->key);

      rbt_traverse(rbt, x->right, apply);
   }
}   

/*---------------------------------------------------------------------------*/
/*--- Name     : Apply to data.                                           ---*/
/*--- Function : Traverse the tree in key order and applys a function.    ---*/
/*--- Parameter: Pointer to RBT, function to apply                        ---*/
/*--- Returns  : Nothing.                                                 ---*/
/*---------------------------------------------------------------------------*/
void rbt_apply(
   RBT*   rbt,
   void (*apply)(void*))
{
   assert(rbt_valid(rbt));
   assert(apply != NULL);

   rbt_traverse(rbt, rbt->start.right, apply);
}

/* --------------------------- E O F ----------------------------------------*/
