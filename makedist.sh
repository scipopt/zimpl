#!/bin/sh
VERSION="3.3.4"
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
--exclude="*OLD*" \
$NAME/LICENSE $NAME/README $NAME/CHANGELOG \
$NAME/Makefile $NAME/Makefile.nmake $NAME/make $NAME/src $NAME/doc/zimpl.pdf \
$NAME/doc/zimpl.man $NAME/check \
$NAME/example/chvatal_diet.zpl $NAME/example/facility_location.zpl \
$NAME/example/queens[12345].zpl $NAME/example/steinerbaum.zpl \
$NAME/example/tsp.zpl $NAME/example/tsp.dat \
$NAME/CMakeLists.txt $NAME/src/CMakeLists.txt $NAME/zimpl-config.cmake.in \
$NAME/cmake/Modules/FindGMP.cmake $NAME/cmake/Modules/FindPCRE.cmake
rm -f $NAME
echo ""
echo "check version numbers in Makefile, doc/zimpl.tex (2x), doc/docu.c, makedist.sh and ($VERSION):"
grep "^VERSION" Makefile
grep " Version" doc/zimpl.tex
grep "@version" doc/docu.c
grep "ZIMPL_VERSION" src/zimpl/mme.h
