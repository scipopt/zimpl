# $Id: chvatal_diet.zpl,v 1.1 2003/09/28 13:56:51 bzfkocht Exp $
#
# From V. Chvatal: Linear Programming
# Chapter 1, Page 3ff.
#
# A diet problem
#
set Food := { "Oatmeal", "Chicken", "Eggs", "Milk", "Pie", "Pork" };
set Attr := { "Servings", "Energy", "Protein", "Calcium", "Price" };

param min_energy  := 2000; # kcal
param min_protein := 55;   # g
param min_calcium := 800;  # mg

param nutrition[Food * Attr] := 
           | "Servings", "Energy", "Protein", "Calcium", "Price" |
|"Oatmeal" |         4 ,     110 ,        4 ,        2 ,      3  |
|"Chicken" |         3 ,     205 ,       32 ,       12 ,     24  |
|"Eggs"    |         2 ,     160 ,       13 ,       54 ,     13  |
|"Milk"    |         8 ,     160 ,        8 ,      284 ,      9  |
|"Pie"     |         2 ,     420 ,        4 ,       22 ,     20  |
|"Pork"    |         2 ,     260 ,       14 ,       80 ,     19  |;
#                          (kcal)        (g)        (mg)  (cents)       

var x[<f> in Food] integer >= 0 <= nutrition[f, "Servings"];

minimize cost: sum <f> in Food : nutrition[f, "Price"] * x[f];

subto energy : 
   sum <f> in Food : nutrition[f, "Energy" ] * x[f] >= min_energy;
subto protein: 
   sum <f> in Food : nutrition[f, "Protein"] * x[f] >= min_protein;
subto calcium: 
   sum <f> in Food : nutrition[f, "Calcium"] * x[f] >= min_calcium;



