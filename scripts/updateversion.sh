#!/bin/bash
#
# Script to update all version numbers in ZIMPL
#
# usage: updateversion.sh [<major> <minor> <patch>]"
#

if [[ $# -eq 3 ]]
then
  NEWVERSION=$1.$2.$3


  sed -i "s/^VERSION	.*/VERSION		=  ${NEWVERSION}/" Makefile
  sed -i "s/^VERSION	.*/VERSION		=  ${NEWVERSION}/" Makefile.nmake
  sed -i "s/^VERSION=.*/VERSION=\"${NEWVERSION}\"/" scripts/makedist.sh

  if [[ "$1" -eq "0" ]]
  then
    sed -i "s/ZIMPL_VERSION \+[0-9]\+/ZIMPL_VERSION  $2$3/" src/zimpl/mme.h
    if [[ "$2" -eq "0" ]]
    then
      sed -i "s/ZIMPL_VERSION \+[0-9]\+/ZIMPL_VERSION  $3/" src/zimpl/mme.h
    fi
  else
    sed -i "s/ZIMPL_VERSION \+[0-9]\+/ZIMPL_VERSION  $1$2$3/" src/zimpl/mme.h
  fi
  sed -i "s/@version  \+[0-9].[0-9].[0-9]\+/@version  ${NEWVERSION}/" doc/docu.c

  sed -i "s/    VERSION \+[0-9].[0-9].[0-9]\+/    VERSION ${NEWVERSION}/" CMakeLists.txt

  echo "new version:"
  grep " VERSION" CMakeLists.txt
  grep -i @version doc/docu.c
  grep -i ZIMPL_VERSION src/zimpl/mme.h
  grep -i ^VERSION= scripts/makedist.sh
  grep -i ^VERSION Makefile.nmake Makefile
else
  echo "usage:"
  echo ""
  echo "$0 <major> <minor> <patch>"
  echo " -- update all version numbers to the specified one"
  echo ""
  echo "current version:"
  grep " VERSION" CMakeLists.txt
  grep -i @version doc/docu.c
  grep -i ZIMPL_VERSION src/zimpl/mme.h
  grep -i ^VERSION= scripts/makedist.sh
  grep -i ^VERSION Makefile.nmake Makefile
  exit 1;
fi

echo "TODO update in two places in doc/zimpl.tex"
