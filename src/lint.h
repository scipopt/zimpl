#pragma ident "@(#) $Id: lint.h,v 1.3 2003/03/18 11:47:59 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: lint.h                                                        */
/*   Name....: Lint defines                                                  */
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

/* Use this file only if we are linting
 */
#ifdef _lint

#ifndef _LINT_H_
#define _LINT_H_

/* Unfortunately strdup() is not a POSIX function.
 */
/*lint -sem(strdup, nulterm(1), malloc(1p), 1p, @p > 0) */ 
extern char* strdup(const char* s);

/*lint -esym(757, optarg, optind, opterr, optopt) */
/*lint -sem(getopt, 1n > 0 && 2p && 3p) */
extern int getopt(int argc, char* const argv[], const char* optstring);
extern char* optarg;
extern int optind;
extern int opterr;
extern int optopt;

/*lint -function(fopen, popen) */
extern FILE* popen(const char *command, const char *type);
/*lint -function(fclose, pclose) */
/*lint -esym(534, pclose) */
extern int   pclose(FILE *stream);

#endif /* _LINT_H_ */
#endif /* _lint */
