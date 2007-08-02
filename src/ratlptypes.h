#pragma ident "@(#) $Id: ratlptypes.h,v 1.7 2007/08/02 08:36:56 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: ratlptypes.h                                                  */
/*   Name....: Rational Number LP Storage Library                            */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2003-2007 by Thorsten Koch <koch@zib.de>
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
#ifndef _RATLPTYPES_H_
#define _RATLPTYPES_H_

#ifndef _BOOL_H_
#error "Need to include bool.h before ratlp.h"
#endif

enum con_type        { CON_FREE    = 0, CON_LHS,   CON_RHS,   CON_RANGE, CON_EQUAL };
enum var_type        { VAR_FREE    = 0, VAR_LOWER, VAR_UPPER, VAR_BOXED, VAR_FIXED };
enum sos_type        { SOS_ERR     = 0, SOS_TYPE1, SOS_TYPE2 };
enum con_state       { CON_ERR     = 0, CON_BASIC, CON_ON_LO, CON_ON_UP }; 
enum var_state       { VAR_ERR     = 0, VAR_BASIC, VAR_ON_LO, VAR_ON_UP };
enum var_class       { VAR_CON     = 0, VAR_IMP,   VAR_INT };
enum lp_direct       { LP_MIN      = 0, LP_MAX };
enum lp_type         { LP_ERR      = 0, LP_LP, LP_IP };
enum lp_format       { LP_FORM_ERR = 0, LP_FORM_LPF, LP_FORM_HUM, LP_FORM_MPS, LP_FORM_RLP };
enum presolve_result
{
   PRESOLVE_ERROR = 0, PRESOLVE_OKAY, PRESOLVE_INFEASIBLE,
   PRESOLVE_UNBOUNDED, PRESOLVE_VANISHED
};

typedef struct nonzero       Nzo;
typedef struct variable      Var;
typedef struct constraint    Con;
typedef struct soset         Sos;
typedef struct soselement    Sse;
typedef struct lpstorage     Lps;

typedef enum   var_type      VarType;
typedef enum   con_type      ConType;
typedef enum   sos_type      SosType;
typedef enum   var_state     VarState;
typedef enum   con_state     ConState;
typedef enum   var_class     VarClass;
typedef enum   lp_direct     LpDirect;
typedef enum   lp_type       LpType;
typedef enum   lp_format     LpFormat;

typedef enum presolve_result PSResult;

#define LP_FLAG_CON_SCALE    0x1
#define LP_FLAG_CON_SEPAR    0x2

#define HAS_LOWER(var)  ((var)->type != VAR_FREE && (var)->type != VAR_UPPER)
#define HAS_UPPER(var)  ((var)->type != VAR_FREE && (var)->type != VAR_LOWER)
#define HAS_LHS(con)    ((con)->type != CON_FREE && (con)->type != CON_RHS)
#define HAS_RHS(con)    ((con)->type != CON_FREE && (con)->type != CON_LHS)

#endif /* _RATLPTYPES_H_ */
