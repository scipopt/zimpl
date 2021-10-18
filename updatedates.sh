#!/usr/bin/env bash
#
# This bash script updates all copyrights in files that are under version
# control (git) and have a ZIB copyright.
#
# You just have to run this script. There is nothing to adjust.
# The correct year is detected through the 'date' function
#
# Note that not all files (usually scripts) contain a copyright. A copyright is only
# needed for those files which are part of a SCIP distribution (see makedist.sh)

set -euo pipefail

NEWYEAR=`date +"%Y"`

echo "Updating copyright of all files under version control and list findings of possibly incorrect copyright string."

for f in `git ls-files` ; do

# skip binary files
grep -Iq . $f || continue

# skip this file
[[ $f =~ "updatedates.sh" ]] && continue

# skip symbolic links
if `test -L $f`; then
   echo "Skipping symbolic link $f"
   continue
fi

# Copyright (C) 2005-2017 by Thorsten Koch <koch@zib.de>
# process files with ZIB copyright string that do not include current year
if grep -o 'Copyright (C) [0-9]*-[0-9]* by Thorsten Koch' $f | grep -vq $NEWYEAR ; then
   echo "Updating $f"
   sed -i "s/Copyright (C) \([0-9]*\)-[0-9]* by Thorsten Koch/Copyright (C) \1-$NEWYEAR by Thorsten Koch/g" $f
fi

# print matches for lines that have "Copyright" and "Zuse" but are not a valid ZIB copyright
grep -iH "Copyright" $f | grep -v "Copyright (C) [0-9]*-$NEWYEAR byThorsten Koch" || true

done
