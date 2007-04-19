#pragma ident "@(#) $Id: term2.c,v 1.1 2007/04/19 19:18:20 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: term.c                                                        */
/*   Name....: Term Functions                                                */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2006 by Thorsten Koch <koch@zib.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* #define TRACE 1 */

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "xlpglue.h"

#define TERM_EXTEND_SIZE 16
#define TERM_SID         0x5465726d

typedef struct term_element TermElem;

struct term_element
{
   const Entry* entry;
   Numb*        coeff;
};

struct term
{
   SID
   Numb*     constant;
   int       size;
   int       used;
   TermElem* elem;
};

#pragma ident "@(#) $Id: term2.c,v 1.1 2007/04/19 19:18:20 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: term.c                                                        */
/*   Name....: Term Functions                                                */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2006 by Thorsten Koch <koch@zib.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* #define TRACE 1 */

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "xlpglue.h"

#define TERM_EXTEND_SIZE 16
#define TERM_SID         0x5465726d

struct term
{
   SID
   Numb*     constant; // ???
   int       entry_size;
   int       monom_size:
   int       entry_used;
   int       monom_used;
   int*      monom;  /* monoms */
   int*      next;   /* entrys */
   Numb*     coeff;  /* monoms */
   Entry**   entry;  /* entrys */
};


Term* term_new(int size)
{
   Term* term = calloc(1, sizeof(*term));

   Trace("term_new");
   
   assert(term != NULL);
   assert(size >  0);
   
   term->constant   = numb_new_integer(0);
   term->entry_size = size;
   term->monom_size = size;
   term->entry_used = 0;
   term->monom_used = 0;
   term->monom      = calloc(term->size, sizeof(*term->monom));
   term->next       = calloc(term->size, sizeof(*term->next));
   term->coeff      = calloc(term->size, sizeof(*term->coeff));
   term->entry      = calloc(term->size, sizeof(*term->entry));
   
   SID_set(term, TERM_SID);
   assert(term_is_valid(term));

   return term;
}

void term_add_elem(Term* term, const Entry* entry, const Numb* coeff)
{
   Trace("term_add_elem");

   assert(term_is_valid(term));
   assert(entry_is_valid(entry));
   assert(!numb_equal(coeff, numb_zero()));
   assert(term->used <= term->size);
   
   if (term->monom_used == term->monom_size)
   {
      term->monom_size += TERM_EXTEND_SIZE;
      term->monom       = realloc(
         term->monom, (size_t)term->monom_size * sizeof(*term->monom));
      term->coeff       = realloc(
         term->coeff, (size_t)term->monom_size * sizeof(*term->coeff));

      assert(term->monom != NULL);
      assert(term->coeff != NULL);
   }
   assert(term->monom_used < term->monom_size);

   if (term->entry_used == term->entry_size)
   {
      term->entry_size += TERM_EXTEND_SIZE;
      term->entry       = realloc(
         term->entry, (size_t)term->entry_size * sizeof(*term->entry));
      term->next        = realloc(
         term->next, (size_t)term->entry_size * sizeof(*term->next));

      assert(term->entry != NULL);
      assert(term->coeff != NULL);
   }
   assert(term->entry_used < term->entry_size);
   
   term->monom[term->monom_used] = term->entry_used;
   term->coeff[term->monom_used] = numb_copy(coeff);
   term->entry[term->entry_used] = entry;
   term->next [term->entry_used] = -1;
   term->monom_used++;
   term->entry_used++;

   assert(term_is_valid(term));
}

void term_free(Term* term)
{
   int i;

   Trace("term_free");

   assert(term_is_valid(term));

   SID_del(term);

   for(i = 0; i < term->monom_used; i++)
      numb_free(term->coeff[i]);

   numb_free(term->constant);
   
   free(term->monom);
   free(term->next);
   free(term->coeff);
   free(term->entry);
   free(term);
}
