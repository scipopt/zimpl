# $Id: w200.zpl,v 1.1 2005/07/10 10:21:24 bzfkocht Exp $
var x[{1 .. 2}] binary;
sos c1: 5 * x[1] + 5 * x[2] == type1 priority 100;
