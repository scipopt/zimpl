#pragma ident "@(#) $Id: rdefpar.c,v 1.6 2003/07/12 15:24:02 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: rdefpar.c                                                     */
/*   Name....: Read Definition / Parameter                                   */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"

#define RDEF_SID     0x52446566
#define RPAR_SID     0x52506172

enum read_param_type { RPAR_ERR = 0, RPAR_SKIP, RPAR_USE, RPAR_CMNT };

typedef enum read_param_type   RParType;
typedef union read_param_value RParVal;

union read_param_value
{
   int         i;
   const char* s;
};

struct read_param
{
   SID
   RParType type;
   RParVal  val;
};
   
struct read_definition
{
   SID
   const char* filename;
   const char* template;
   const char* comment;
   int         use;
   int         skip;
   int         refc;
};

RDef* rdef_new(const char* filename, const char* template)
{
   RDef* rdef = calloc(1, sizeof(*rdef));

   assert(filename != NULL);
   assert(template != NULL);
   assert(rdef     != NULL);
   
   rdef->filename = filename;
   rdef->template = template;
   rdef->comment  = str_new("");
   rdef->skip     = 0;
   rdef->use      = -1;
   rdef->refc     = 1;
   
   SID_set(rdef, RDEF_SID);

   assert(rdef_is_valid(rdef));
   
   return rdef;
}

void rdef_free(RDef* rdef)
{
   assert(rdef_is_valid(rdef));

   rdef->refc--;
   
   if (rdef->refc == 0)
   {
      SID_del(rdef);

      free(rdef);
   }
}

Bool rdef_is_valid(const RDef* rdef)
{
   return ((rdef != NULL)
      && SID_ok(rdef, RDEF_SID)
      && (rdef->filename != NULL)
      && (rdef->template != NULL)
      && (rdef->comment  != NULL));
}

RDef* rdef_copy(const RDef* source)
{
   RDef* rdef = (RDef*)source;
   
   assert(rdef_is_valid(rdef));

   rdef->refc++;

   return rdef;
}

void rdef_set_param(RDef* rdef, const RPar* rpar)
{
   assert(rdef_is_valid(rdef));
   assert(rpar_is_valid(rpar));

   switch(rpar->type)
   {
   case RPAR_SKIP :
      rdef->skip = rpar->val.i;
      break;
   case RPAR_USE :
      rdef->use  = rpar->val.i;
      break;
   case RPAR_CMNT :
      rdef->comment = rpar->val.s;
      break;
   case RPAR_ERR :
   default :
      abort();
   }
}

const char* rdef_get_filename(const RDef* rdef)
{
   assert(rdef_is_valid(rdef));
   
   return rdef->filename;
}

const char* rdef_get_template(const RDef* rdef)
{
   assert(rdef_is_valid(rdef));
   
   return rdef->template;
}

const char* rdef_get_comment(const RDef* rdef)
{
   assert(rdef_is_valid(rdef));
   
   return rdef->comment;
}

int rdef_get_use(const RDef* rdef)
{
   assert(rdef_is_valid(rdef));
   
   return rdef->use;
}

int rdef_get_skip(const RDef* rdef)
{
   assert(rdef_is_valid(rdef));
   
   return rdef->skip;
}

/* ----------------------------------------------------------------------------
 * Read Parameter
 * ----------------------------------------------------------------------------
 */
RPar* rpar_new_skip(int skip)
{
   RPar* rpar = calloc(1, sizeof(*rpar));

   assert(rpar != NULL);

   rpar->type  = RPAR_SKIP;
   rpar->val.i = skip;
   
   SID_set(rpar, RPAR_SID);

   assert(rpar_is_valid(rpar));

   return rpar;
}

RPar* rpar_new_use(int use)
{
   RPar* rpar = calloc(1, sizeof(*rpar));

   assert(rpar != NULL);
   
   rpar->type  = RPAR_USE;
   rpar->val.i = use;

   SID_set(rpar, RPAR_SID);

   assert(rpar_is_valid(rpar));

   return rpar;
}

RPar* rpar_new_comment(const char* comment)
{
   RPar* rpar = calloc(1, sizeof(*rpar));

   assert(rpar != NULL);
   
   rpar->type  = RPAR_CMNT;
   rpar->val.s = comment;
   
   SID_set(rpar, RPAR_SID);

   assert(rpar_is_valid(rpar));

   return rpar;
}

void rpar_free(RPar* rpar)
{
   assert(rpar_is_valid(rpar));

   SID_del(rpar);

   free(rpar);
}

Bool rpar_is_valid(const RPar* rpar)
{
   return ((rpar != NULL) && SID_ok(rpar, RPAR_SID)
      && (rpar->type != RPAR_ERR));
}

RPar* rpar_copy(const RPar* rpar)
{
   RPar* rpnew = calloc(1, sizeof(*rpar));

   assert(rpar  != NULL);
   assert(rpnew != NULL);
   
   rpnew->type = rpar->type;
   rpnew->val  = rpar->val;
   
   SID_set(rpnew, RPAR_SID);

   assert(rpar_is_valid(rpnew));

   return rpnew;
}

