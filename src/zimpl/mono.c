/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mono.c                                                        */
/*   Name....: Monom Functions                                               */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2007-2021 by Thorsten Koch <koch@zib.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

/* #define TRACE 1 */

#include "zimpl/lint.h"
#include "zimpl/attribute.h"
#include "zimpl/mshell.h"

#include "zimpl/ratlptypes.h"
#include "zimpl/numb.h"
#include "zimpl/elem.h"
#include "zimpl/tuple.h"
#include "zimpl/mme.h"
#include "zimpl/set.h"
#include "zimpl/entry.h"

#include "zimpl/mono.h"

#define MONO_SID         0x4d6f6e6f
#define MOEL_SID         0x4d6f456c

extern int lps_varnumber(const Var* var) expects_NONNULL is_PURE; 

Mono* mono_new(const Numb* coeff, const Entry* entry, MFun fun)
{
   Mono* mono = calloc(1, sizeof(*mono));

   Trace("mono_new");

   assert(mono != NULL);
   assert(entry_is_valid(entry));
   assert(entry_get_type(entry) == SYM_VAR);

   mono->count       = 1;
   mono->coeff       = numb_copy(coeff);
   mono->fun         = fun;
   mono->first.entry = entry_copy(entry);
   mono->first.next  = NULL;

   SID_set(mono, MONO_SID);
   SID_set2(mono->first, MOEL_SID);
   
   assert(mono_is_valid(mono));

   return mono;
}

#ifndef NDEBUG
bool mono_is_valid(const Mono* mono)
{
   const MonoElem* e;
   int             count = 1;
   
   assert(SID_ok(mono, MONO_SID));
   assert(SID_ok2(mono->first, MOEL_SID));
   assert(mono->count > 0);

   mem_check(mono);

   assert(entry_is_valid(mono->first.entry));
   
   for(e = mono->first.next; e != NULL; e = e->next)
   {      
      count++;
      
      mem_check(e);

      assert(SID_ok(e, MOEL_SID));      
      assert(entry_is_valid(e->entry));
      assert(entry_get_type(e->entry) == SYM_VAR);

      // Variables in mono are ordered
      assert(e->next == NULL || (lps_varnumber(entry_get_var(e->entry)) <= lps_varnumber(entry_get_var(e->next->entry))));
   }
   assert(count == mono->count);
   
   return true;
}
#endif

void mono_free(Mono* mono)
{
   MonoElem* e;
   MonoElem* q;

   Trace("mono_free");

   assert(mono_is_valid(mono));
   
   for(e = mono->first.next; e != NULL; e = q)
   {
      q = e->next;
      entry_free(e->entry);
      SID_del(e);
      free(e);
   }   
   entry_free(mono->first.entry);
   numb_free(mono->coeff);
   SID_del2(mono->first);
   SID_del(mono);
   free(mono);
}

void mono_mul_entry(
   Mono*        mono,
   const Entry* entry)
{
   Trace("mono_mul_entry");

   assert(mono_is_valid(mono));
   assert(entry_is_valid(entry));
   assert(entry_get_type(entry) == SYM_VAR);

   MonoElem* new_me = calloc(1, sizeof(*new_me));
   
   Var* var       = entry_get_var(entry);
   int  varnumber = lps_varnumber(var);
   
   /* Do we have to put the new entry at the start of the list?
    */
   if (lps_varnumber(entry_get_var(mono->first.entry)) >= varnumber)
   {
      *new_me = mono->first;

      mono->first.entry = entry_copy(entry);
      mono->first.next  = new_me;
      SID_set2(mono->first, MOEL_SID);
   }
   else
   {
   /* This ensures that if the same variable is to come several times,
    * all of them come together, i.e. yxy is not allowed, yyx would be ok.
    * Neccessary for simplify of terms and other things.
    */
      MonoElem* last = NULL;

      for(MonoElem* e = &mono->first; e != NULL; e = e->next)
      {
         assert(var != entry_get_var(e->entry));

         last = e;

         if (e->next == NULL)
            break;
         
         assert(entry_is_valid(e->next->entry));

         if (varnumber <= lps_varnumber(entry_get_var(e->next->entry)))
            break;
      }
      assert(last != NULL);
       
      new_me->entry   = entry_copy(entry);
      new_me->next    = last->next;
      SID_set(new_me, MOEL_SID);
      last->next      = new_me;
   }
   mono->count++;
   
   assert(mono_is_valid(mono));
}

Mono* mono_copy(const Mono* mono)
{
   Mono*     mnew;
   MonoElem* e;
   
   assert(mono_is_valid(mono));

   mnew = mono_new(mono->coeff, mono->first.entry, mono->fun);
   
   for(e = mono->first.next; e != NULL; e = e->next)
      mono_mul_entry(mnew, e->entry);

   assert(mono_is_valid(mnew));

   return mnew;
}
     
void mono_mul_coeff(const Mono* mono, const Numb* value)
{
   Trace("mono_mul_coeff");

   assert(mono_is_valid(mono));
   assert(numb_is_valid(value));

   numb_mul(mono->coeff, value);
}

void mono_add_coeff(const Mono* mono, const Numb* value)
{
   Trace("mono_add_coeff");

   assert(mono_is_valid(mono));
   assert(numb_is_valid(value));

   numb_add(mono->coeff, value);
}

unsigned int mono_hash(const Mono* mono)
{
   size_t          hcode = 0;
   const MonoElem* e;
   
   assert(mono_is_valid(mono));   
   
   for(e = &mono->first; e != NULL; e = e->next)
      hcode += ((size_t)entry_get_var(e->entry)) >> 2;

   return DISPERSE((unsigned int)hcode);
}

/** Checks whether two monoms cosist of the same variables.
 */  
#if 1
/* We assume (I think it is true that there is only one distinct entry per var
 */
bool mono_equal(const Mono* ma, const Mono* mb)
{
   const MonoElem* ea;
   const MonoElem* eb;
   
   assert(mono_is_valid(ma));   
   assert(mono_is_valid(mb));   

   if (ma->count != mb->count)
      return false;

   if (ma->count == 1 && (ma->first.entry != mb->first.entry))
      return false;

   for(ea = &ma->first; ea != NULL; ea = ea->next)
   {
      const Entry* entry_a = ea->entry;

      assert(entry_is_valid(entry_a));
      assert(entry_get_type(entry_a) == SYM_VAR);
           
      for(eb = &mb->first; eb != NULL; eb = eb->next)
         if (entry_a == eb->entry)
            break;

      if (eb == NULL)
         return false;
      
      /* Now all variables of a kind are consecutive 
       */
      while(ea->next != NULL && ea->next->entry == entry_a) 
      {
         if (eb->next == NULL || eb->next->entry != entry_a)
            return false;
               
         ea = ea->next; /*lint !e850 loop index variable is modified in body of the loop */
         eb = eb->next;               
      }
   } 
   return true;
}
#else /* old */
bool mono_equal(const Mono* ma, const Mono* mb)
{
   const MonoElem* ea;
   const MonoElem* eb;
   Var*            var_a;
   
   assert(mono_is_valid(ma));   
   assert(mono_is_valid(mb));   

   if (ma->count != mb->count)
      return false;

   if (ma->count == 1 && (entry_get_var(ma->first.entry) != entry_get_var(mb->first.entry)))
      return false;

   for(ea = &ma->first; ea != NULL; ea = ea->next)
   {
      assert(entry_is_valid(ea->entry));

      var_a = entry_get_var(ea->entry);
      
      for(eb = &mb->first; eb != NULL; eb = eb->next)
         if (var_a == entry_get_var(eb->entry))
            break;

      if (eb == NULL)
         return false;
      
      /* Now all variables of a kind are consecutive 
       */
      while(ea->next != NULL && entry_get_var(ea->next->entry) == var_a)
      {
         if (eb->next == NULL || entry_get_var(eb->next->entry) != var_a)
            return false;
               
         ea = ea->next;
         eb = eb->next;               
      }
   }
   return true;
}
#endif

Mono* mono_mul(const Mono* ma, const Mono* mb)
{
   Mono*           mono;
   const MonoElem* eb;
   
   assert(mono_is_valid(ma));   
   assert(mono_is_valid(mb));   

   mono = mono_copy(ma);

   numb_mul(mono->coeff, mb->coeff);

   for(eb = &mb->first; eb != NULL; eb = eb->next)
   {
      assert(entry_is_valid(eb->entry));
      
      mono_mul_entry(mono, eb->entry);
   }
   assert(mono_is_valid(mono));

   return mono;
}

void mono_neg(Mono* mono)
{
   assert(mono_is_valid(mono));

   numb_neg(mono->coeff);
}

bool mono_is_linear(const Mono* mono)
{
   assert(mono_is_valid(mono));

   /* ??? Effect not fully testet */
   return mono->count == 1 && (mono->fun == MFUN_NONE || mono->fun == MFUN_TRUE || mono->fun == MFUN_FALSE);
}

int mono_get_degree(const Mono* mono)
{
   assert(mono_is_valid(mono));

   return mono->count;
}

const Numb* mono_get_coeff(const Mono* mono)
{
   assert(mono_is_valid(mono));

   return mono->coeff;
}

void mono_set_function(Mono* mono, MFun f)
{
   assert(mono_is_valid(mono));

   mono->fun = f;

   assert(mono_is_valid(mono));
}

MFun mono_get_function(const Mono* mono)
{
   assert(mono_is_valid(mono));

   return mono->fun;
}

Var* mono_get_var(const Mono* mono, int idx)
{
   const MonoElem* e = &mono->first;
   
   assert(mono_is_valid(mono));
   assert(mono->count > 0);
   assert(idx >= 0);
   assert(idx <= mono->count);

   while(idx > 0)
   {
      assert(e != NULL);

      e = e->next;
      idx--;
   }
   assert(e != NULL);

   assert(entry_is_valid(e->entry));

   return entry_get_var(e->entry);
}

#ifndef NDEBUG
void mono_print(FILE* fp, const Mono* mono, bool print_symbol_index)
{
   const MonoElem* e;

   assert(mono_is_valid(mono));
   
   if (numb_equal(mono->coeff, numb_one()))
      fputc('+', fp);
   else
   {
      if (numb_cmp(mono->coeff, numb_zero()) >= 0)
         fprintf(fp, "+ %g", numb_todbl(mono->coeff));
      else
         fprintf(fp, "- %g", -numb_todbl(mono->coeff));
   }
   fputc(' ', fp);
   
   for(e = &mono->first; e != NULL; e = e->next)
   {      
      entry_print(fp, e->entry);

      if (print_symbol_index)
         tuple_print(fp, entry_get_tuple(e->entry));

      if (e->next != NULL) 
         fprintf(fp, " * ");
   }
}
#endif /* !NDEBUG */
