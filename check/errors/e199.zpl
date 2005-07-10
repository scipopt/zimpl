# $Id: e199.zpl,v 1.1 2005/07/10 10:21:24 bzfkocht Exp $
set I   := {1 .. 5};
var x[I] binary;
sos c2: sum <i> in I : i * x[i] + 3 == type1 priority 100;
