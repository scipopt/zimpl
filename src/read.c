#ident "@(#) $Id: read.c,v 1.1 2001/05/06 11:43:21 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: inst.c                                                        */
/*   Name....: Instructions                                                  */
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
#include <alloca.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"
#include "code.h"
#include "inst.h"

/* read "datei" as "<1n,3s,12s,4n> %6s" skip 17 use 2000 fs " ;"
 */

#define MAX_LINE_LEN 8192
#define MAX_FIELDS    256

#define RDEF_SID     0x52446566
#define RPAR_SID     0x52506172

enum read_param_type { RPAR_ERR = 0, RPAR_SKIP, RPAR_USE, RPAR_FS };

typedef enum read_param_type   RParType;
typedef union read_param_value RParVal
typedef struct read_param      RPar;
typedef struct read_definition RDef;

union read_param_value
{
   int   i;
   char* s;
};

struct read_parameter
{
   SID
   RParType  type;
   RParValue val;
};
   
struct read_definition
{
   SID
   const char* filename;
   const char* template;
   const char* fieldsep;
   int         count;
   int         skip;
   int         refc;
};

RDef* rdef_new(const char* filename, const char* template)
{
   RDef* rdef = calloc(1, sizeof(*rdef));

   assert(self != NULL);
   assert(rdef != NULL);
   
   rdef->filename = filename;
   rdef->template = template;
   rdef->fieldsep = str_new(";");
   rdef->skip     = 0;
   rdef->use      = -1;
   rdef->refc     = 1;
   
   SID_set(set, SET_SID);

   assert(rdef_is_valid(rpar));
   
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
      && (rdef->fieldsep != NULL));
}

void rdef_copy(RDef* rdef)
{
   assert(rdef_is_valid(rdef));

   rdef->refc++;

   return rdef;
}

void rdef_set_param(RDef* rdef, RPar* rpar)
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
   case RPAR_FS :
      rdef->fieldsep = rpar->val.s;
      break;
   case RPAR_ERR :
   default :
      abort();
   }
}

static int parse_template(
   const CodeNode* self,
   const char*     template,
   int*            param_field,
   int*            param_type,
   Bool*           is_tuple_list)
{
   const char* sep = " ,<>";
   
   char* temp = strcpy(alloca(strlen(template) + 1), template);
   int   field;
   int   type;
   int   params = 0;
   
   assert(self          != NULL);
   assert(template      != NULL);
   assert(param_field   != NULL);
   assert(param_type    != NULL);
   assert(is_tuple_list != NULL);
   
   /* Is this a tuple_list "<1n,2s>" or
    * an entry_list "<1n,2n> 3s" template ?
    */
   if (NULL == (s = strchr(temp, '>')))
   {
      fprintf(stderr, "*** Error: Not a valid read template\n");
      code_errmsg(self);
      abort();
   }
   for(++s; isspace(*s); s++)
      ;

   *is_tuple_list = (*s == '\0');

   /* Here we start to rip the template apart
    */
   for(s = strtok(temp, sep);
      (s != NULL) && (params < MAX_FIELDS);
       s = strtok(NULL, sep))
   {
      if (2 != sscanf(s, "%d%c", &field, &type))
      {
         fprintf(stderr, "*** Error: Invalid read template syntax\n");
         code_errmsg(self);
         abort();
      }
      if ((field <= 0) || (field >= MAX_FIELDS))
      {
         fprintf(stderr, "*** Error: Invalid field number [%d]\n", field);
         code_errmsg(self);
         abort();
      }
      if ((type != 'n') && (type != 's'))
      {
         fprintf(stderr, "*** Error: Invalid field type [%c]\n", type);
         code_errmsg(self);
         abort();
      }
      param_field[params] = field;
      param_type [params] = type;
      params++;
   }
   return params;
}

static void i_read(CodeNode* self)
{
   FILE*    fp;
   char*    s;
   char*    t;
   char*    buf[MAX_LINE_LEN];
   char*    field[MAX_FIELDS];
   int      fields;
   int      param_field[MAX_FIELDS];  /* template element field number */
   ElemType param_type [MAX_FIELDS];  /* template element field type */
   Bool     is_tuple_list;
   int      dim;
   
   RDef*    rdef;
   List*    list = NULL;
   Elem*    elem;
   Tuple*   tuple;
   Entry*   entry;
   int      skip;
   int      use;
   
   rdef = Code_eval_child_rdef(self, 0);
   use  = rdef->use;
   skip = rdef->skip;
   dim  = parse_template(
      self, rdef->template, param_field, param_type, is_tuple_list);

   /* The last template parameter is the value for the entry_list.
    */
   if (!is_tuple_list)
      dim--;
   
   if (NULL == (fp = fopen(rdef->filename, "r")))
   {
      perror(rdef->filename);
      code_errmsg(self);
      abort();
   }
   else
   {
     list  = list_new_tuple(tuple);
     list  = list_new_entry(entry);

     while(NULL != fgets(buf, sizeof(buf), fp))
     {
        if (skip-- > 0)
           continue;
        
        fields = 0;
        
        for(s = strtok(buf, rdef->fieldsep);
           (s != NULL) && (fields < MAX_FIELDS);
            s = strtok(NULL, rdef->fieldsep))
           field[fields++] = s;
        
        tuple = tuple_new(dim);
        
        for(i = 0; i < dim; i++)
        {
           t    = field[param_field[i]];
           elem = (param_type[i] == 'n')
              ? elem_new_numb(atof(t));
              : elem_new_strg(str_new(t));
           
           tuple_set_elem(tuple, i, elem);
        }
        
        if (is_tuple_list)
        {
           if (list == NULL)
              list = list_new_tuple(tuple);
           else
              list_add_tuple(list, tuple);
        }
        else
        {
           t     = field[param_field[i]];
           entry = (param_type[i] == 'n')
              ? entry_new_numb(tuple, atof(t));
              : entry_new_strg(tuple, str_new(t));
           
           if (list == NULL)
              list = list_new_entry(entry);
           else
              list_add_entry(list, entry);   
        }
        if (--use == 0)
           break;
     }
     fclose(fp);
   }

   if (list == NULL)
   {
      fprintf(stderr, "*** Error: Read from file found no data\n");
      code_errmsg(self);
      abort();
   }
   code_value_list(self, list);
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

RPar* rpar_new_fieldsep(const char* fieldsep)
{
   RPar* rpar = calloc(1, sizeof(*rpar));

   assert(rpar != NULL);
   
   rpar->type  = RPAR_SKIP;
   rpar->val.s = fieldsep;
   
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
   return ((rpar != NULL) && SID_ok(rpar, RDEF_SID)
      && (rpar->type != RPAR_ERR));
}

RPar* rpar_copy(RPar* rpar)
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

