#pragma ident "@(#) $Id: iread.c,v 1.6 2003/03/18 11:47:59 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: iread.c                                                       */
/*   Name....: Read Instruction                                              */
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
#include <ctype.h>

#include "lint.h"
#include "portab.h"
#include "mshell.h"
#include "mme.h"
#include "code.h"
#include "inst.h"

/* read "datei" as "<1n,3s,12s,4n> %6s" skip 17 use 2000 fs " ;"
 */
#define MAX_LINE_LEN 8192
#define MAX_FIELDS    256

CodeNode* i_read_new(CodeNode* self)
{
   RDef*       rdef;
   const char* filename;
   const char* template;
   
   Trace("i_read_new");
   
   assert(code_is_valid(self));

   filename = code_eval_child_strg(self, 0);
   template = code_eval_child_strg(self, 1);   
   rdef     = rdef_new(filename, template);

   code_value_rdef(self, rdef);

   rdef_free(rdef);

   return self;
}

CodeNode* i_read_param(CodeNode* self)
{
   RDef*       rdef;
   const RPar* rpar;
   
   Trace("i_read_param");
   
   assert(code_is_valid(self));

   rdef = rdef_copy(code_eval_child_rdef(self, 0));
   rpar = code_eval_child_rpar(self, 1);   

   rdef_set_param(rdef, rpar);
   
   code_value_rdef(self, rdef);

   rdef_free(rdef);

   return self;
}

CodeNode* i_read_comment(CodeNode* self)
{
   RPar*       rpar;
   const char* comment;
   
   Trace("i_read_comment");
   
   assert(code_is_valid(self));

   comment = code_eval_child_strg(self, 0);
   rpar     = rpar_new_comment(comment);   

   code_value_rpar(self, rpar);

   rpar_free(rpar);

   return self;
}

CodeNode* i_read_use(CodeNode* self)
{
   RPar* rpar;
   int   use;
   
   Trace("i_read_use");
   
   assert(code_is_valid(self));

   use  = (int)code_eval_child_numb(self, 0);
   rpar = rpar_new_use(use);   

   code_value_rpar(self, rpar);

   rpar_free(rpar);

   return self;
}

CodeNode* i_read_skip(CodeNode* self)
{
   RPar* rpar;
   int   skip;
   
   Trace("i_read_skip");
   
   assert(code_is_valid(self));

   skip = (int)code_eval_child_numb(self, 0);
   rpar = rpar_new_skip(skip);   

   code_value_rpar(self, rpar);

   rpar_free(rpar);

   return self;
}

static int parse_template(
   const CodeNode* self,
   const char*     template,
   int*            param_field,
   int*            param_type,
   Bool*           is_tuple_list)
{
   const char* sep = " ,<>";
   
   char* temp = strdup(template);
   char* s;
   int   field;
   char  type;
   int   params = 0;
   
   assert(self          != NULL);
   assert(template      != NULL);
   assert(param_field   != NULL);
   assert(param_type    != NULL);
   assert(is_tuple_list != NULL);
   
   /* Is this a tuple_list "<1n,2s>" or
    * an entry_list "<1n,2n> 3s" template ?
    */
   if (  (NULL               == strchr(temp, '>'))
      || (NULL               == strchr(temp, '<'))
      || (strrchr(temp, '>') != strchr(temp, '>'))
      || (strrchr(temp, '<') != strchr(temp, '<')))
   {
      fprintf(stderr, "*** Error: Not a valid read template\n");
      code_errmsg(self);
      abort();
   }
   s = strchr(temp, '>');

   assert(s != NULL);
   
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
      field--;
      
      if ((field < 0) || (field >= MAX_FIELDS))
      {
         fprintf(stderr, "*** Error: Invalid field number [%d]\n", field + 1);
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
   free(temp);

   if (params - (is_tuple_list ? 0 : 1) < 1)
   {
      fprintf(stderr, "*** Error: Invalid read template, not enough fields\n");
      code_errmsg(self);
      abort();
      
   }
   return params;
}

static int split_fields(char* s, char* field[])
{
   char* t = s;
   char* u;
   int   fields = 0;
   Bool  new_field;
   
   for(;;)
   {
      new_field = FALSE;
      
      switch(*s)
      {
      case '\"' :
         s++;
         t = s;
         while((*s != '\0') && (*s != '\"'))
            s++;
         new_field = TRUE;
         break;
      case '\0' :
      case '\t' :
      case ' ' :
      case ',' :
      case ';' :
      case ':' :
         new_field = TRUE;
         break;
      default :
         s++;
         break;
      }
      if (new_field)
      {
         u = s;
         field[fields] = t;
         fields++;

         if (*s == '\"')
            s++;
         while(isspace(*s))
            s++;
         if (*s == ',' || *s == ';' || *s == ':')
            s++;
         while(isspace(*s))
            s++;
         *u = '\0';

         if (*s == '\0')
            break;

         t  = s;
      }
   }
   return fields;
}

CodeNode* i_read(CodeNode* self)
{
   FILE*       fp;
   char*       s;
   char*       t;
   char        buf[MAX_LINE_LEN];
   char*       field[MAX_FIELDS];
   int         fields;
   int         param_field[MAX_FIELDS];  /* template parameter field number */
   int         param_type [MAX_FIELDS];  /* template parameter field type */
   Bool        is_tuple_list;
   int         dim;
   const RDef* rdef;
   List*       list = NULL;
   Elem*       elem;
   Tuple*      tuple;
   Entry*      entry;
   const char* filename;
   char*       comment;
   int         skip;
   int         use;
   int         i;
   int         line = 0;
   
   Trace("i_read");
   
   assert(code_is_valid(self));
   
   rdef     = code_eval_child_rdef(self, 0);
   use      = rdef_get_use(rdef);
   skip     = rdef_get_skip(rdef);
   filename = rdef_get_filename(rdef);
   dim      = parse_template(self,
      rdef_get_template(rdef), param_field, param_type, &is_tuple_list);

   comment    = malloc(strlen(rdef_get_comment(rdef)) + 2);

   assert(comment != NULL);
   
   comment[0] = '\n';

   strcpy(&comment[1], rdef_get_comment(rdef));

   /* The last template parameter is the value for the entry_list.
    */
   if (!is_tuple_list)
      dim--;

   if (verbose)
      printf("Reading %s\n", filename);
   
   if (NULL == (fp = fopen(filename, "r")))
   {
      perror(filename);
      code_errmsg(self);
      abort();
   }
   else
   {
      while(NULL != fgets(buf, sizeof(buf), fp))
      {
         /* Count the line
          */
         line++;

         /* Should we skip this line ?
          */
         if (skip-- > 0)
            continue;

         /* Is this a comment line or is there a comment on the line,
          * then remove it.
          */
         if (NULL != (s = strpbrk(buf, comment)))
            *s = '\0';

         /* If this is an empty line...
          */
         for(s = buf; (*s != '\0') && isspace(*s); s++)
            ;

         /* ... we skip it.
          */
         if (*s == '\0')
            continue;

         /* Now we break the line in fields.
          */
#if 0
         fields = 0;
        
         for(s = strtok(buf, fieldsep);
            (s != NULL) && (fields < MAX_FIELDS);
             s = strtok(NULL, fieldsep))
            field[fields++] = s;
#endif
         fields = split_fields(s, field);
#if 0
         fprintf(stderr, "Fields=%d\n", fields);
         for(i = 0; i < fields; i++)
            fprintf(stderr, "Field[%d]=[%s]\n", i, field[i]);    
#endif
         tuple = tuple_new(dim);
         
         for(i = 0; i < dim; i++)
         {
            if (param_field[i] >= fields)
            {
               fprintf(stderr, "*** Error: Not enough fields in data\n");
               fprintf(stderr, "***        File: %s line %d\n",
                  rdef_get_filename(rdef), line);
               code_errmsg(self);
               abort();
            }
            t    = field[param_field[i]];
            elem = (param_type[i] == 'n')
               ? elem_new_numb(atof(t))
               : elem_new_strg(str_new(t));
            
            tuple_set_elem(tuple, i, elem);

            elem_free(elem);
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
            if (param_field[i] >= fields)
            {
               fprintf(stderr, "*** Error: Not enough fields in data\n");
               fprintf(stderr, "***        File: %s line %d\n",
                  rdef_get_filename(rdef), line);
               code_errmsg(self);
               abort();
            }
            t     = field[param_field[i]];
            entry = (param_type[i] == 'n')
               ? entry_new_numb(tuple, atof(t))
               : entry_new_strg(tuple, str_new(t));
            
            if (list == NULL)
               list = list_new_entry(entry);
            else
               list_add_entry(list, entry);

            entry_free(entry);
         }
         tuple_free(tuple);
         
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

   list_free(list);
   free(comment);

   return self;
}
