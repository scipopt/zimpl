/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: stmt.h                                                        */
/*   Name....: Statement Functions                                           */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2020 by Thorsten Koch <koch@zib.de>
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
#ifndef _STMT_H_
#define _STMT_H_

#ifdef __cplusplus
extern "C" {
#endif

enum statement_type
{
   STMT_ERR = 0, STMT_SET, STMT_PARAM, STMT_VAR, STMT_MIN, STMT_MAX,
   STMT_CONS, STMT_DEF, STMT_DO, STMT_SOS
};

typedef enum statement_type      StmtType;
typedef struct statement         Stmt;

/* stmt.c
 */
//lint -sem(        stmt_new, 2p, chneg(3), 4p, @P >= malloc(1)) 
extern Stmt*        stmt_new(StmtType type, const char* filename, int lineno,
   const char* text) expects_NONNULL returns_NONNULL;
//lint -sem(        stmt_free, custodial(1), inout(1), 1p == 1) 
extern void         stmt_free(Stmt* stmt) expects_NONNULL;
//lint -sem(        stmt_is_valid, pure, 1p == 1) 
extern bool         stmt_is_valid(const Stmt* stmt) expects_NONNULL;
//lint -sem(        stmt_get_filename, 1p == 1, @p) 
extern const char*  stmt_get_filename(const Stmt* stmt) expects_NONNULL returns_NONNULL;
//lint -sem(        stmt_get_lineno, 1p == 1, chneg(@)) 
extern int          stmt_get_lineno(const Stmt* stmt) expects_NONNULL;
//lint -sem(        stmt_get_text, 1p == 1, @p) 
extern const char*  stmt_get_text(const Stmt* stmt) expects_NONNULL returns_NONNULL;
//lint -sem(        stmt_parse, inout(1), 1p == 1) 
extern void         stmt_parse(Stmt* stmt) expects_NONNULL;
//lint -sem(        stmt_execute, inout(1), 1p == 1) 
extern void         stmt_execute(const Stmt* stmt) expects_NONNULL;
//lint -sem(        stmt_print, inout(1), 1p == 1, 2p == 1) 
extern void         stmt_print(FILE* fp, const Stmt* stmt) expects_NONNULL;
//lint -sem(        stmt_trigger_warning, chneg(1)) 
extern bool         stmt_trigger_warning(int no);

/* mmlparse.y
 */ 
extern int          yyparse(void);

/* mmlscan.l
 */
//lint -sem(        parse_stmt, 1p == 1) 
extern void         parse_stmt(const Stmt* stmt) expects_NONNULL;
//lint -sem(        scan_get_stmt, @p == 1) 
extern const Stmt*  scan_get_stmt(void) is_PURE;
//lint -sem(        scan_get_column, chneg(@p)) 
extern int          scan_get_column(void) is_PURE;

#ifdef __cplusplus
}
#endif
#endif // _STMT_H_ 
