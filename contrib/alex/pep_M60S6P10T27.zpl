# Mitarbeiter
set M := { 1 to 60 };

# Skills
set S := { 1 to 6 };

# Planungszeitraum
set T := { 1 to 27 };

# Projekte
set P := { 1 to 10 };

# Skillprofile/kombinationen
# set K := {<1,3,1>, ...};
include "K_M60S2";

# Skill Assignments
# set A := {<1,3>, <1,1>, ...};
include "A_M60S6";

# Projektphasen
# set F := {<1,2,3>, <2,1,2>, <2,3,4>};
include "F_P10L8";

# Planungseinheiten pro Periode 
# param u[T] := <1> 12, <2> 12, ...;
include "u_T27";

# Demands
# param b[F*S] := <1,2,3,1> 1, ...;
include "b_P10S6B16";

# Variablen Indizes
set I := A * P * T;

# Zielfunktion
# param c[I] := <1,3,1,1> 11, ...;
include "c_M60S6P10T27";

# Variablen
var x[I] integer;

# Zielfunktion
minimize cost: sum <i,s,p,t> in A * P * T do  c[i,s,p,t] * x[i,s,p,t];

# Nebenbedingungen

# Jeder Mitarbeiter darf zeitgleich max. u[t] Projekteinheiten durchfuehren
subto limit: forall <i,t> in M * T do
   sum <j,s,p> in A * P with j == i do  x[j,s,p,t] <= u[t];
subto fix: forall <i,s,p,t> in A * P * T with t < (min <p,a,e> in F : a) or t > (max <p,a,e> in F : e) do
   x[i,s,p,t] == 0;
# Jeder Mitarbeiter soll zu 80% in seinem bevorzugten Skill eingesetzt sein
subto einsatz: forall <i,s1,s2> in K do
   sum <p,t> in P * T do (x[i,s1,p,t] -  4 * x[i,s2,p,t]) >= 0; 

# Die Auslastung der Mitarbeiter gleichen Skillprofils soll um max. 20% schwanken
subto auslastord: forall <i,r1,r2,j,s1,s2> in K * K with i < j and r1 == s1 and r2 == s2 do 
   sum <p,t> in P * T do (x[i,r1,p,t] + x[i,r2,p,t] - x[j,s1,p,t] - x[j,s2,p,t]) <= 0;
subto auslastmax: forall <i,r1,r2,j,s1,s2> in K * K with r1 == s1 and r2 == s2 and i < j and i == (min <l,r1,r2> in K : l) and j == (max <k,s1,s2> in K : k) do 
   sum <p,t> in P * T do 
     (x[j,s1,p,t] + x[j,s2,p,t] - 1.2 * x[i,r1,p,t] - 1.2 * x[i,r2,p,t]) <= 0;

# Der Bedarf an Skillprofil s muss fuer jede Projektphase gedeckt sein
subto bedarf: forall <p,a,e,s> in F * S do
   sum <i,t> in M * T with t >= a and t <= e and <i,s> in A do 
      x[i,s,p,t] == b[p,a,e,s];




