# $Id: print.zpl,v 1.1 2004/05/29 11:29:35 bzfkocht Exp $
set A := { 1 .. 5 };
set B := { "a", "b", "c" };
do check card(A) > card(B);
do print "Test";
do print 13 mod 7 div 2;
do print A*B;
