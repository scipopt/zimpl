# $Id: ex1.zpl,v 1.2 2003/09/28 13:56:51 bzfkocht Exp $
#
# R. Fourer, D.M. Gay, B.W. Kernighan: 
# AMPL A Modelling Language for Mathematical Programming
#
# Chapter 1, Page 5
#
var XB;
var XC;
maximize profit: 25 * XB + 30 * XC;
subto Time   : (1/200) * XB + (1/140) * XC <= 40;
subto B_limit: 0 <= XB <= 6000; 
subto C_limit: 0 <= XC <= 4000; 
