#ident "@(#) $Id: lpstore.c,v 1.4 2002/05/26 12:44:57 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: lpstore.c                                                     */
/*   Name....: Store Linear Programm                                         */
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
#include "mshell.h"
#include "portab.h"
#include "rbt.h"
#include "mme.h"
#include "lpstore.h"

struct storage
{
   int  size;
   Nzo* begin;
   Sto* next;
};

static const unsigned int sto_size  = 1000;
static const unsigned int tree_size = 1000;

static Lps* lp = NULL;

#ifndef NDEBUG
static Bool lps_valid()
{
   const char* err1  = "Wrong Previous Variable";
   const char* err2  = "Wrong Variable Previous Nonzero";
   const char* err3  = "Wrong Variable Nonzero Count";
   const char* err4  = "Wrong Variable Count";
   const char* err5  = "Wrong Previous Constraint";
   const char* err6  = "Wrong Constraint Previous Nonzero";
   const char* err7  = "Wrong Constraint Nonzero Count";
   const char* err8  = "Wrong Constraint Count";
   const char* err9  = "Wrong Variable";
   const char* err10 = "Wrong Constraint";
   const char* err11 = "Storage-size error";
   
   Var* var;
   Var* var_prev;
   Con* con;
   Con* con_prev;
   Nzo* nzo;
   Nzo* nzo_prev;
   Sto* sto;
   int  var_count;
   int  con_count;
   int  nzo_count;
   int  sto_count;
   
   assert(lp != NULL);

   /* Variable Test
    */
   var_count = lp->vars;
   
   for(var = lp->var_root, var_prev = NULL; var != NULL; var = var->next)
   {
      if (var_prev == var->prev)
         var_prev = var;
      else
      {
         fprintf(stderr, "%s\n", err1);
         return FALSE;
      }
      var_count--;
      nzo_count = var->size;
      
      for(nzo = var->first, nzo_prev = NULL; nzo != NULL; nzo = nzo->var_next)
      {
         if (nzo_prev == nzo->var_prev)
            nzo_prev = nzo;
         else
         {
            fprintf(stderr, "%s\n", err2);
            return FALSE;
         }
         if (nzo->var != var)
         {
            fprintf(stderr, "%s\n", err9);
            return FALSE;
         }         
         nzo_count--;
      }
      if (nzo_count)
      {
         fprintf(stderr, "%s\n", err3);
         return FALSE;
      }
   }
   if (var_count)
   {
      fprintf(stderr, "%s\n", err4);
      return FALSE;
   }

   /* Constraint Test
    */
   con_count = lp->cons;
   
   for(con = lp->con_root, con_prev = NULL; con != NULL; con = con->next)
   {
      if (con_prev == con->prev)
         con_prev = con;
      else
      {
         fprintf(stderr, "%s\n", err5);
         return FALSE;
      }
      con_count--;
      nzo_count = con->size;
      
      for(nzo = con->first, nzo_prev = NULL; nzo != NULL; nzo = nzo->con_next)
      {
         if (nzo_prev == nzo->con_prev)
            nzo_prev = nzo;
         else
         {
            fprintf(stderr, "%s\n", err6);
            return FALSE;
         }
         if (nzo->con != con)
         {
            fprintf(stderr, "%s\n", err10);
            return FALSE;
         }         
         nzo_count--;
      }
      if (nzo_count)
      {
         fprintf(stderr, "%s\n", err7);
         return FALSE;
      }
   }
   if (con_count)
   {
      fprintf(stderr, "%s\n", err8);
      return FALSE;
   }

   /* Storage Test
    */
   sto_count = 0;
   
   for(sto = lp->sto_root; sto != NULL; sto = sto->next)
   {
      assert(sto->begin != NULL);
      
      sto_count++;
   }
   if (sto_count * (int)sto_size < lp->nonzeros)
   {
      fprintf(stderr, "%s %d %d %d\n",
         err11, sto_count, sto_size, lp->nonzeros);
      return FALSE;
   }
   return TRUE;
}
#endif /* !NDEBUG */

static void lps_storage()
{
   Sto*         s;
   Nzo*         n;
   unsigned int i;
         
   assert(lps_valid());
   assert(lp->next == NULL); 
  
   s = malloc(sizeof(*s));
   
   assert(s != NULL);
   
   n = malloc(sto_size * sizeof(*n));
   
   assert(n != NULL);
   
   for(i = 0; i < sto_size - 1; i++)
   {
      n[i].var_next = &n[i + 1];
#ifndef NDEBUG
      n[i].var_prev = NULL;
      n[i].con_next = NULL;
      n[i].con_prev = NULL;
      n[i].var      = NULL;
      n[i].con      = NULL;
      n[i].value    = 0.0;
#endif      
   }  
   n[i].var_next = NULL;
#ifndef NDEBUG
   n[i].var_prev = NULL;
   n[i].con_next = NULL;
   n[i].con_prev = NULL;
   n[i].var      = NULL;
   n[i].con      = NULL;
   n[i].value    = 0.0;
#endif      

   s->size       = sto_size;
   s->begin      = &n[0];
   s->next       = lp->sto_root;
   lp->sto_root  = s;
   lp->next      = s->begin;
}

static int var_name_cmp(
   const void* a,
   const void* b)
{
   assert(a != NULL);
   assert(b != NULL);

   return(strcmp((const char*)a, ((const Var*)b)->name));
}   
   
static int con_name_cmp(
   const void* a,
   const void* b)
{
   assert(a != NULL);
   assert(b != NULL);

   return(strcmp((const char*)a, ((const Con*)b)->name));
}   

void lps_alloc(
   const char* name)
{
   assert(name != NULL);
   
   lp = malloc(sizeof(*lp));
   
   assert(lp != NULL);

   lp->name     = strdup(name);
   lp->objname  = NULL;
   lp->type     = LP_LP;
   lp->direct   = LP_MIN;
   lp->vars     = 0;
   lp->cons     = 0;
   lp->nonzeros = 0;
   lp->var_root = NULL;
   lp->con_root = NULL;
   lp->sto_root = NULL;
   lp->next     = NULL;
   lp->var_tree = rbt_init(tree_size, var_name_cmp);
   lp->con_tree = rbt_init(tree_size, con_name_cmp);
   
   assert(lps_valid());
}

void lps_free()
{
   Var* var;
   Var* var_next;
   Con* con;
   Con* con_next;
   Sto* sto;
   Sto* sto_next;
   
   assert(lps_valid());

   rbt_exit(lp->var_tree);
   rbt_exit(lp->con_tree);
   
   for(sto = lp->sto_root; sto != NULL; sto = sto_next)
   {
      sto_next = sto->next;
       
      free(sto->begin);
      free(sto);
   }
   for(var = lp->var_root; var != NULL; var = var_next) 
   {
      var_next = var->next;

      free(var->name);
      free(var);
   }
   for(con = lp->con_root; con != NULL; con = con_next)
   {
      con_next = con->next;

      free(con->name);
      free(con);
   }
   if (lp->objname != NULL)
      free(lp->objname);
   
   free(lp->name);
   free(lp);
}

void lps_number()
{
   Var* var;
   Con* con;
   int  i;

   assert(lps_valid());
   
   for(var = lp->var_root, i = 0; var != NULL; var = var->next, i++)
   {
      assert(var != NULL);
      
      var->number = i;
   }
   assert(i == lp->vars);
   
   for(con = lp->con_root, i = 0; con != NULL; con = con->next, i++)
   {
      assert(con != NULL);
      
      con->number = i;
   }
   assert(i == lp->cons);
}

Var* lps_getvar(
   const char* name)
{
   Var* vr;
   
   assert(lps_valid());
   assert(name != NULL);

   vr = rbt_search(lp->var_tree, name);

   assert((vr == NULL) || (!strcmp(vr->name, name)));

   return vr;
}
   
Con* lps_getcon(
   const char* name)
{
   Con* cr;
   
   assert(lps_valid());
   assert(name != NULL);
    
   cr = rbt_search(lp->con_tree, name);

   assert((cr == NULL) || (!strcmp(cr->name, name)));

   return cr;   
}
   
Nzo* lps_getnzo(
   const Con* con,
   const Var* var)
{
   Nzo*   nzo;
   
   assert(lps_valid());
   assert(con != NULL);
   assert(var != NULL);   

   /* Whatever is shorter
    */
   if (var->size <= con->size)
   {
      for(nzo = var->first;  nzo != NULL; nzo = nzo->var_next)
         if (nzo->con == con)
            break;
   }
   else
   {
      for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
         if (nzo->var == var)
            break;
   }   
   assert((nzo == NULL) || ((nzo->var == var) && (nzo->con == con)));
   
   return nzo;
}
   
Var* lps_addvar(
   const char* name,
   VarType     type,
   double      lower,
   double      upper)
{
   Var* v;
   
   assert(lps_valid());
   assert(name             != NULL);
   assert(lps_getvar(name) == NULL);
   
   v = malloc(sizeof(*v));
   
   assert(v != NULL);
   
   v->name      = strdup(name);
   v->number    = lp->vars;
   v->size      = 0;
   v->type      = type;
   v->cost      = 0.0;
   v->lower     = (type == VAR_BIN) ? 0.0 : lower;
   v->upper     = (type == VAR_BIN) ? 1.0 : upper;
   v->first     = NULL;
   v->prev      = NULL;   
   v->next      = lp->var_root;
   lp->var_root = v;
   
   if (v->next != NULL)
   {
      assert(v->next->prev == NULL);
      
      v->next->prev = v;
   }
   lp->vars++;

   rbt_insert(lp->var_tree, v, RBT_STORE_REFERENCE);

   if (type != VAR_CON)
      lp->type = LP_IP;
   
   assert(lps_valid());

   return v;
}

Con* lps_addcon(
   const char*  name,
   ConType      type,
   double       rhs,
   unsigned int flags)
{
   Con* c;
   
   assert(lps_valid());
   assert(name             != NULL);
   assert(lps_getcon(name) == NULL);

   c = malloc(sizeof(*c));
   
   assert(c != NULL);
   
   c->name      = strdup(name);
   c->number    = lp->cons;   
   c->size      = 0;
   c->type      = type;
   c->flags     = flags;
   c->rhs       = rhs;
   c->first     = NULL;
   c->prev      = NULL;
   c->next      = lp->con_root;
   lp->con_root = c;
   
   if (c->next != NULL)
   {
      assert(c->next->prev == NULL);
      
      c->next->prev = c;
   }
   lp->cons++;

   rbt_insert(lp->con_tree, c, RBT_STORE_REFERENCE);

   assert(lps_valid());

   return c;
}
  
void lps_addnzo(
   Con*   con,
   Var*   var,
   double value)
{
   Nzo* nzo;
   
   assert(lps_valid());
   assert(con          != NULL);
   assert(var          != NULL);
   assert(fabs(value)  >  EPS_ZERO);

   /* Ins LP aufnehmen
    */
   if (lp->next == NULL)
      lps_storage();
      
   nzo = lp->next;

   assert(nzo != NULL);

   lp->next = nzo->var_next;
   lp->nonzeros++;

   nzo->value = value;
      
   /* In die Spalte aufnehmen
    */
   nzo->var      = var;
   nzo->var_prev = NULL;
   nzo->var_next = var->first;
   var->first    = nzo;
   var->size++;
   
   if (nzo->var_next != NULL)
   {
      assert(nzo->var_next->var_prev == NULL);
      
      nzo->var_next->var_prev = nzo;       
   }
   
   /* In die Zeile aufnehmen
    */
   nzo->con      = con;
   nzo->con_prev = NULL;
   nzo->con_next = con->first;
   con->first    = nzo;
   con->size++;
   
   if (nzo->con_next != NULL)
   {
      assert(nzo->con_next->con_prev == NULL);
      
      nzo->con_next->con_prev = nzo;       
   }
   assert(lps_valid());
}

void lps_delnzo(
   Nzo* nzo)
{
   assert(lps_valid());
   assert(nzo != NULL);

   /* Aus der Spalte nahmen
    */
   if (nzo == nzo->var->first)
      nzo->var->first = nzo->var_next;
   
   if (nzo->var_prev != NULL)
      nzo->var_prev->var_next = nzo->var_next;
   if (nzo->var_next != NULL)
      nzo->var_next->var_prev = nzo->var_prev;
   nzo->var->size--;
   
   /* Aus der Zeile nahmen
    */
   if (nzo == nzo->con->first)
      nzo->con->first = nzo->con_next;
   
   if (nzo->con_prev != NULL)
      nzo->con_prev->con_next = nzo->con_next;
   if (nzo->con_next != NULL)
      nzo->con_next->con_prev = nzo->con_prev;
   nzo->con->size--;

   /* Aus dem LP nehmen
    */
   nzo->var_next = lp->next;
   lp->next      = nzo;
   lp->nonzeros--;
   
   assert(lps_valid());
}

void lps_setval(Nzo* nzo, double value)
{
   assert(nzo != NULL);

   nzo->value = value;   
}

double lps_getval(const Nzo* nzo)
{
   assert(nzo != NULL);

   return nzo->value;
}

void lps_setdir(
   LpDirect direct)
{
   assert(lps_valid());

   lp->direct = direct;
}

void lps_objname(
   const char* name)
{
   assert(lps_valid());
   assert(name != NULL);
   
   if (lp->objname != NULL)
      free(lp->objname);
   
   lp->objname = strdup(name);
}

double lps_getcost(
   const Var* var)
{
   assert(var != NULL);

   return var->cost;
}

void lps_setcost(
   Var*   var,
   double cost)
{
   assert(var != NULL);

   var->cost = cost;   
}

void lps_setlower(
   Var*   var,
   double lower)
{
   assert(var   != NULL);
   assert(lower <  INFINITY - EPS_ZERO);
   assert(lower <= var->upper + EPS_ZERO);

   var->lower = lower;   
}

void lps_setupper(
   Var*   var,
   double upper)
{
   assert(var   != NULL);
   assert(upper >  -INFINITY + EPS_ZERO);
   assert(upper >= var->lower - EPS_ZERO);
   
   var->upper = upper;   
}

void lps_setsense(
   Con*    con,
   ConType type)
{
   assert(con != NULL);

   con->type = type;   
}

void lps_setrhs(
   Con*   con,
   double rhs)
{
   assert(con != NULL);

   con->rhs = rhs;   
}

void lps_stat()
{
   assert(lps_valid());

   printf("Name: [%s]   Variables: %d   Constraints: %d   Non Zeros: %d\n",
      lp->name, lp->vars, lp->cons, lp->nonzeros);
}

int lps_getsense(const Con* con)
{
   assert(con != NULL);
   
   switch(con->type)
   {
   case CON_LE :
      return 'L';
   case CON_GE :
      return 'G';
   case CON_EQ :
      return 'E';
   default :
      abort();
   }
}

void lps_write(FILE* fp, LpForm format)
{
   lps_number();

   switch(format)
   {
   case LP_FORM_LPF :
      lpf_write(lp, fp);
      break;
   case LP_FORM_MPS :
      mps_write(lp, fp);
      break;
   default :
      abort();
   }
}

void lps_makename(
   char*       target,
   int         size,
   const char* name,
   int         no)
{
   char* temp;
   int   len;
   int   i;
   
   assert(target != NULL);
   assert(size   >= MPS_NAME_LEN);
   assert(name   != NULL);
   assert(no     >= 0);
   
   if (strlen(name) < (size_t)size)
      strcpy(target, name);
   else
   {
      /* The 16 is to make sure the %d later has enough room
       */
      temp = malloc((size_t)size + 16);

      assert(temp != NULL);
      
      sprintf(temp, "%d", no);

      /* -2 : fuer '#' und '\0'
       */
      len = size - (int)strlen(temp) - 2;

      assert(len >= 0);
   
      for(i = 0; i < len; i++)      
         target[i] = isalnum(name[i]) ? name[i] : '_';

      target[i++] = '#';

      strcpy(&target[i], temp);
   
      assert(strlen(target) == (size_t)size - 1);

      free(temp);
   }
}

void lps_transtable(FILE* fp)
{
   Var* var;
   Con* con;
   char temp[MPS_NAME_LEN + 1];

   assert(lps_valid());

   assert(fp != NULL);

   lps_number();
   
   for(var = lp->var_root; var != NULL; var = var->next)
   {
      lps_makename(temp, MPS_NAME_LEN + 1, var->name, var->number);

      fprintf(fp, "zimpl\tv %7d\t%-*s\t\"%s\"\n",
         var->number, MPS_NAME_LEN, temp, var->name);
   }
   for(con = lp->con_root; con != NULL; con = con->next)
   {
      lps_makename(temp, MPS_NAME_LEN + 1, con->name, con->number);

      fprintf(fp, "zimpl\tc %7d\t%-*s\t%s\n",
         con->number, MPS_NAME_LEN, temp, con->name);
   }
}

void lps_scale()
{
   Con*   con;
   Nzo*   nzo;
   double maximum;
   double scale;
   
   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if ((con->flags & LP_FLAG_CON_SCALE) > 0)
      {         
         maximum = con->rhs;
      
         for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
            if (fabs(nzo->value) > maximum)
               maximum = fabs(nzo->value);

         scale     = 1.0 / maximum;
         con->rhs *= scale;

         for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
            nzo->value *= scale;
      }
   }
}


