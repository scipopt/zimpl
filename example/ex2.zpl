# $Id: ex2.zpl,v 1.2 2003/09/28 13:56:51 bzfkocht Exp $
#
# R. Fourer, D.M. Gay, B.W. Kernighan: 
# AMPL A Modelling Language for Mathematical Programming
#
# Chapter 1, Page 11
#
set PROD           := { "bands", "coils" };

param rate  [PROD] := <"bands">  200, <"coils">  140;
param profit[PROD] := <"bands">   25, <"coils">   30;
param market[PROD] := <"bands"> 6000, <"coils"> 4000;
param avail        := 40;

var Make[<p> in PROD] >= 0 <= market[p];

maximize Total_Profit: sum <p> in PROD : profit[p] * Make[p];

subto Time: sum <p> in PROD : (1/rate[p]) * Make[p] <= avail;
