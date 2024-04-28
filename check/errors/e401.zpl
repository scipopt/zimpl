set I := { 1 .. 30 };
var x[I] binary;
subto c1: sum <i> in I : 2^i * x[i] >= 1, qubo;

