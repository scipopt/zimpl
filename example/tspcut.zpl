# $Id: tspcut.zpl,v 1.2 2006/03/26 10:23:25 bzfkocht Exp $
#
# Generic cut formulation of the Travelling Salesmen Problem
#
set V   := { read "city.dat" as "<1s>" comment "#" use cities };
set E   := { <i,j> in V * V with i < j };
set P[] := powerset(V);
set K   := indexset(P) \ { 0 };

param px[V] := read "city.dat" as "<1s> 2n" comment "#" use cities;
param py[V] := read "city.dat" as "<1s> 3n" comment "#" use cities;

defnumb dist(a,b) := sqrt((px[a] - px[b])^2 + (py[a] - py[b])^2);

var x[E] binary;

minimize cost: sum <i,j> in E : dist(i,j) * x[i, j];

subto cut:
   forall <k> in K with card(P[k]) <= card(V) / 2 do
      sum <i,j> in E with <i> in P[k] and <j> in V \ P[k] 
         or <i> in V \ P[k] and <j> in P[k] : x[i,j] >= 2;






