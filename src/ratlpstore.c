#pragma ident "@(#) $Id: ratlpstore.c,v 1.3 2003/07/17 07:36:44 bzfkocht Exp $"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: lpstore.c                                                     */
/*   Name....: Store Linear Programm                                         */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2003 by Thorsten Koch <koch@zib.de>
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
#include <ctype.h>
#include <assert.h>

#include <gmp.h>

#include "lint.h"
#include "bool.h"
#include "ratlp.h"
#include "ratlpstore.h"

struct storage
{
   int  size;
   Nzo* begin;
   Sto* next;
};

static const unsigned int sto_size  = 1000;

#define LHT_BUCKETS  1000003U
#define DISPERSE(x) (1664525U * (x) + 1013904223U);

enum lps_hash_type { LHT_ERR = 0, LHT_VAR, LHT_CON };

typedef struct lps_hash_element LpsHElem;
typedef enum   lps_hash_type    LpsHashType;

struct lps_hash_element
{
   LpsHElem* next;
   union
   {
      Con* con;
      Var* var;
   } value;
};

struct lps_hash
{
   unsigned int size;
   int          elems;
   LpsHashType  type;
   LpsHElem**   bucket;
};

static void hash_statist(FILE* fp, const LpsHash* hash);

static Bool hash_valid(const LpsHash* hash)
{
   return hash != NULL && (hash->type == LHT_CON || hash->type == LHT_VAR);
}

static unsigned int hashit(const char* s)
{
   unsigned int hcode = 0;
   
   /* I am not too sure this works well for 64bit integers.
    */
   assert(sizeof(hcode) == 4); 

   for(; *s != '\0'; s++)
      hcode = DISPERSE((unsigned char)*s + hcode);

   return hcode;
}

static LpsHash* lps_hash_new(LpsHashType type)
{
   LpsHash* hash = calloc(1, sizeof(*hash));

   assert(hash != NULL);

   hash->size   = LHT_BUCKETS;
   hash->elems  = 0;
   hash->type   = type;
   hash->bucket = calloc(hash->size, sizeof(*hash->bucket));

   assert(hash->bucket != NULL);

   assert(hash_valid(hash));
   
   return hash;
}

static void lps_hash_free(LpsHash* hash)
{
   LpsHElem*    he;
   LpsHElem*    hq;
   unsigned int i;
      
   assert(hash_valid(hash));

#ifndef NDEBUG
   hash_statist(stdout, hash);
#endif
   
   for(i = 0; i < hash->size; i++)
   {
      for(he = hash->bucket[i]; he != NULL; he = hq)
      {
         hq = he->next;
         free(he);
      }
   }
   free(hash->bucket);
   free(hash);
}

/* Liefert NULL wenn nicht gefunden.
 */
static Var* hash_lookup_var(const LpsHash* hash, const char* name)
{
   unsigned int hcode;
   LpsHElem*    he     = NULL;
   
   assert(hash_valid(hash));
   assert(hash->type == LHT_VAR);
   assert(name != NULL);
   
   hcode  = hashit(name) % hash->size;

   for(he = hash->bucket[hcode]; he != NULL; he = he->next)
      if (!strcmp(he->value.var->name, name))
         break;

   return (he == NULL) ? (Var*)0 : he->value.var;
}

/* Liefert NULL wenn nicht gefunden.
 */
static Con* hash_lookup_con(const LpsHash* hash, const char* name)
{
   unsigned int hcode;
   LpsHElem*    he     = NULL;
   
   assert(hash_valid(hash));
   assert(hash->type == LHT_CON);
   assert(name != NULL);
   
   hcode  = hashit(name) % hash->size;

   for(he = hash->bucket[hcode]; he != NULL; he = he->next)
      if (!strcmp(he->value.con->name, name))
         break;

   return (he == NULL) ? (Con*)0 : he->value.con;
}

static void hash_add_var(LpsHash* hash, Var* var)
{
   LpsHElem*    he = calloc(1, sizeof(*he));
   unsigned int hcode;

   assert(hash_valid(hash));
   assert(var        != NULL);
   assert(hash->type == LHT_VAR);
   assert(he         != NULL);
   
   hcode               = hashit(var->name) % hash->size;
   he->value.var       = var;
   he->next            = hash->bucket[hcode];
   hash->bucket[hcode] = he;
   hash->elems++;

   assert(hash_lookup_var(hash, var->name) == var);
}

static void hash_add_con(LpsHash* hash, Con* con)
{
   LpsHElem*    he = calloc(1, sizeof(*he));
   unsigned int hcode;

   assert(hash_valid(hash));
   assert(con        != NULL);
   assert(hash->type == LHT_CON);
   assert(he         != NULL);
   
   hcode               = hashit(con->name) % hash->size;
   he->value.con       = con;
   he->next            = hash->bucket[hcode];
   hash->bucket[hcode] = he;
   hash->elems++;

   assert(hash_lookup_con(hash, con->name) == con);
}

static void hash_statist(FILE* fp, const LpsHash* hash)
{
   LpsHElem*    he;
   int          min    = (int)hash->size;
   int          max    = 0;
   int          sum    = 0;
   int          zeros  = 0;
   int          filled = 0;
   int          count;
   double       avg    = 0.0;
   unsigned int i;

   assert(fp != NULL);
   assert(hash_valid(hash));

   for(i = 0; i < hash->size; i++)
   {
      count = 0;
      
      for(he = hash->bucket[i]; he != NULL; he = he->next)
         count++;

      if (count == 0)
         zeros++;
      else
         filled++;

      if (count < min)
         min = count;
      if (count > max)
         max = count;
      sum += count;
   }
   assert(sum == hash->elems);

   if (filled > 0)
      avg = (double)sum / (double)filled;
   
   fprintf(fp,
      "HashStat: size=%u sum=%d min=%d max=%d avg=%.1f zero=%d filled=%d\n",
      hash->size, sum, min, max, avg, zeros, filled);
}

#ifndef NDEBUG
static Bool lps_valid(const Lps* lp)
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
   const char* err14 = "Strange lhs/rhs";
   
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
   mpq_t zero;
   
   assert(lp != NULL);

   mpq_init(zero);
   
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
      switch(con->type)
      {
      case CON_FREE :
      case CON_LHS :
      case CON_RHS :
         break;
      case CON_RANGE :
         if (mpq_cmp(con->lhs, con->rhs) >= 0)
         {
            fprintf(stderr, "%s %s %g %g\n",
               err14, con->name, mpq_get_d(con->lhs), mpq_get_d(con->rhs));
            return FALSE;
         }
         break;
      case CON_EQUAL :
         if (!mpq_equal(con->lhs, con->rhs))
         {
            fprintf(stderr, "%s %s %g %g\n",
               err14, con->name, mpq_get_d(con->lhs), mpq_get_d(con->rhs));
            return FALSE;
         }
         break;
      default :
         abort();
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
   mpq_clear(zero);
   
   return TRUE;
}
#endif /* !NDEBUG */

static void lps_storage(Lps* lp)
{
   Sto*         s;
   Nzo*         n;
   unsigned int i;
         
   assert(lps_valid(lp));
   assert(lp->next == NULL); 
  
   s = malloc(sizeof(*s));
   
   assert(s != NULL);
   
   n = malloc(sto_size * sizeof(*n));
   
   assert(n != NULL);
   
   for(i = 0; i < sto_size - 1; i++)
   {
      n[i].var_next = &n[i + 1];
      mpq_init(n[i].value);
      
#ifndef NDEBUG
      n[i].var_prev = NULL;
      n[i].con_next = NULL;
      n[i].con_prev = NULL;
      n[i].var      = NULL;
      n[i].con      = NULL;
#endif      
   }  
   n[i].var_next = NULL;
   mpq_init(n[i].value);
#ifndef NDEBUG
   n[i].var_prev = NULL;
   n[i].con_next = NULL;
   n[i].con_prev = NULL;
   n[i].var      = NULL;
   n[i].con      = NULL;
#endif      

   s->size       = sto_size;
   s->begin      = n; 
   s->next       = lp->sto_root;
   lp->sto_root  = s;
   lp->next      = s->begin;
}

Lps* lps_alloc(
   const char* name)
{
   Lps* lp;
   
   assert(name != NULL);
   
   lp = malloc(sizeof(*lp));
   
   assert(lp != NULL);

   lp->name     = strdup(name);
   lp->probname = NULL;
   lp->objname  = NULL;
   lp->rhsname  = NULL;
   lp->bndname  = NULL;
   lp->rngname  = NULL;
   lp->type     = LP_LP;
   lp->direct   = LP_MIN;
   lp->vars     = 0;
   lp->cons     = 0;
   lp->nonzeros = 0;
   lp->var_root = NULL;
   lp->con_root = NULL;
   lp->sto_root = NULL;
   lp->next     = NULL;
   lp->var_hash = lps_hash_new(LHT_VAR);
   lp->con_hash = lps_hash_new(LHT_CON);
   lp->var_last = NULL;
   lp->con_last = NULL;

   assert(lps_valid(lp));

   return lp;
}

void lps_free(Lps* lp)
{
   Var* var;
   Var* var_next;
   Con* con;
   Con* con_next;
   Sto* sto;
   Sto* sto_next;
   unsigned int  i;
   
   assert(lps_valid(lp));

   lps_hash_free(lp->var_hash);
   lps_hash_free(lp->con_hash);

   for(sto = lp->sto_root; sto != NULL; sto = sto_next)
   {
      for(i = 0; i < sto_size; i++)
         mpq_clear(sto->begin[i].value);

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
      mpq_clear(var->cost);
      mpq_clear(var->lower);
      mpq_clear(var->upper);
      mpq_clear(var->value);
      
      free(var->name);
      free(var);
   }
   for(con = lp->con_root; con != NULL; con = con_next)
   {
      con_next = con->next;

#ifndef NDEBUG
      con->sid = 0x0;
#endif
      mpq_clear(con->lhs);
      mpq_clear(con->rhs);
      mpq_clear(con->scale);
      
      free(con->name);
      free(con);
   }
   if (lp->probname != NULL)
      free(lp->probname);
   if (lp->objname != NULL)
      free(lp->objname);
   if (lp->rhsname != NULL)
      free(lp->rhsname);
   if (lp->bndname != NULL)
      free(lp->bndname);
   if (lp->rngname != NULL)
      free(lp->rngname);
   
   free(lp->name);
   free(lp);
}

void lps_number(const Lps* lp)
{
   Var* var;
   Con* con;
   int  i;

   assert(lps_valid(lp));
   
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

Var* lps_getvar(
   const Lps*  lp,
   const char* name)
{
   Var* vr;
   
   assert(lps_valid(lp));
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
   
Con* lps_getcon(
   const Lps*  lp,
   const char* name)
{
   Con* cr;
   
   assert(lps_valid(lp));
   assert(name != NULL);
    
   cr = hash_lookup_con(lp->con_hash, name);

   assert((cr == NULL) || (cr->sid == CON_SID));
   assert((cr == NULL) || (!strcmp(cr->name, name)));

#ifndef NDEBUG
   {
      const Con* con;
      
      for(con = lp->con_root; con != NULL; con = con->next)
         if (!strcmp(con->name, name))
            break;

      assert(con == cr);
   }
#endif
   return cr;   
}
   
Nzo* lps_getnzo(
   const Lps* lp,
   const Con* con,
   const Var* var)
{
   Nzo*   nzo;
   
   assert(lps_valid(lp));
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
   Lps*        lp,
   const char* name)
{
   Var* v;
   
   assert(lps_valid(lp));
   assert(name                 != NULL);
   assert(lps_getvar(lp, name) == NULL);
   
   v = malloc(sizeof(*v));
   
   assert(v != NULL);

   mpq_init(v->cost);
   mpq_init(v->lower);
   mpq_init(v->upper);
   mpq_init(v->value);
   
   v->sid       = VAR_SID;
   v->name      = strdup(name);
   v->number    = lp->vars;
   v->class     = VAR_CON;
   v->type      = VAR_FREE;
   v->state     = VAR_ERR;
   v->priority  = 0;
   v->size      = 0;
   v->first     = NULL;
   v->next      = NULL;   
   v->prev      = lp->var_last;
   lp->var_last = v;

   if (v->prev == NULL)
      lp->var_root = v;
   else
   {
      assert(v->prev->next == NULL);
      
      v->prev->next = v;
   }
   lp->vars++;

   hash_add_var(lp->var_hash, v);

   assert(lps_valid(lp));

   return v;
}

Con* lps_addcon(
   Lps*         lp,
   const char*  name)
{
   Con* c;
   
   assert(lps_valid(lp));
   assert(name                 != NULL);
   assert(lps_getcon(lp, name) == NULL);

   c = malloc(sizeof(*c));
   
   assert(c != NULL);

   mpq_init(c->lhs);
   mpq_init(c->rhs);
   mpq_init(c->scale);
   
   c->sid       = CON_SID;
   c->name      = strdup(name);
   c->number    = lp->cons;   
   c->size      = 0;
   c->type      = CON_FREE;
   c->state     = CON_ERR;
   c->flags     = 0;
   c->first     = NULL;
   c->next      = NULL;
   c->prev      = lp->con_last;
   lp->con_last = c;
   
   if (c->prev == NULL)
      lp->con_root = c;
   else
   {
      assert(c->prev->next == NULL);
      
      c->prev->next = c;
   }
   lp->cons++;

   hash_add_con(lp->con_hash, c);

   assert(lps_valid(lp));

   return c;
}
  
void lps_addnzo(
   Lps*        lp,
   Con*        con,
   Var*        var,
   const mpq_t value)
{
   Nzo* nzo;
   
   assert(lps_valid(lp));
   assert(con         != NULL);
   assert(con->sid    == CON_SID);
   assert(var         != NULL);   
   assert(var->sid    == VAR_SID);

   /* Ins LP aufnehmen
    */
   if (lp->next == NULL)
      lps_storage(lp);
      
   nzo = lp->next;

   assert(nzo != NULL);

   lp->next = nzo->var_next;
   lp->nonzeros++;

   mpq_set(nzo->value, value);
      
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
   assert(lps_valid(lp));
}

void lps_delnzo(
   Lps* lp,
   Nzo* nzo)
{
   assert(lps_valid(lp));
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
   
   assert(lps_valid(lp));
}

void lps_setval(
   Nzo*  nzo,
   const mpq_t value)
{
   assert(nzo != NULL);

   mpq_set(nzo->value, value);
}

void lps_getval(
   const Nzo* nzo,
   mpq_t      value)
{
   assert(nzo != NULL);

   mpq_set(value, nzo->value);
}

void lps_setdir(
   Lps*     lp,
   LpDirect direct)
{
   assert(lps_valid(lp));

   lp->direct = direct;
}

void lps_setprobname(
   Lps*        lp,
   const char* name)
{
   assert(lp   != NULL);
   assert(name != NULL);
   
   if (lp->probname != NULL)
      free(lp->probname);
   
   lp->probname = strdup(name);
}

void lps_setobjname(
   Lps*        lp,
   const char* name)
{
   assert(lp   != NULL);
   assert(name != NULL);
   
   if (lp->objname != NULL)
      free(lp->objname);
   
   lp->objname = strdup(name);
}

void lps_setrhsname(
   Lps*        lp,
   const char* name)
{
   assert(lp   != NULL);
   assert(name != NULL);
   
   if (lp->rhsname != NULL)
      free(lp->rhsname);
   
   lp->rhsname = strdup(name);
}

void lps_setbndname(
   Lps*        lp,
   const char* name)
{
   assert(lp   != NULL);
   assert(name != NULL);
   
   if (lp->bndname != NULL)
      free(lp->bndname);
   
   lp->bndname = strdup(name);
}

void lps_setrngname(
   Lps*        lp,
   const char* name)
{
   assert(lp   != NULL);
   assert(name != NULL);
   
   if (lp->rngname != NULL)
      free(lp->rngname);
   
   lp->rngname = strdup(name);
}

void lps_getcost(
   const Var* var,
   mpq_t      cost)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   
   mpq_set(cost, var->cost);
}

void lps_setcost(
   Var*        var,
   const mpq_t cost)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);

   mpq_set(var->cost, cost);
}

void lps_setlower(
   Var*        var,
   const mpq_t lower)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);

   mpq_set(var->lower, lower);

   /* FREE  -> LOWER
    * LOWER -> LOWER
    * UPPER -> BOXED/FIXED
    * BOXED -> BOXED/FIXED
    * FIXED -> BOXED/FIXED
    */
   if (var->type == VAR_FREE)
      var->type = VAR_LOWER;
   else if (var->type != VAR_LOWER)
   {
      assert(var->type == VAR_UPPER || var->type == VAR_BOXED || var->type == VAR_FIXED);

      var->type = mpq_equal(var->lower, var->upper) ? VAR_FIXED : VAR_BOXED;
   }
}

void lps_setupper(
   Var*        var,
   const mpq_t upper)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   
   mpq_set(var->upper, upper);

   /* FREE  -> LOWER
    * LOWER -> LOWER
    * UPPER -> BOXED/FIXED
    * BOXED -> BOXED/FIXED
    * FIXED -> BOXED/FIXED
    */
   if (var->type == VAR_FREE)
      var->type = VAR_UPPER;
   else if (var->type != VAR_UPPER)
   {
      assert(var->type == VAR_LOWER || var->type == VAR_BOXED || var->type == VAR_FIXED);
      
      var->type = mpq_equal(var->lower, var->upper) ? VAR_FIXED : VAR_BOXED;
   }
}

void lps_setlhs(
   Con*        con,
   const mpq_t lhs)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

   mpq_set(con->lhs, lhs);

   /* FREE  -> LHS
    * LHS   -> LHS
    * RHS   -> RANGE/EQUAL
    * RANGE -> RANGE/EQUAL
    * EQUAL -> RANGE/EQUAL
    */
   if (con->type == CON_FREE)
      con->type = CON_LHS;
   else if (con->type != CON_LHS)
   {
      assert(con->type == CON_RHS || con->type == CON_RANGE || con->type == CON_EQUAL);
      
      con->type = mpq_equal(con->lhs, con->rhs) ? CON_EQUAL : CON_RANGE;
   }
}

void lps_setrhs(
   Con*        con,
   const mpq_t rhs)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

   mpq_set(con->rhs, rhs);

   /* FREE  -> RHS
    * RHS   -> RHS
    * LHS   -> RANGE/EQUAL
    * RANGE -> RANGE/EQUAL
    * EQUAL -> RANGE/EQUAL
    */
   if (con->type == CON_FREE)
      con->type = CON_RHS;
   else if (con->type != CON_RHS)
   {
      assert(con->type == CON_LHS || con->type == CON_RANGE || con->type == CON_EQUAL);
      
      con->type = mpq_equal(con->lhs, con->rhs) ? CON_EQUAL : CON_RANGE;
   }
}

void lps_setcontype( Con* con, ConType type)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

   con->type = type;
}

ConType lps_contype(const Con* con)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

   return con->type;
}

VarType lps_vartype(const Var* var)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);

   return var->type;
}

VarClass lps_class(const Var *var)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);

   return var->class;
}

void lps_setclass(Var *var, VarClass class)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);

   var->class = class;
}

void lps_getlhs(
   const Con* con,
   mpq_t      lhs)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

   mpq_set(lhs, con->lhs);
}

void lps_getrhs(
   const Con* con,
   mpq_t      rhs)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

   mpq_set(rhs, con->rhs);
}

void lps_setvartype(
   Var*    var,
   VarType type)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   
   var->type = type;   
}

VarState lps_varstate(const Var* var)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);

   return var->state;
}

void lps_setvarstate(
   Var*     var,
   VarState state)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   
   var->state = state;   
}

ConState lps_constate(const Con* con)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

   return con->state;
}

void lps_setconstate(
   Con*     con,
   ConState state)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);
   
   con->state = state;   
}

unsigned int lps_flags(const Con* con)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);

   return con->flags;
}

void lps_addflags(
   Con*         con,
   unsigned int flags)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);
   
   con->flags |= flags;   
}

void lps_setscale(
   Con*        con,
   const mpq_t scale)
{
   assert(con      != NULL);
   assert(con->sid == CON_SID);
   
   mpq_set(con->scale, scale);
}

void lps_setpriority(
   Var* var,
   int  priority)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   
   var->priority = priority;
}

void lps_setvalue(
   Var*        var,
   const mpq_t value)
{
   assert(var      != NULL);
   assert(var->sid == VAR_SID);
   
   mpq_set(var->value, value);
}

void lps_stat(const Lps* lp)
{
   assert(lps_valid(lp));

   printf("Name: %s   Variables: %d   Constraints: %d   Non Zeros: %d\n",
      lp->name, lp->vars, lp->cons, lp->nonzeros);
}

void lps_write(
   const Lps*  lp,
   FILE*       fp,
   LpFormat    format,
   const char* text)
{
   assert(lp   != NULL);
   assert(fp   != NULL);
   
   lps_number(lp);

   switch(format)
   {
   case LP_FORM_LPF :
      lpf_write(lp, fp, text);
      break;
   case LP_FORM_MPS :
      mps_write(lp, fp, text);
      break;
   default :
      abort();
   }
}

/* size has to be big enough to store a '#', a '\0'
 * and the var or row number.
 * ??? would be better to store the row number as base 26
 */
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
   assert(size   >  0);
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

void lps_transtable(const Lps* lp, FILE* fp, int size, const char* head)
{
   Var*  var;
   Con*  con;
   char* temp = malloc((size_t)size + 1);
   mpq_t zero;
   
   assert(lps_valid(lp));
   assert(fp   != NULL);
   assert(size >  0);
   assert(head != NULL);
   assert(temp != NULL);

   mpq_init(zero);
   
   lps_number(lp);
   
   for(var = lp->var_root; var != NULL; var = var->next)
   {
      if (var->size == 0 && mpq_equal(var->cost, zero))
         continue;

      lps_makename(temp, size + 1, var->name, var->number);

      fprintf(fp, "%s\tv %7d\t%-*s\t\"%s\"\n",
         head, var->number, size, temp, var->name);
   }
   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if (con->size == 0 && mpq_equal(con->lhs, zero) && mpq_equal(con->rhs, zero))
         continue;

      lps_makename(temp, size + 1, con->name, con->number);
      
      fprintf(fp, "%s\tc %7d\t%-*s\t\"%s\"\t%.16e\n",
         head, con->number, size, temp, con->name, mpq_get_d(con->scale));
   }
   free(temp);
   
   mpq_clear(zero);
}

#if 0
void lps_orderfile(const Lps* lp, FILE* fp)
{
   assert(lps_valid(lp));

   ord_write(lp, fp);
}
#endif

void lps_scale(const Lps* lp)
{
   Con*   con;
   Nzo*   nzo;
   mpq_t  maximum;
   mpq_t  v;

   assert(lps_valid(lp));
      
   mpq_init(maximum);
   mpq_init(v);
   
   for(con = lp->con_root; con != NULL; con = con->next)
   {
      if ((con->flags & LP_FLAG_CON_SCALE) > 0)
      {         
         /* should be abs and I don't know if it is right maximum = con->rhs; */
      
         for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
         {
            mpq_abs(v, nzo->value);
            
            if (mpq_cmp(v, maximum) > 0)
               mpq_set(maximum, v);
         }
         mpq_inv(con->scale, maximum); /* scale = 1 / maximum */

         mpq_mul(con->rhs, con->rhs, con->scale);

         for(nzo = con->first; nzo != NULL; nzo = nzo->con_next)
            mpq_mul(nzo->value, nzo->value, con->scale);
      }
   }
   mpq_clear(v);
   mpq_clear(maximum);
}


/* ------------------------------------------------------------------------- */
/* Emacs Local Variables:                                                    */
/* Emacs mode:c                                                              */
/* Emacs c-basic-offset:3                                                    */
/* Emacs tab-width:8                                                         */
/* Emacs indent-tabs-mode:nil                                                */
/* Emacs End:                                                                */
/* ------------------------------------------------------------------------- */


