#ident "@(#) $Id: lpstore.h,v 1.3 2002/05/26 12:44:57 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: lpstore.h                                                     */
/*   Name....: Store Linear Programm                                         */
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

#ifndef _LPSTORE_H_
#define _LPSTORE_H_

typedef struct storage Sto;

struct nonzero
{
   Var*   var;
   Con*   con;
   Nzo*   var_prev;
   Nzo*   var_next;
   Nzo*   con_prev;
   Nzo*   con_next;
   double value;
};

struct variable
{
   char*   name;
   int     number;
   int     size;
   VarType type;
   double  cost;
   double  lower;
   double  upper;
   Var*    prev;
   Var*    next;   
   Nzo*    first;
};

struct constraint
{
   char*        name;
   int          number;
   ConType      type;
   unsigned int flags;
   int          size;
   double       rhs;
   Con*         prev;
   Con*         next;
   Nzo*         first;
};

struct lpstorage
{
   char*    name;
   LpDirect direct;
   LpType   type;
   char*    objname;
   int      vars;
   int      cons;
   int      nonzeros;
   Var*     var_root;
   Con*     con_root;
   Sto*     sto_root;
   Nzo*     next;
   RBT*     var_tree;
   RBT*     con_tree;
}; 

#define MPS_NAME_LEN  8

#endif /* _LPSTORE_H_ */
