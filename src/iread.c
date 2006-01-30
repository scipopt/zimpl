#pragma ident "@(#) $Id: iread.c,v 1.20 2006/01/30 11:19:43 bzfkocht Exp $"
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
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>

#ifndef _lint
#include <zlib.h>
#else
#include "lint.h"
#endif /* _lint */

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "code.h"
#include "inst.h"

/* read "datei" as "<1n,3s,12s,4n> %6s" skip 17 use 2000 fs " ;"
 */
#define MAX_LINE_LEN 8192
#define MAX_FIELDS    256

CodeNode* i_read_new(CodeNode* self)
{
   const char* filename;
   const char* template;
   
   Trace("i_read_new");
   
   assert(code_is_valid(self));

   filename = code_eval_child_strg(self, 0);
   template = code_eval_child_strg(self, 1);   

   code_value_rdef(self, rdef_new(filename, template));

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

   return self;
}

CodeNode* i_read_comment(CodeNode* self)
{
   const char* comment;
   
   Trace("i_read_comment");
   
   assert(code_is_valid(self));

   comment = code_eval_child_strg(self, 0);

   code_value_rpar(self, rpar_new_comment(comment));

   return self;
}

CodeNode* i_read_use(CodeNode* self)
{
   const Numb* use;
   int         int_use;
   
   Trace("i_read_use");
   
   assert(code_is_valid(self));

   use = code_eval_child_numb(self, 0);

   if (!numb_is_int(use))
   {
      fprintf(stderr, "*** Error 147: use value ");
      numb_print(stderr, use);
      fprintf(stderr, " is too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   int_use = numb_toint(use);

   if (int_use <= 0)
   {
      fprintf(stderr, "*** Error 148: use value %d is not positive\n", int_use);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   code_value_rpar(self, rpar_new_use(int_use));

   return self;
}

CodeNode* i_read_skip(CodeNode* self)
{
   const Numb* skip;
   int         int_skip;
   
   Trace("i_read_skip");
   
   assert(code_is_valid(self));

   skip = code_eval_child_numb(self, 0);

   if (!numb_is_int(skip))
   {
      fprintf(stderr, "*** Error 149: skip value ");
      numb_print(stderr, skip);
      fprintf(stderr, " is too big or not an integer\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   int_skip = numb_toint(skip);
   
   if (int_skip < 0)
   {
      fprintf(stderr, "*** Error 150: skip value %d is negative\n", int_skip);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   code_value_rpar(self, rpar_new_skip(int_skip));

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
   Bool  is_single_value = FALSE;
   
   assert(self          != NULL);
   assert(template      != NULL);
   assert(param_field   != NULL);
   assert(param_type    != NULL);
   assert(is_tuple_list != NULL);
   
   /* Is this a tuple_list "<1n,2s>" or
    * an entry_list "<1n,2n> 3s" template
    * or a single value "2n" ?
    */
   /*lint -e{731} supress "Boolean argument to equal/not equal"
    */
   if (( (NULL == strchr(temp, '>')) != (NULL == strchr(temp, '<')))
      || (strrchr(temp, '>') != strchr(temp, '>'))
      || (strrchr(temp, '<') != strchr(temp, '<')))
   {
      fprintf(stderr, "*** Error 151: Not a valid read template\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   s = strchr(temp, '>');

   if (s == NULL)
   {
      *is_tuple_list  = FALSE;
      is_single_value = TRUE;
   }
   else
   {
      for(++s; isspace(*s); s++)
         ;

      *is_tuple_list = (*s == '\0');
   }
   /* Here we start to rip the template apart
    */
   for(s = strtok(temp, sep);
      (s != NULL) && (params < MAX_FIELDS);
       s = strtok(NULL, sep))
   {
      if (2 != sscanf(s, "%d%c", &field, &type))
      {
         fprintf(stderr, "*** Error 152: Invalid read template syntax\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      field--;
      
      if ((field < 0) || (field >= MAX_FIELDS))
      {
         fprintf(stderr, "*** Error 153: Invalid field number [%d]\n", field + 1);
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      if ((type != 'n') && (type != 's'))
      {
         fprintf(stderr, "*** Error 154: Invalid field type [%c]\n", type);
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
      param_field[params] = field;
      param_type [params] = type;
      params++;
   }
   free(temp);

   if (is_single_value)
   {
      if (params != 1)
      {
         fprintf(stderr, "*** Error 201: Invalid read template, only one field allowed\n");
         code_errmsg(self);
         zpl_exit(EXIT_FAILURE);
      }
   }
   else if (params - (*is_tuple_list ? 0 : 1) < 1)
   {
      fprintf(stderr, "*** Error 155: Invalid read template, not enough fields\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   return params;
}

/*lint -sem(split_fields, nulterm(1), 1p && 2p, @n >= 0) */
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
         /*lint --e{661}  Possible access of out-of-bounds pointer */
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

         t = s;
      }
   }
   return fields;
}

CodeNode* i_read(CodeNode* self)
{
   gzFile      fp;
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
   Numb*       numb;
   char*       filename;
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
   dim      = parse_template(self,
      rdef_get_template(rdef), param_field, param_type, &is_tuple_list);

   filename = malloc(strlen(rdef_get_filename(rdef)) + 4);
   assert(filename != NULL);
   strcpy(filename, rdef_get_filename(rdef));

   comment  = malloc(strlen(rdef_get_comment(rdef)) + 3);
   assert(comment  != NULL);
   comment[0] = '\n';
   comment[1] = '\r';
   strcpy(&comment[2], rdef_get_comment(rdef));

   /* The last template parameter is the value for the entry_list.
    */
   if (!is_tuple_list)
      dim--;

   if (access(filename, R_OK) != 0)
   {
      strcat(filename, ".gz");

      /* If .gz also does not work, revert to the old name
       * to get a better error message.
       */
      if (access(filename, R_OK) != 0)
         strcpy(filename, rdef_get_filename(rdef));
   }

   if (verbose >= VERB_NORMAL)
      printf("Reading %s\n", filename);

   if (NULL == (fp = gzopen(filename, "r")))
   {
      perror(filename);
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   else
   {
      while(NULL != gzgets(fp, buf, sizeof(buf)))
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
               fprintf(stderr, "*** Error 156: Not enough fields in data\n");
               fprintf(stderr, "***            File: %s line %d\n",
                  rdef_get_filename(rdef), line);
               code_errmsg(self);
               zpl_exit(EXIT_FAILURE);
            }
            t = field[param_field[i]];

            if (param_type[i] == 'n')
            {
               if (!numb_is_number(t))
               {
                  fprintf(stderr, "*** Error 174: Numeric field %d", i + 1);
                  fprintf(stderr, " read as \"%s\". This is not a number.\n", t);
                  fprintf(stderr, "***            File: %s line %d\n",
                     rdef_get_filename(rdef), line);
                  code_errmsg(self);
                  zpl_exit(EXIT_FAILURE);
               }
               numb = numb_new_ascii(t);
               elem = elem_new_numb(numb);
               numb_free(numb);
            }
            else
            {
               elem = elem_new_strg(str_new(t));
            }
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
            if (param_field[i] >= fields)
            {
               fprintf(stderr, "*** Error 157: Not enough fields in data (value)\n");
               fprintf(stderr, "***            File: %s line %d\n",
                  rdef_get_filename(rdef), line);
               code_errmsg(self);
               zpl_exit(EXIT_FAILURE);
            }
            t     = field[param_field[i]];
            if (param_type[i] == 'n')
            {
               Numb* n = numb_new_ascii(t);
               entry = entry_new_numb(tuple, n);
               numb_free(n);
            }
            else
            {
               entry = entry_new_strg(tuple, str_new(t));
            }
            
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
      gzclose(fp);
   }
   
   if (list == NULL)
   {
      fprintf(stderr, "*** Error 158: Read from file found no data\n");
      code_errmsg(self);
      zpl_exit(EXIT_FAILURE);
   }
   code_value_list(self, list);

   free(comment);
   free(filename);
   
   return self;
}



