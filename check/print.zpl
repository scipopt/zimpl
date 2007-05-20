# $Id: print.zpl,v 1.2 2007/05/20 09:25:53 bzfkocht Exp $
set A := { 1 .. 5 };
set B := { "a", "b", "c" };
var x[A];
do check card(A) > card(B);
do print "Test";
do print 13 mod 7 div 2;
do print A*B;
do forall <a,b> in A*B do print <a,b>;
do print x;
do print unknown;