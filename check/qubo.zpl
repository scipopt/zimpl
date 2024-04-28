set I := { 1 .. 7 };

var x[I] binary;

minimize cost: 17 * x[1];

subto c1: sum <i> in { 2 .. 4 } do x[i] == 1, qubo, penalty4;
subto c2: x[1] == 1, qubo, penalty2;
subto c4: x[5] + x[6] + x[6] == 2, qubo, penalty1;
subto c5: x[5] + x[6] + x[7] >= 1, qubo, penalty1;
subto c6: 2 * x[5] + 3 * x[6] + x[7] == 1, qubo, penalty1;


