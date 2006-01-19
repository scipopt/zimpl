# $Id: e199.zpl,v 1.2 2006/01/19 20:53:06 bzfkocht Exp $
set I   := {1 .. 5};
var x[I] binary;
sos c2: type1 priority 100 : sum <i> in I : i * x[i] + 3;
