/* $Id: zcplex.c,v 1.2 2007/03/07 12:26:29 bzfkocht Exp $ */
/*
   gcc -g -O -Wall -Wno-unknown-pragmas -I../src -I /opt/ilog/cplex100/include/ilcplex/ zcplex.c -L ../lib -lzimpl-2.06b.linux.x86.gnu.dbg -L /opt/ilog/cplex100/lib/x86_RHEL3.0_3.2/static_pic -lcplex -lpthread -lgmp -lm -lz
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <cplex.h>

#include "bool.h"
#include "mshell.h"
#include "ratlptypes.h"
#include "mme.h"
#include "zimpllib.h"

static CPXENVptr env          = NULL;
static CPXLPptr  lp           = NULL;
static Bool      use_startval = FALSE;
static int       cols   = 0;
static char**    cname  = NULL;
static char*     cstore = NULL;
static int       solno  = 0;

static char*     rownam = NULL;
static double    rowrhs = 0.0;
static double    rowrng = 0.0;
static char      rowsen = ' ';
static int       rngind = 0;
static int       rowcnt = 0;
static int       rowmax = 0;
static int*      rowidx = NULL;
static double*   rowval = NULL;
static int       rowno  = -1;

static void xxx_check_status(int status, const char* filename, int lineno)
{
   char errstrg[1024];
   
   if (status)
   {
      CPXgeterrorstring(env, status, errstrg);

      fprintf(stderr, "%s(%d) CPLEX returned status %d: %s\n",
         filename, lineno, status, errstrg);

      abort();
   }
}

#define check_status(s)  xxx_check_status((s), __FILE__, __LINE__)

static void flush_row()
{
   int rmatbeg[2];

   assert(rownam != NULL);
   assert(rowcnt <= rowmax);
   assert(rowidx != NULL);
   assert(rowval != NULL);

   if (rowcnt > 0)
   {
      rmatbeg[0] = 0;
      rmatbeg[1] = rowcnt;
   
      check_status(CPXaddrows(env, lp, 0, 1, rowcnt, &rowrhs, &rowsen, rmatbeg,
                      rowidx, rowval, NULL, &rownam));

      /* get ranges right ??? */
      
      free(rownam);

      rownam = NULL;
      rowno  = -1;
      rowcnt = 0;
      rowsen= ' ';
   }
}
   
static void cplex_init()
{
   int status;
   
   assert(env == NULL);
   assert(lp  == NULL);

   env = CPXopenCPLEX(&status);

   check_status(status);
   
   assert(env != NULL);

   check_status(CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON));
}

static void cplex_exit()
{
   assert(env != NULL);

   if (lp != NULL)
      check_status(CPXfreeprob(env, &lp));

   check_status(CPXcloseCPLEX(&env));

   env = NULL;
   lp  = NULL;
}

void xlp_alloc(const char* name, Bool need_startval)
{
   int status;
   
   assert(name != NULL);
   
   lp = CPXcreateprob(env, &status, name);

   check_status(status);
   
   assert(lp != NULL);

   conname_format(CON_FORM_FULL);

   use_startval = need_startval;

   rowmax = 1000;
   rowidx = malloc(rowmax * sizeof(*rowidx));
   rowval = malloc(rowmax * sizeof(*rowval));

   assert(rowidx != NULL);
   assert(rowval != NULL);
}

void xlp_free()
{
   if (rownam != NULL && rowcnt > 0)
      flush_row();

   if (rowidx != NULL)
      free(rowidx);
   if (rowval != NULL)
      free(rowval);
   if (rownam != NULL)
      free(rownam);
   
   rownam = NULL;
   rowidx = NULL;
   rowval = NULL;
   rowno  = -1;
   rowcnt = 0;
   rowmax = 0;
   rowsen= ' ';
}

Bool xlp_conname_exists(const char* conname)
{
   int index_p;
   int status;
   
   assert(conname != NULL);

   status = CPXgetrowindex(env, lp, conname, &index_p);
   
   return status == 0;
}

Con* xlp_addcon(
   const char*  name,
   ConType      contype,
   const Numb*  lhs,
   const Numb*  rhs,
   unsigned int flags)
{
   int    con_no;
   
   assert(name != NULL);
   assert(lhs  != NULL);
   assert(rhs  != NULL);

   if (rownam != NULL && strcmp(name, rownam))
      flush_row();

   rownam = strdup(name);
   
   switch(contype)
   {
   case CON_RHS :
      rowsen = 'L';
      rowrhs = numb_todbl(rhs);
      break;
   case CON_LHS :
      rowsen = 'G';
      rowrhs = numb_todbl(lhs);
      break;
   case CON_EQUAL : 
      rowsen = 'E';
      rowrhs = numb_todbl(rhs);
      break;
   case CON_RANGE :
      fprintf(stderr, "RANGES not yet supported\n");
      abort();
      rowsen = 'R';
      rowrhs = numb_todbl(lhs);
      rowrng = numb_todbl(rhs) - rowrhs;
      rngind = 1;
      break;
   default :
      abort();
   }
   con_no = CPXgetnumrows(env, lp); 

   rowno = con_no + 1;
   
   return (Con*)rowno;
}

Var* xlp_addvar(
   const char*  name,
   VarClass     varclass,
   const Bound* lower,
   const Bound* upper,
   const Numb*  priority,
   const Numb*  startval)
{
   double obj   [1];
   double lb    [1];
   double ub    [1];
   char   xctype[1];
   char*  cnames[1];
   int    var_no;
   
   assert(name     != NULL);
   assert(lower    != NULL);
   assert(upper    != NULL);
   assert(priority != NULL);
   assert(startval != NULL);

   cnames[0] = name;
   obj   [0] = 0.0;
   
   switch(varclass)
   {
   case VAR_CON :
      xctype[0] = 'C';
      break;
   case VAR_BIN :
      xctype[0] = 'B';
      break;
   case VAR_INT :
      xctype[0] = 'I';
      break;
   default :
      abort();
   }
   switch(bound_get_type(lower))
   {
   case BOUND_VALUE :
      lb[0] = numb_todbl(bound_get_value(lower));
      break;
   case BOUND_MINUS_INFTY :
      lb[0] = -CPX_INFBOUND;
      break;
   case BOUND_INFTY :
   default :
      abort();
   }
   switch(bound_get_type(upper))
   {
   case BOUND_VALUE :
      ub[0] = numb_todbl(bound_get_value(upper));
      break;
   case BOUND_INFTY :
      ub[0] = CPX_INFBOUND;
      break;
   case BOUND_MINUS_INFTY :
   default :
      abort();
   }

#if 0
   numb_get_mpq(priority, temp);
   
   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   if (mpz_get_si(mpq_denref(temp)) != 1)
      if (verbose > VERB_QUIET)
         fprintf(stderr, "*** Warning variable priority has to be integral\n");

   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   lps_setpriority(var, (int)mpz_get_si(mpq_numref(temp)));
#endif
   
   check_status(CPXnewcols(env, lp, 1, obj, lb, ub, xctype, cnames));
   
   var_no = CPXgetnumcols(env, lp);

   if (use_startval)
   {
      int    indices[1];
      double values[1];
      
      indices[0] = var_no - 1;
      values [0] = numb_todbl(startval);

      check_status(CPXchgmipstart(env, lp, 1, indices, values));
   }   
   return (Var*)var_no;
}

Sos* xlp_addsos(
   const char*  name,
   SosType      type,
   const Numb*  priority)
{
   fprintf(stderr, "SOS not yet supported\n");
   abort();
   
#if 0
   Sos*   sos;
   mpq_t  temp;
   
   assert(name     != NULL);
   assert(priority != NULL);

   mpq_init(temp);

   numb_get_mpq(priority, temp);
   
   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   if (mpz_get_si(mpq_denref(temp)) != 1)
      if (verbose > VERB_QUIET)
         fprintf(stderr, "*** Warning SOS priority has to be integral\n");

   /*lint -e{663} supress "Suspicious array to pointer conversion" */
   sos = lps_addsos(lp, name, type, (int)mpz_get_si(mpq_numref(temp)));

   mpq_clear(temp);
   
   return sos;
   int CPXPUBLIC CPXaddsos(CPXCENVptr env, CPXLPptr lp, int numsos, int numsosnz, const char * sostype, const int * sospri, const int * sosbeg, const int * sosind, const double * soswt);
#endif
   return NULL;
}

void xlp_addtosos(Sos* sos, Var* var, const Numb* weight)
{
#if 0
   mpq_t temp;
   
   assert(weight != NULL);
   assert(var    != NULL);
   assert(sos    != NULL);

   mpq_init(temp);

   numb_get_mpq(weight, temp);
   
   lps_addsse(sos, var, temp);

   mpq_clear(temp);
#endif
}

VarClass xlp_getclass(const Var* var)
{
   char     ctype[1];
   VarClass vclass;
   
   assert(var != NULL);

   check_status(CPXgetctype(env, lp, ctype, (int)var - 1, (int)var - 1));

   switch(ctype[0])
   {
   case 'C' :
      vclass = VAR_CON;
      break;
   case 'B' :
      vclass = VAR_BIN;
      break;
   case 'I' :
      vclass = VAR_INT;
      break;
   default :
      abort();
   }
   return vclass;
}

Bound* xlp_getlower(const Var* var)
{
   Bound* bound;
   Numb*  numb;
   double lb[1];
   
   assert(var != NULL);

   check_status(CPXgetlb(env, lp, lb, (int)var - 1, (int)var - 1));

   if (lb[0] <= -CPX_INFBOUND)
      bound = bound_new(BOUND_MINUS_INFTY, numb_zero());
   else
   {
      char temp[64];
      
      sprintf(temp, "%.16f", lb[0]);
      numb  = numb_new_ascii(temp);
      bound = bound_new(BOUND_VALUE, numb);
      numb_free(numb);
   }
   return bound;
}
     
Bound* xlp_getupper(const Var* var)
{
   Bound* bound;
   Numb*  numb;
   double ub[1];
   
   assert(var != NULL);

   check_status(CPXgetub(env, lp, ub, (int)var - 1, (int)var - 1));

   if (ub[0] >= CPX_INFBOUND)
      bound = bound_new(BOUND_INFTY, numb_zero());
   else
   {
      char temp[64];
      
      sprintf(temp, "%.16f", ub[0]);
      numb  = numb_new_ascii(temp);
      bound = bound_new(BOUND_VALUE, numb);
      numb_free(numb);
   }
   return bound;
}

void xlp_objname(const char* name)
{
   assert(name != NULL);

   check_status(CPXcopyobjname(env, lp, name));
}

void xlp_setdir(Bool minimize)
{
   CPXchgobjsen(env, lp, minimize ? CPX_MIN : CPX_MAX);
}

void xlp_addtonzo(Var* var, Con* con, const Numb* value)
{
   int i;

   assert(var   != NULL);
   assert(con   != NULL);
   assert(value != NULL);

   if ((int)con != rowno)
   {
      fprintf(stderr, "Uhhhooooooooo!\n");
      abort();
   }
   if (rowcnt == rowmax)
   {
      rowmax += 10000;
      rowidx  = realloc(rowidx, rowmax * sizeof(*rowidx));
      rowval  = realloc(rowval, rowmax * sizeof(*rowval));

      assert(rowidx != NULL);
      assert(rowval != NULL);
   }
   assert(rowcnt < rowmax);

   for(i = 0; i < rowcnt; i++)
      if (rowidx[i] == (int)var - 1)
         break;

   if (i < rowcnt)
      rowval[i] += numb_todbl(value);
   else
   {
      assert(i == rowcnt);
      
      rowidx[rowcnt] = (int)var - 1;
      rowval[rowcnt] = numb_todbl(value);
      rowcnt++;
   }
}

void xlp_addtocost(Var* var, const Numb* cost)
{
   double  obj    [1];
   double  values [1];
   int     indices[1];
      
   assert(var  != NULL);
   assert(cost != NULL);

   
   check_status(CPXgetobj(env, lp, obj, (int)var - 1, (int)var - 1));

   indices[0] = (int)var - 1;
   values [0] = obj[0] + numb_todbl(cost);
   
   check_status(CPXchgobj(env, lp, 1, indices, values));
}

Bool xlp_concheck(const Con* con)
{
#if 0
   Bool  ok = TRUE;
   mpq_t lhs;
   mpq_t rhs;
   mpq_t sum;

   if (!use_startval)
      return TRUE;
   
   mpq_init(sum);
   mpq_init(lhs);
   mpq_init(rhs);

   /* Get sum of coefficients time startval if usable
    * (no continous variables)
    */
   if (lps_con_sumup(con, sum))
   {
      lps_getlhs(con, lhs);
      lps_getrhs(con, rhs);

      switch(lps_contype(con))
      {
      case CON_RHS :
         if (mpq_cmp(sum, rhs) > 0 && stmt_trigger_warning(215))
         {
            ok = FALSE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) <= RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(rhs));
         }
         break;
      case CON_LHS :
         if (mpq_cmp(lhs, sum) > 0 && stmt_trigger_warning(215))
         {
            ok = FALSE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) >= RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(lhs));
         }
         break;
      case CON_EQUAL : /* In case of EQUAL, both should be equal */
         if ((!mpq_equal(lhs, sum) || !mpq_equal(rhs, sum)) && stmt_trigger_warning(215))
         {
            ok = FALSE;
            fprintf(stderr, "--- Warning 215: Startvals violate constraint, sum of LHS (%g) == RHS (%g)\n",
               mpq_get_d(sum), mpq_get_d(rhs));
         }
         break;
      case CON_RANGE :
         if ((mpq_cmp(sum, rhs) > 0 || mpq_cmp(lhs, sum) > 0) && stmt_trigger_warning(215))
         {
            ok = FALSE;
            fprintf(stderr, "--- Warning 215: Startvals violate range, LHS (%g) <= Sum (%g) <= RHS (%g)\n",
               mpq_get_d(lhs), mpq_get_d(sum), mpq_get_d(rhs));
         }
         break;
      default :
         abort();
      }
   }
   mpq_clear(sum);
   mpq_clear(rhs);
   mpq_clear(lhs);

   return ok;
#endif
   return TRUE;
}

static void write_solution(FILE* fp, double objval, int columns, double* x, char** colname)
{
   int i;
   
   fprintf(fp, "Objective: %f\n", objval);

   for(i = 0; i < columns; i++)
   {
      /* Only show variables unequal zero
       */
      if (fabs(x[i]) > 1e-12)
      {
         /* Only show non-internal variables
          */
         if (*cname[i] != '@')
            fprintf(fp, "%s %f\n", colname[i], x[i]);
      }
   }   
}

int my_incumbent_cb(
   CPXCENVptr p_env,
   void*      cbdata,
   int        wherefrom,
   void*      cbhandle,
   double     objval,
   double*    x,
   int*       isfeas_p,
   int*       useraction_p)
{
   char  filename[1024];
   FILE* fp;
   
   assert(p_env        != NULL);
   assert(x            != NULL);
   assert(isfeas_p     != NULL);
   assert(useraction_p != NULL);

   sprintf(filename, "solution.%05d.txt", ++solno);
   
   if (NULL == (fp = fopen(filename, "w")))
      perror(filename);
   else
   {
      write_solution(fp, objval, cols, x, cname);
      
      fclose(fp);
   }
   *isfeas_p     = 1;
   *useraction_p =  CPX_CALLBACK_SET;

   return 0;
}

void cplex_optimize()
{
   char    statstrg[1024];
   double* x;
   int     cspace;
   int     surplus;
   int     statind;
   double  objval;
   
   cols   = CPXgetnumcols(env, lp);
   x      = calloc(cols, sizeof(*x));
   cname  = calloc(cols, sizeof(*cname));
   cspace = cols * 256;
   cstore = calloc(cspace, sizeof(*cstore));

   assert(x      != NULL);
   assert(cname  != NULL);
   assert(cstore != NULL);
   
   check_status(CPXgetcolname(env, lp, cname, cstore, cspace, &surplus, 0, cols - 1));
   check_status(CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON));
   check_status(CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF));
   check_status(CPXsetincumbentcallbackfunc(env, my_incumbent_cb, NULL));

   if (use_startval)
      check_status(CPXsetintparam(env, CPX_PARAM_ADVIND, 1));

   check_status(CPXwriteprob(env, lp, "zcplex.lp", NULL));

   check_status(CPXmipopt(env, lp));

   statind = CPXgetstat(env, lp);

   printf("# Status: %d %s\n", statind, CPXgetstatstring(env, statind, statstrg));
   
   (void)CPXgetobjval(env, lp, &objval);
   
   (void)CPXgetx(env, lp, x, 0, cols - 1);

   write_solution(stdout, objval, cols, x, cname);

   free(cstore);
   free(cname);
   free(x);
}

int main(
   int    argc,
   char** argv)
{
   verbose = VERB_VERBOSE;
   
   cplex_init();
   
   if (zpl_read_with_args(argc, argv))
   {
      cplex_optimize();
   }
   cplex_exit();
   
   return 0;
}



