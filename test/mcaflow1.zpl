# Multi-Commodity-Arborescence-Flow (see Diplomarbeit Mathematik, p. 24)

# nodes
set Vtarget := { "v001", "v002", "v003" };
set V       := { "root", "v001", "v002", "v003" };
# arcs
set A       := V * Vtarget;

# node demands
param demand[Vtarget] :=
<"v001"> 10,
<"v002"> 20,
<"v003"> 30;

# arc capacities
param capacity[A] :=
<"root","v001"> 30,
<"root","v002"> 20,
<"root","v003"> 60,
<"v001","v001">  0,
<"v001","v002"> 20,
<"v001","v003">  0,
<"v002","v001"> 10,
<"v002","v002">  0,
<"v002","v003"> 30,
<"v003","v001"> 30,
<"v003","v002"> 30,
<"v003","v003">  0;

# arc costs
param cost[A] :=
<"root","v001"> 5,
<"root","v002"> 8,
<"root","v003"> 7,
<"v001","v001"> 0,
<"v001","v002"> 4,
<"v001","v003"> 0,
<"v002","v001"> 3,
<"v002","v002"> 0,
<"v002","v003"> 6,
<"v003","v001"> 9,
<"v003","v002"> 6,
<"v003","v003"> 0;

# variable: x[a] in {0,1}, x[a] == 1 <=> arc 'a' is chosen in network
var x[A] binary;

# variable: f[a,v] in {0,1}, f[a,v] == 1 <=> demand of node 'v' is routed through arc 'a'
var f[A * Vtarget] binary;


# Objective: minimize costs for fullfilling all demands
minimize total_costs: sum <u,v> in A : cost[u,v] * x[u,v];

# single input constraints
subto singleinput: forall <v> in Vtarget do
                     sum <u> in V : x[u,v] == 1;

# flow balance constraints
subto flowbalance: forall <u,v> in Vtarget * Vtarget do
                     sum <s> in V       : f[s,v,u]
                   - sum <t> in Vtarget : f[v,t,u]  == if u == v then 1 else 0 end;

# capacity constraints
subto capacitylim: forall <s,t> in A do
                     sum <v> in Vtarget : demand[v] * f[s,t,v] - capacity[s,t] * x[s,t] <= 0;

# variable link
subto variabllink: forall <s,t,v> in A * Vtarget do
                      f[s,t,v] - x[s,t] <= 0;

