# $Id: queens5.zpl,v 1.1 2004/12/26 10:27:48 bzfkocht Exp $
#
# This is a formulation of the n queens problem using binary variables.
# variables. Since the number of queens is maximized, the size of the
# board can be set arbitrarily.
#
# set packing formulation
#
param columns := 8;

set I   := { 1 .. columns };
set IxI := I * I;

var x[IxI] binary;

maximize queens: sum <i,j> in IxI : x[i,j];

# one in a row 
subto row: forall <i> in I do
   sum <i,j> in IxI : x[i,j] <= 1;

# one in a column
subto col: forall <j> in I do
   sum <i,j> in IxI : x[i,j] <= 1;

# rowwise down
subto row: forall <i> in I do
   sum <m,n> in IxI with m - i == n - 1: x[m,n] <= 1;
      
# rowwise up
subto row: forall <i> in I do
   sum <m,n> in IxI with m - i == 1 - n: x[m,n] <= 1;
      
# colwise down
subto row: forall <j> in I do
   sum <m,n> in IxI with m - 1 == n - j: x[m,n] <= 1;

# colwise up
subto row: forall <j> in I do
   sum <m,n> in IxI with 1 - m == n - j: x[m,n] <= 1;


