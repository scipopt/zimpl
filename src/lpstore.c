#pragma ident "@(#) $Id: lpstore.c,v 1.13 2003/03/18 11:47:59 bzfkocht Exp $"
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

#include "lint.h"
#include "mshell.h"
#include "portab.h"
#include "mme.h"
#include "lpstore.h"

#define HASH_BUCKETS  65537U

struct storage
{
   int  size;
   Nzo* begin;
   Sto* next;
};

static const unsigned int sto_size  = 1000;

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
   const char* err12 = "Wrong Variable SID";
   const char* err13 = "Wrong Constraint SID";
   
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
      if (var->sid != VAR_SID)
      {
         fprintf(stderr, "%s\n", err12);
         return FALSE;
      }
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
      if (con->sid != CON_SID)
      {
         fprintf(stderr, "%s\n", err13);
         return FALSE;
      }
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
   lp->var_hash = hash_new(HASH_VAR, HASH_BUCKETS);
   lp->con_hash = hash_new(HASH_CON, HASH_BUCKETS);
   
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

   hash_free(lp->var_hash);
   hash_free(lp->con_hash);
   
   for(sto = lp->sto_root; sto != NULL; sto = sto_next)
   {
      sto_next = sto->next;
       
      free(sto->begin);
      free(sto);
   }
   for(var = lp->var_root; var != NULL; var = var_next) 
   {
      var_next = var->next;

#ifndef NDEBUG
      var->sid = 0x0;
#endif
      free(var->name);
      free(var);
   }
   for(con = lp->con_root; con != NULL; con = con_next)
   {
      con_next = con->next;

#ifndef NDEBUG
      con->sid = 0x0;
#endif
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
      assert(var      != NULL);
      assert(var->sid == VAR_SID);
      
      var->number = i;
   }
   assert(i == lp->vars);
   
   for(con = lp->con_root, i = 0; con != NULL; con = con->next, i++)
   {
      assert(con      != NULL);
      assert(con->sid == CON_SID);
      
      con->number = i;
   }
   assert(i == lp->cons);
}

const Var* lps_getvar(
   const char* name)
{
   const Var* vr;
   
   assert(lps_valid());
   assert(name != NULL);

   vr = hash_lookup_var(lp->var_hash, name);

   assert((vr == NULL) || (vr->sid == VAR_SID));
   assert((vr == NULL) || (!strcmp(vr->name, name)));

#ifndef NDEBUG
   {
      const Var* var;
      
      for(var = lp->var_root; var != NULL; var = var->next)
         if (!strcmp(var->name, name))
            break;

      assert(var == vr);
   }
#endif
   return vr;
}
   
const Con* lps_getcon(
   const char* name)
{
   const Con* cr;
   
   assert(lps_valid());
   assert(name != NULL);
    
   cr = hash_lookup_con(lp->con_hash, name);

   assert((cr == NULL) || (cr->sid == CON_SID));
   assert((cr == NULL) || (!strcmp(cr->name, name)));

#ifndef NDEBUG
   {
      const Con* con;
      
      for(con = lp->con_root; con != NULL; con = con->next)
      {
         if (!strcmp(con->name, name))
            break;
      }
      assert(con == cr);
   }
#endif
   return cr;   
}
   
Nzo* lps_getnzo(
   const Con* con,
   const Var* var)
{
   Nzo*   nzo;
   
   assert(lps_valid());
   assert(con      != NULL);
   assert(con->sid == CON_SID);
   assert(var      != NULL);   
   assert(var->sid == VAR_SID);

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
   double      upper,
   int         priority,
   double      startval)
{
   Var* v;
   
   assert(lps_valid());
   assert(name             != NULL);
   assert(lps_getvar(name) == NULL);
   
   v = malloc(sizeof(*v));
   
   assert(v != NULL);

   v->sid       = VAR_SID;
   v->name      = strdup(name);
   v->number    = lp->vars;
   v->size      = 0;
   v->type      = type;
   v->cost      = 0.0;
   v->lower     = (type == VAR_BIN) ? 0.0 : lower;
   v->upper     = (type == VAR_BIN) ? 1.0 : upper;
   v->priority  = priority;
   v->startval  = startval;
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

   hash_add_var(lp->var_hash, v);

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

   c->sid       = CON_SID;
   c->name      = strdup(name);
   c->number    = lp->cons;   
   c->size      = 0;
   c->type      = type;
   c->flags     = flags;
   c->rhs       = rhs;
   c->scale     = 1.0;
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

   hash_add_con(lp->con_hash, c);

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
   assert(con         != NULL);
   assert(con->sid    == CON_SID);
   assert(var         != NULL);   
   assert(var->sid    == VAR_SID);
   assert(fabs(value) >  EPS_ZERO);

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

unsigned int lps_hash(const char* s)
{
   unsigned int hcode = 0;
   
   /* I am not too sure this works well for 64bit integers.
    */
   assert(sizeof(hcode) == 4); 

   for(; *s != '\0'; s++)
      hcode = DISPERSE((unsigned char)*s + hcode);

   return hcode;
}

const char* lps_varname(const Var* var)
{
   assert(var               != NULL);
   assert(var->name         != NULL);

   return var->name;
}

const char* lps_conname(const Con* con)
{
   assert(con               != NULL);
   assert(con->name         != NULL);

   return con->name;
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
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   
   return var->cost;
}

void lps_setcost(
   Var*   var,
   double cost)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);

   var->cost = cost;   
}

void lps_setlower(
   Var*   var,
   double lower)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   assert(lower    <  INFINITY - EPS_ZERO);
   assert(lower    <= var->upper + EPS_ZERO);

   var->lower = lower;   
}

void lps_setupper(
   Var*   var,
   double upper)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   assert(upper    >  -INFINITY + EPS_ZERO);
   assert(upper    >= var->lower - EPS_ZERO);
   
   var->upper = upper;   
}

void lps_setsense(
   Con*    con,
   ConType type)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);
   
   con->type = type;   
}

void lps_setrhs(
   Con*   con,
   double rhs)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

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
   assert(con      != NULL);
   assert(con->sid == CON_SID);
   
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
      if (var->size == 0)
      {
         if ((fabs(var->cost) < EPS_ZERO) || (fabs((lp->direct == LP_MIN) ?
            var->lower : var->upper) < EPS_ZERO))
            continue;
      }
      lps_makename(temp, MPS_NAME_LEN + 1, var->name, var->number);

      fprintf(fp, "zimpl\tv %7d\t%-*s\t\"%s\"\n",
         var->number, MPS_NAME_LEN, temp, var->name);
   }
   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if ((con->size > 0) || (fabs(con->rhs) > EPS_ZERO))
      {
         lps_makename(temp, MPS_NAME_LEN + 1, con->name, con->number);

         fprintf(fp, "zimpl\tc %7d\t%-*s\t\"%s\"\t%.16e\n",
            con->number, MPS_NAME_LEN, temp, con->name, con->scale);
      }
   }
}

void lps_orderfile(FILE* fp)
{
   ord_write(lp, fp);
}

void lps_scale()
{
   Con*   con;
   Nzo*   nzo;
   double maximum;
   
   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if ((con->flags & LP_FLAG_CON_SCALE) > 0)
      {         
         maximum = con->rhs;
      
         for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
            if (fabs(nzo->value) > maximum)
               maximum = fabs(nzo->value);

         con->scale = 1.0 / maximum;
         con->rhs  *= con->scale;

         for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
            nzo->value *= con->scale;
      }
   }
}


