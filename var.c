#ident "@(#) $Id: var.c,v 1.1 2001/01/26 07:11:37 thor Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: var.c                                                         */
/*   Name....: Variable Functions                                            */
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

#define VAR_SID  0x5661725f

struct variable
{
   SID
   int     refc;
   VarType type;
   double  lower;
   double  upper;
};

Var* var_new(VarType type, double lower, double upper)
{
   Var* var = calloc(1, sizeof(*var));

   assert(var != NULL);

   var->refc  = 1;
   var->type  = type;
   var->lower = lower;
   var->upper = upper;

   SID_set(var, VAR_SID);
   assert(var_is_valid(var));
   
   return var;
}

void var_free(Var* var)
{
   assert(var_is_valid(var));

   var->refc--;

   if (var->refc == 0)
   {
      SID_del(var);   
      free(var);
   }
}

int var_is_valid(const Var* var)
{
   return ((var != NULL) && SID_ok(var, VAR_SID) && (var->refc > 0));
}

Var* var_copy(Var* var)
{
   assert(var_is_valid(var));

   var->refc++;

   return var;
}

void var_print(FILE* fp, const Var* var)
{
   const char* text;
   
   assert(var_is_valid(var));
   
   switch(var->type)
   {
   case VAR_CON :
      text = "Con";
      break;
   case VAR_INT :
      text = "Int";
      break;      
   case VAR_BIN :
      text = "Bin";
      break;
   default :
      text = "Var-ERR";
      break;
   }
   fprintf(fp, "%s [%g..%g]", text, var->lower, var->upper);
}


