/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: rdefpar.c                                                     */
/*   Name....: Read Definition / Parameter                                   */
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
#ifndef _RDEFPAR_H_
#define _RDEFPAR_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct read_param        RPar;
typedef struct read_definition   RDef;

//lint -sem(       rdef_new, 1p, 2p, @P >= malloc(1)) 
extern RDef*       rdef_new(const char* filename, const char* pattern) expects_NONNULL returns_NONNULL;
//lint -sem(       rdef_free, custodial(1), inout(1), 1p == 1) 
extern void        rdef_free(RDef* rdef) expects_NONNULL;
//lint -sem(       rdef_is_valid, pure, 1p == 1) 
extern bool        rdef_is_valid(const RDef* rdef) expects_NONNULL is_PURE;
//lint -sem(       rdef_copy, 1p == 1, @P >= malloc(1)) 
extern RDef*       rdef_copy(const RDef* rdef) expects_NONNULL returns_NONNULL;
//lint -sem(       rdef_set_param, inout(1), 1p == 1, 2p == 1) 
extern void        rdef_set_param(RDef* rdef, const RPar* rpar) expects_NONNULL;
//lint -sem(       rdef_get_filename, pure, 1p == 1, @p) 
extern const char* rdef_get_filename(const RDef* rdef) expects_NONNULL returns_NONNULL is_PURE;
//lint -sem(       rdef_get_pattern, pure, 1p == 1, @p) 
extern const char* rdef_get_pattern(const RDef* rdef) expects_NONNULL returns_NONNULL is_PURE;
//lint -sem(       rdef_get_comment, pure, 1p == 1, @p) 
extern const char* rdef_get_comment(const RDef* rdef) expects_NONNULL returns_NONNULL is_PURE;
//lint -sem(       rdef_get_match, pure, 1p == 1, r_null) 
extern const char* rdef_get_match(const RDef* rdef) expects_NONNULL is_PURE;
//lint -sem(       rdef_get_use, pure, 1p == 1, chneg(@)) 
extern int         rdef_get_use(const RDef* rdef) expects_NONNULL is_PURE;
//lint -sem(       rdef_get_skip, pure, 1p == 1, chneg(@)) 
extern int         rdef_get_skip(const RDef* rdef) expects_NONNULL is_PURE;

//lint -sem(       rpar_new_skip, chneg(1), @P >= malloc(1)) 
extern RPar*       rpar_new_skip(int skip) returns_NONNULL;
//lint -sem(       rpar_new_use, chneg(1), @P >= malloc(1)) 
extern RPar*       rpar_new_use(int use) returns_NONNULL;
//lint -sem(       rpar_new_comment, 1p, @P >= malloc(1)) 
extern RPar*       rpar_new_comment(const char* comment) expects_NONNULL returns_NONNULL;
//lint -sem(       rpar_new_match, 1p, @P >= malloc(1)) 
extern RPar*       rpar_new_match(const char* match) expects_NONNULL returns_NONNULL;
//lint -sem(       rpar_free, custodial(1), inout(1), 1p == 1) 
extern void        rpar_free(RPar* rpar) expects_NONNULL;
//lint -sem(       rpar_is_valid, pure, 1p == 1) 
extern bool        rpar_is_valid(const RPar* rpar) expects_NONNULL is_PURE;
//lint -sem(       rpar_copy, 1p == 1, @P >= malloc(1)) 
extern RPar*       rpar_copy(const RPar* rpar) expects_NONNULL returns_NONNULL;

#ifdef __cplusplus
}
#endif
#endif // _RDEFPAR_H_ 

