#ident "@(#) $Id: strstore.c,v 1.2 2001/01/26 12:18:28 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: strstore.c                                                    */
/*   Name....: String Storage Functions                                      */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "portab.h"
#include "mshell.h"
#include "mme.h"

typedef struct string Str;

struct string
{
   char* value;
   Str*  next;
};

static Str* anchor = NULL;

const char* str_new(const char* s)
{
   Str* str = malloc(sizeof(*str));

   assert(s   != NULL);
   assert(str != NULL);

   str->value = strdup(s);
   str->next  = anchor;
   anchor     = str;

   assert(str->value != NULL);
   
   return str->value;
}

void str_init(void)
{
}

void str_exit(void)
{
   Str* p;
   Str* q;

   for(p = anchor; p != NULL; p = q)
   {
      q = p->next;
      free(p->value);
      free(p);
   }
   anchor = NULL;
}

unsigned int str_hash(const char* s)
{
   unsigned int sum;
   int          i;
   
   for(sum = 0, i = 0; s[i] != '\0'; i++)
      sum = DISPERSE(sum + s[i]);

   return sum;
}

