#!/bin/bash -e

VERSION="3.5.1"
NAME="zimpl-$VERSION"

echo ">>> Packaging $NAME."

# remove old tarball
rm -f $NAME.tgz

# compile a fresh documentation
echo ">>> Compiling documentation."
make doc > /dev/null 2>&1

# create tarball
echo ">>> Creating tarball."
tar --transform "s|^|${NAME}/|" -czhf $NAME.tgz \
--exclude="*CVS*" \
--exclude="*cvs*" \
--exclude="*~" \
--exclude="*.o" \
--exclude="*.d" \
--exclude="*TODO" \
--exclude="lint.out" \
--exclude=".\#*" \
--exclude="make/local/*" \
--exclude="*.output" \
--exclude="*OLD*" \
LICENSE README CHANGELOG \
Makefile Makefile.nmake make src doc/zimpl.pdf \
doc/zimpl.man check \
example/chvatal_diet.zpl example/facility_location.zpl \
example/queens[12345].zpl example/steinerbaum.zpl \
example/tsp.zpl example/tsp.dat \
CMakeLists.txt zimpl-config.cmake.in \
cmake/Modules/FindGMP.cmake cmake/Modules/FindPCRE.cmake

# check version numbers
echo ">>> Checking version numbers in Makefile, doc/zimpl.tex (2x), doc/docu.c, CMakeLists.txt, src/zimpl/mme.h and scripts/makedist.sh. (Should be $VERSION)"

# find all version numbers
VERSIONS=""
VERSIONS="$VERSIONS\n$(grep "^VERSION" Makefile)"
VERSIONS="$VERSIONS\n$(grep " Version" doc/zimpl.tex |grep -v date|grep -v License)"
VERSIONS="$VERSIONS\n$(grep "@version" doc/docu.c)"
VERSIONS="$VERSIONS\n$(grep " VERSION" CMakeLists.txt |head -n 1)"
VERSIONS="$VERSIONS\n$(grep "ZIMPL_VERSION" src/zimpl/mme.h)"

# boil the formerly found versionnumbers down to a short format
VERSIONNUMBERS=$(echo "$VERSIONS" | grep  -o "[0-9.]*" | sed 's/\.//g')

# check if the boiled down numbers coincide with the one defined above
VERSIONNUMBER=$(echo "$VERSION" | sed 's/\.//g')
while read -r line; do
  if [ "${VERSIONNUMBER}" != "${line}" ]; then
    echo "${VERSIONNUMBER} and ${line} do not coincide! Aborting."
    exit 1
  fi
done <<< "${VERSIONNUMBERS}";

echo ">>> Successfully packaged ${NAME}."
