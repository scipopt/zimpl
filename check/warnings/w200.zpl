# $Id: w200.zpl,v 1.2 2006/01/19 20:53:06 bzfkocht Exp $
var x[{1 .. 2}] binary;
sos c1: type1 priority 100 : 5 * x[1] + 5 * x[2];
