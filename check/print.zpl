# $Id: print.zpl,v 1.3 2007/08/01 10:17:13 bzfkocht Exp $
set A := { 1 .. 5 };
set B := { "a", "b", "c" };
#
var x[A];
var y binary;
var z integer >= -5 <= 7;
var w implicit integer >= 8;
#
do check card(A) > card(B);
do print "Test: ", 13 mod 7 div 2;
do print A*B;
do forall <a,b> in A*B do print <a,b>;
do print x;
do print y;
do print z;
do print w;
do print unknown;
