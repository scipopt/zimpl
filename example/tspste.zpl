# $Id: tspste.zpl,v 1.4 2006/05/18 19:41:07 bzfkocht Exp $
#
# Generic formulation of the Travelling Salesmen Problem
#
set V   := { read "city.dat" as "<1s>" comment "#" use cities };
set E   := { <i,j> in V * V with i < j };
set P[] := powerset(V \ { ord(V,1,1) });
set K   := indexset(P) \ { 0 };

param px[V] := read "city.dat" as "<1s> 2n" comment "#" use cities;
param py[V] := read "city.dat" as "<1s> 3n" comment "#" use cities;

defnumb dist(a,b) := sqrt((px[a] - px[b])^2 + (py[a] - py[b])^2);

var x[E] binary;

minimize cost: sum <i,j> in E : dist(i,j) * x[i, j];

subto two_connected:
   forall <v> in V do
      (sum <v,j> in E : x[v,j]) + (sum <i,v> in E : x[i,v]) == 2;

subto no_subtour:
   forall <k> in K with card(P[k]) > 2 and card(P[k]) < card(V) - 2 do
      sum <i,j> in E with <i> in P[k] and <j> in P[k] : x[i,j] 
      <= card(P[k]) - 1;





