# $Id: tsp1.zpl,v 1.1 2003/09/03 07:54:19 bzfkocht Exp $
set V := { 1 .. 5 };
set E := { <i,j> in V * V with i < j };

param px[V] := <1> 1, <2> 3, <3> 2, <4> 1, <5> 3;
param py[V] := <1> 1, <2> 1, <3> 2, <4> 3, <5> 3; 

param c[<i,j> in E] := px[i]^2 + py[j]^2;

var x[E] binary;

minimize cost: sum <i,j> in E : c[i,j] * x[i, j];

set P[] := powerset(V);
set K   := indexset(P);

#subto two_con:
#   forall <v> in V do
#      sum <i,j> in E with i == v or j == v : x[i,j] == 2;

subto two_con:
   forall <v> in V do
      sum <v,j> in E : x[v,j] + sum <i,v> in E : x[i,v] == 2;

subto subtour:
   forall <k> in K with card(P[k]) > 2 and P[k] != V do
     sum <i,j> in E with <i> in P[k] and <j> in P[k] : 
       x[i,j] <= card(P[k]) - 1;
