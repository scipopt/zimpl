# $Id: define.zpl,v 1.1 2003/09/18 11:55:49 bzfkocht Exp $
set I := { 1..5 } * { 4..7};
var x[I];

defnumb dist(a,b) := a*a + b*b;

subto c1: sum <i,j> in I do dist(i,j) * x[i,j] >= 0;

defnumb ack(i,j) := 
   if i == 0 then j + 1 
             else if j == 0 then ack(i - 1, 1)
                            else ack(i - 1, ack(i, j - 1))
                  end
   end;

subto c2: ack(3,3) * x[ack(1,3),ack(2,2)] >= 0;            

set K := { 1..10 };
var y[K];

defset bigger(i) := { <j> in K with j > i };

subto c3: sum <i> in bigger(5) : y[i] >= 0;
