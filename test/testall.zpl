# $Id: testall.zpl,v 1.1 2001/05/06 11:43:21 thor Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
#*                                                                           * 
#*   File....: testall.zpl                                                   * 
#*   Name....: ZIMPLE Test Modell   es                                       * 
#*   Author..: Thorsten Koch                                                 * 
#*   Copyright by Author, All rights reserved                                * 
#*                                                                           * 
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
#
#  This modell does nothing usefull but to test a lot of
#  legal language constructs in ZIMPLE.
#  The optimal value 474.
# 
include "testdata.zpl";

set Q := { read "test.txt" as "<1n,2n>" use 16 fs " " };

param f := 5;
#param p[D] := <"Hallo"> 10, <"Tach"> 9, <"Moin"> 8;
param p[D] := read "test.txt" as "<1s> 2n" skip 16 use 3 fs " ";

var x[D] binary;
var y[E] integer >= 2 <= 100;
var z[B] real <= 1000;

var u[<d,a> in DD] real 
   >= if d == "Moin" then p[d] else 0 end
   <= if a > 1 then 20 else 40 end;

maximize win:
   sum <d> in D do 3 * x[d] 
 + sum <e> in E do 1 / 2 * y[e]
 + sum <b> in B with b mod 2 == 1 do if b > 6 then 4 else 5 end * z[b]
 + sum <b> in B with b mod 2 == 0 do -b div 2 * z[b];

subto cons1: 
   forall <b> in B with b > 5 do 
      sum <i> in { <0>, <1>, <2> } do z[b - i] <= 70;

subto cons2:
   sum <d> in D do p[d] * x[d] <= 20;

subto cons3:
   forall <e> in E with e < 5 do y[e] + y[e + 4] <= 200 / e;

subto cons4:
   sum <b> in B do z[b] >= 100;

subto cons5:
   forall <d> in D do sum <d,a> in DD do u[d,a] <= 10 * p[d];
