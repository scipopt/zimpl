#pragma ident "@(#) $Id: gmpmisc.c,v 1.1 2003/07/12 15:24:01 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: ratmisc.c                                                     */
/*   Name....: miscellenious rational arithmetic functions                   */
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
//#include <sys/types.h>
//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <gmp.h>
#include "bool.h"
#include "mshell.h"

#include "gmpmisc.h"

mpq_t const_zero;
mpq_t const_one;
mpq_t const_minus_one;

/* [+|-]?[0-9]*.[0-9]+[[e|E][+|-][0-9]+]? */
/* if it does not fit here, it doesn't fit in a double either */
void gmp_str2mpq(mpq_t value, const char* num)
{
   char  tmp[1024]; 
   int   i;
   int   k = 0;
   int   exponent = 0;
   int   fraction = 0;
   
   assert(num         != NULL);
   assert(strlen(num) <  32);

   /* printf("%s ", num); */
   
   /* Skip initial whitespace
    */
   while(isspace(*num))
      num++;

   /* Skip initial +/-
    */
   if (*num == '+')
      num++;
   else if (*num == '-')
      tmp[k++] = *num++;
   
   for(i = 0; num[i] != '\0'; i++)
   {
      if (isdigit(num[i]))
      {
         tmp[k++]  = num[i];
         exponent -= fraction;
      }
      else if (num[i] == '.')
         fraction = 1;
      else if (tolower(num[i]) == 'e')
      {
         exponent += atoi(&num[i + 1]);
         break;
      }
   }
   while(exponent > 0)
   {
      tmp[k++] = '0';
      exponent--;
   }         
   tmp[k++] = '/';
   tmp[k++] = '1';

   while(exponent < 0)
   {
      tmp[k++] = '0';
      exponent++;
   }         
   tmp[k] = '\0';

   /* printf("%s\n", tmp);*/
   
   mpq_set_str(value, tmp, 10);
   mpq_canonicalize(value);
}

void gmp_print_mpq(FILE* fp, const mpq_t qval)
{
   mpf_t fval;
   
   mpf_init(fval);
   mpf_set_q(fval, qval);
   mpf_out_str(fp, 10, 32, fval);
   fprintf(fp, " = ");
   mpq_out_str(fp, 10, qval);
   fputc('\n', fp);
   mpf_clear(fval);
}
   
/*ARGSUSED*/
static void* gmp_alloc(size_t size)
{
   return malloc(size);
}

/*ARGSUSED*/
static void* gmp_realloc(void* ptr, size_t old_size, size_t new_size)
{
   return realloc(ptr, new_size);
}

/*ARGSUSED*/
static void gmp_free(void* ptr, size_t size)
{
   free(ptr);
}


void gmp_init(Bool verbose)
{
   mp_set_memory_functions(gmp_alloc, gmp_realloc, gmp_free);

   mpq_init(const_zero);
   mpq_init(const_one);
   mpq_init(const_minus_one);

   mpq_set_ui(const_one, 1, 1);        /* = 1 */
   mpq_set_si(const_minus_one, -1, 1); /* = -1 */

   if (verbose)
      printf("Using GMP Version %s\n", gmp_version);
}

void gmp_exit()
{
   mpq_clear(const_zero);
   mpq_clear(const_one);
   mpq_clear(const_minus_one);
}






