#pragma ident "@(#) $Id: portab.h,v 1.4 2003/03/18 11:47:59 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: portab.h                                                      */
/*   Name....: Portable Defintions                                           */
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

#ifndef _PORTAB_H_
#define _PORTAB_H_

#include <limits.h>
#include <math.h>
#include <float.h>

typedef int            Bool;

#ifndef TRUE
#define TRUE            ((Bool)1)
#endif
#ifndef FALSE
#define FALSE           ((Bool)0)
#endif

#ifndef SUCCESS
#define SUCCESS         0
#endif
#ifndef FAILURE
#define FAILURE       (-1)
#endif

#ifndef Max
#define Max(a, b)    (((a) >= (b)) ? (a) : (b))
#endif
#ifndef Min
#define Min(a, b)    (((a) <= (b)) ? (a) : (b)) 
#endif

#define INFINITY  (DBL_MAX / 1e5)
#define EPS_ZERO  (DBL_MIN * 1e5) /* (DBL_EPSILON * 1e4)*/

#ifndef Fsgn
#define Fsgn(x)   ((((x) > -EPS_ZERO) && ((x) < EPS_ZERO)) ? 0 : (((x) < 0.0) ? -1 : 1))
#endif /* fsgn */

/* Hi Fortran !
 */
#define EQ(a, b)   (fabs((a) - (b)) <= EPS_ZERO)
#define NE(a, b)   (fabs((a) - (b)) >  EPS_ZERO)
#define LT(a, b)   (((a) - (b))     < -EPS_ZERO)
#define LE(a, b)   (((a) - (b))     <  EPS_ZERO)
#define GT(a, b)   (((a) - (b))     >  EPS_ZERO)
#define GE(a, b)   (((a) - (b))     > -EPS_ZERO)

#ifdef __GNUC__
#define does_not_return   __attribute__((noreturn))
#else
#define does_not_return   /**/
#define inline            /**/
#endif

#ifdef TRACE
#define Trace(fname) fprintf(stderr, "Trace: %s\n", fname);
#else
#define Trace(fname) /* */
#endif /* TRACE */

#endif   /* PORTAB_H */
