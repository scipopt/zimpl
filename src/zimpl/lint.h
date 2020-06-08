/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: lint.h                                                        */
/*   Name....: Lint defines                                                  */
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
#ifndef _LINT_H_
#define _LINT_H_

#ifndef _lint

#include <assert.h>

#else // Use this only if we are linting

extern void __assert(bool);  // this special function has the assert semantic by default
#define assert(x) /*lint -e{453} */ __assert(x)  

//typedef _Bool LintBool;

#endif /* _lint */


#if defined(__GNUC__) || defined(__CLANG__) || defined(_lint)

#define UNUSED            __attribute__ ((unused))
#define NORETURN          __attribute__ ((noreturn))

#define is_UNUSED         __attribute__ ((unused))            //lint !e755
#define is_NORETURN       __attribute__ ((noreturn))          //lint !e755
#define is_CONST          __attribute__ ((const))             //lint !e755  
#define is_PURE           __attribute__ ((pure))              //lint !e755
#define returns_NONNULL   __attribute__ ((returns_nonnull))   //lint !e755

#else

#define UNUSED            /**/
#define NORETURN          /**/
#define is_UNUSED         /**/
#define is_NORETURN       /**/
#define is_CONST          /**/
#define is_PURE           /**/
#define returns_NONNULL   /**/

#endif // // __GNUC__ || __CLANG__ || _lint

#if (defined(__GNUC__) || defined(__CLANG__)) && !defined(_lint)

#define is_MALLOC         __attribute__ ((malloc))            //lint !e755
#define always_INLINE     __attribute__ ((always_inline))     //lint !e755
#define has_SIZE1         __attribute__ ((alloc_size (1)))    //lint !e755
#define has_SIZE1x2       __attribute__ ((alloc_size (1,2)))  //lint !e755
#define is_COLD           __attribute__ ((cold))              //lint !e755
#define is_HOT            __attribute__ ((hot))               //lint !e755
#define expects_NONNULL   __attribute__ ((nonnull))           //lint !e755
#define expects_NONNULL1  __attribute__ ((nonnull (1)))       //lint !e755
#define expects_NONNULL12 __attribute__ ((nonnull (1,2)))     //lint !e755
#define expects_NONNULL13 __attribute__ ((nonnull (1,3)))     //lint !e755

#else

#define is_MALLOC         /**/
#define always_INLINE     /**/
#define has_SIZE1         /**/
#define has_SIZE1x2       /**/
#define is_COLD           /**/
#define is_HOT            /**/
#define expects_NONNULL   /**/
#define expects_NONNULL1  /**/
#define expects_NONNULL12 /**/
#define expects_NONNULL13 /**/

#endif // (__GNUC__ || __CLANG__) && !_lint
   
   
#if 0  // not needed anymore for pc-lint 1.2 ?
#ifdef __cplusplus
extern "C" {
#endif

/* Unfortunately strdup() is not a POSIX function.
 */
/*lint -sem(strdup, 1p, @P >= malloc(1)) */ 
extern char* strdup(const char* s);

/* It is not clear if isinf() and isnan() are already POSIX
 * or only in the next Draft.
 */
extern int isinf(double);
extern int isnan(double);
extern int isfinite(double);
extern int finite(double); /* This is probably not POSIX */

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
/*lint -sem(fsync, 1n >= 0, @n <= 0) */
extern int   fsync(int fd);

/* zlib support
 */
/*lint -esym(534,gzclose) */

typedef void* gzFile;

/*lint -sem(  gzopen, 1p && 2p, r_null) */
extern gzFile gzopen(const char *path, const char *mode);
/*lint -sem(  gzread, 1p == 1 && 2p) */
extern int    gzread(gzFile file, void* buf, unsigned len);
/*lint -sem(  gzwrite, 1p == 1 && 2p) */
extern int    gzwrite(gzFile file, const void* buf, unsigned len);
/*lint -sem(  gzputs, 1p == 1 && 2p) */
extern int    gzputs(gzFile file, const char *s);
/*lintx -sem(  gzgets, 1p == 1 && 2p > 0 && 2P <= 3n && 3n > 0, @P == 2P || @P == 0) */
/*lint -function(fgets(1), gzgets(2)) */
/*lint -function(fgets(2), gzgets(3)) */
/*lint -function(fgets(3), gzgets(1)) */
/*lint -function(fgets(r), gzgets(r)) */
extern char*  gzgets(gzFile file, char *buf, int len);
/*lint -function(fgetc, gzgetc) */
extern int    gzgetc(gzFile file);
/*lint -sem(  gzclose, 1p == 1) */
extern int    gzclose(gzFile file);

#ifdef __cplusplus
}
#endif
#endif // 0

#endif /* _LINT_H_ */



