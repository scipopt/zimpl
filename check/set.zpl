#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#*                                                                           */
#*   File....: set.zpl                                                       */
#*   Name....: Set test                                                      */
#*   Author..: Thorsten Koch                                                 */
#*   Copyright by Author, All rights reserved                                */
#*                                                                           */
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#*
#* Copyright (C) 2001 by Thorsten Koch <koch@zib.de>
#* 
#* This program is free software; you can redistribute it and/or
#* modify it under the terms of the GNU General Public License
#* as published by the Free Software Foundation; either version 2
#* of the License, or (at your option) any later version.
#* 
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#* 
#* You should have received a copy of the GNU General Public License
#* along with this program; if not, write to the Free Software
#* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#*
set A := { "a", "b", "c" };
set B := { 1, 2, 3 };
set C := { 1 to 4 };
set D := { 7 .. 8 by 2 };
set E := (B + C) * A;
set F := B inter C;
set G := { 1 .. 5 } union D without B;
set H := C symdiff { <1>, <2>, <7>, <8> };
set J := { <g> in G with g > 3 }; 
set K := { <"a", 1>, <"b", 7>, <"c", 9> };
set L[B] := <1> { "a", "b" },
            <2> { "c", "d", "e" },
            <3> { "f" };
set M[<i> in D] := { <3 * i, 7, "d">, <i + 2, 9, "c"> };
set N[<i,j> in C * B with i > j] := { i + j, i - j, i * j };
set P[] := powerset(C);
set Q[] := subsets(C, 3);
set R   := indexset(P);
set S   := R;
set T := { 1 to 9 } * { 10 to 19 } * { "A", "B" };
set U := proj(T, <3,1>);
set V := { <a,2> in A*B with a == "a" or a == "b" };
set W[<i> in B] := { <c> in C with c <= i };
set X := { <i> in { <k> in C with k > 2 } with i mod 2 == 0 }; # { 4 }

var a[A];
var b[B];
var c[C];
var d[D];
var e[E];
var f[F];
var g[G];
var h[H];
var j[J];
var k[K];
var l[L[1] + L[2] + L[3]];
var m[M[7]];
var n[{ 1 .. 100 }];
var p[C];
var q[C];
var u[U];
var v[V];
var xabcdefghijklmno[X];

subto a1: sum <i> in A : a[i] >= 0;
subto b1: sum <i> in B : b[i] >= 0;
subto c1: sum <i> in C : c[i] >= 0;
subto d1: sum <i> in D : d[i] >= 0;
subto e1: sum <i1,i2> in E : e[i1,i2] >= 0;
subto f1: sum <i> in F : f[i] >= 0;
subto g1: sum <i> in G : g[i] >= 0;
subto h1: sum <i> in H : h[i] >= 0;
subto j1: sum <i> in J : j[i] >= 0;
subto k1: sum <i1,i2> in K : k[i1,i2] >= 0;
subto l1: forall <i1> in indexset(L) do
   sum <i2> in L[i1] : l[i2] >= 0;
subto m1: sum <i1,i2,i3> in M[7] : m[i1,i2,i3] >= 0;
subto n1: forall <i1,i2> in indexset(N) do
   sum <i3> in N[i1,i2] : n[i3] >= 0;
subto p1: forall <i1> in R do
   sum <i2> in P[i1] : p[i2] >= 0;
subto q1: forall <i1> in indexset(Q) do
   sum <i2> in Q[i1] : q[i2] >= 0;
subto u1: forall <i1,i2> in U do u[i1,i2] >= 0;
subto v1: sum <i1,i2> in V : v[i1,i2] >= 0;
subto w1: forall <i> in B do
   sum <w> in W[i]: n[w] >= 0; 
subto x1: sum <i> in X : xabcdefghijklmno[i] >= 0;










