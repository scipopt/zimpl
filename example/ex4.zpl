# Page 15

# products
set PROD  := { "bands", "coils", "plate" };

# stages
set STAGE := { "reheat", "roll" };

# tons per hour in each stage
param rate[PROD * STAGE] := 
<"bands","reheat">  200, 
<"bands","roll">    200,
<"coils","reheat">  200,
<"coils","roll">    140,
<"plate","reheat">  200,
<"plate","roll">    160;

# profit per ton
param profit[PROD] := <"bands">   25, <"coils">   30, <"plate">   29;

# lower limit on tons sold in week
param commit[PROD] := <"bands"> 1000, <"coils">  500, <"plate">  750;

# upper limit on tons sold in week
param market[PROD] := <"bands"> 6000, <"coils"> 4000, <"plate"> 3500;

# hours available/week in each stage
param avail[STAGE] := <"reheat"> 35, <"roll"> 40;

# tons produced
var Make[<p> in PROD] real >= commit[p] <= market[p]; # tons produced

# Objective: total profits from all products
maximize total_profit: sum <p> in PROD : profit[p] * Make[p];

# In each stage: total of hours usd by all
# products may not exceed hour available
subto Time: forall <s> in STAGE : 
   sum <p> in PROD : (1/rate[p,s]) * Make[p] <= sum <p> in PROD : profit[p];
 

