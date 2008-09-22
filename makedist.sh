#!/bin/sh
VERSION="2.08"
NAME="zimpl-$VERSION"
rm -f $NAME
ln -s . $NAME
rm -f $NAME.tgz
tar -cvzhf $NAME.tgz \
--exclude="*CVS*" \
--exclude="*cvs*" \
--exclude="*~" \
--exclude="*.o" \
--exclude="*.d" \
--exclude="*TODO" \
--exclude="lint.out" \
--exclude=".\#*" \
--exclude="$NAME/make/local/*" \
--exclude="*.output" \
$NAME/LICENSE $NAME/README $NAME/CHANGELOG \
$NAME/Makefile $NAME/make $NAME/src $NAME/doc/*.pdf \
$NAME/doc/zimpl.man $NAME/check \
$NAME/example/chvatal_diet.zpl $NAME/example/facility_location.zpl \
$NAME/example/queens[12345].zpl $NAME/example/steinerbaum.zpl \
$NAME/example/tsp.zpl $NAME/example/tsp.dat
rm -f $NAME
echo ""
echo "check version numbers in Makefile, doc/zimpl.tex (2x) and makedist.sh ($VERSION):"
grep "^VERSION" Makefile
grep " Version" doc/zimpl.tex

