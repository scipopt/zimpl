#pragma ident "@(#) $Id: ratlpstore.h,v 1.9 2005/09/02 02:01:36 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: ratlpstore.h                                                  */
/*   Name....: Rational Number Store Linear Programm                         */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2003 by Thorsten Koch <koch@zib.de>
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

#ifndef _RATLPSTORE_H_
#define _RATLPSTORE_H_

#ifndef __GMP_H__
#error "Need to include gmp.h before gmpmisc.h"
#endif
#ifndef _BOOL_H_
#error "Need to include bool.h before gmpmisc.h"
#endif

typedef struct storage Sto;

typedef struct lps_hash LpsHash;

struct nonzero
{
   Var*   var;
   Con*   con;
   Nzo*   var_prev;
   Nzo*   var_next;
   Nzo*   con_prev;
   Nzo*   con_next;
   mpq_t  value;
};

struct variable
{
   unsigned int sid;
   char*        name;
   int          number;
   VarClass     class;
   VarType      type;
   VarState     state;
   int          size;
   mpq_t        cost;
   mpq_t        lower;
   mpq_t        upper;
   mpq_t        value;
   mpq_t        startval;
   int          priority;
   Var*         prev;
   Var*         next;   
   Nzo*         first;
};

#define VAR_SID 0x5e564152

struct constraint
{
   unsigned int sid;
   char*        name;
   int          number;
   unsigned int flags;
   ConType      type;
   ConState     state;
   int          size;
   mpq_t        lhs;
   mpq_t        rhs;
   mpq_t        scale;
   Con*         prev;
   Con*         next;
   Nzo*         first;
};

#define CON_SID 0x5e434f4e

struct lpstorage
{
   char*    name;
   LpType   type;   /* ??? Wird noch nicht automatisch gesetzt */
   LpDirect direct;
   char*    probname;
   char*    objname;
   char*    rhsname;
   char*    bndname;
   char*    rngname;
   int      vars;
   int      cons;
   int      soss;
   int      nonzeros;
   Var*     var_root;
   Con*     con_root;
   Sos*     sos_root;
   Sto*     sto_root;
   Nzo*     next;
   LpsHash* var_hash;
   LpsHash* con_hash;
   LpsHash* sos_hash;
   Var*     var_last;
   Con*     con_last;
   Sos*     sos_last;
}; 

struct soselement
{
   Var*  var;
   mpq_t weight;
   Sse*  next;
};

struct soset
{
   unsigned int sid;
   char*        name;
   SosType      type;
   int          priority;
   int          sses;
   Sos*         next;
   Sse*         first;
};

#define SOS_SID 0x5e534f53

#define LPF_NAME_LEN  16
#define MPS_NAME_LEN  8

/* Internal functions
 */
/*lint -sem(        lps_makename, nulterm(3), 1p && 2n > 0 && 3p && 4n >= 0) */
void                lps_makename(char* target, int size, const char* name, int no);

#endif /* _RATLPSTORE_H_ */






