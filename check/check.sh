#!/bin/sh
# $Id: check.sh,v 1.9 2003/10/16 08:20:30 bzfkocht Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#*                                                                           */
#*   File....: check.sh                                                      */
#*   Name....: check script                                                  */
#*   Author..: Thorsten Koch                                                 */
#*   Copyright by Author, All rights reserved                                */
#*                                                                           */
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#*
#* Copyright (C) 2003 by Thorsten Koch <koch@zib.de>
#* 
#* This program is free software; you can redistribute it and/or
#* modify it under the terms of the GNU General Public License
#* as published by the Free Software Foundation; either version 2
#* of the License, or (at your option) any later version.
#* 
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#* 
#* You should have received a copy of the GNU General Public License
#* along with this program; if not, write to the Free Software
#* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#*
# $1 = Binary
PASS=0
COUNT=0
for i in expr param set subto var bool define vinst
do
   COUNT=`expr $COUNT + 1` 
   $1 -v0 $i.zpl
   diff $i.lp $i.lp.ref >/dev/null
   case $? in
    0) echo Test $i "(lp)" OK; PASS=`expr $PASS + 1` ;;
    1) echo Test $i "(lp)" FAIL ;;
    *) echo Test $i "(lp)" ERROR ;;
   esac
   COUNT=`expr $COUNT + 1` 
   diff $i.tbl $i.tbl.ref >/dev/null
   case $? in
    0) echo Test $i "(tbl)" OK; PASS=`expr $PASS + 1`  ;;
    1) echo Test $i "(tbl)" FAIL ;;
    *) echo Test $i "(tbl)" ERROR ;;
   esac
   rm $i.tbl $i.lp
done 
#
cd errors
#
for i in e[1-6]*.zpl
do
   COUNT=`expr $COUNT + 1` 
   NAME=`basename $i .zpl`
   ../$1 -v0 $i 2>$NAME.err
   diff $NAME.err $NAME.err.ref >/dev/null
   case $? in
    0) echo Test $i "(err)" OK; PASS=`expr $PASS + 1`  ;;
    1) echo Test $i "(err)" FAIL ;;
    *) echo Test $i "(err)" ERROR ;;
   esac
   rm $NAME.err
done 2>/dev/null
#
# Error 700 to 900 can vary
#
for i in e[789]*.zpl
do
#   COUNT=`expr $COUNT + 1` 
   NAME=`basename $i .zpl`
   # DIFFOPT=`awk -f ../exdiffopt.awk $NAME.err.ref`
   ../$1 -v0 $i 2>$NAME.err
   diff $NAME.err $NAME.err.ref >/dev/null
   case $? in
    0) echo Test $i "(err)" OK;; #PASS=`expr $PASS + 1`  ;;
    1) echo Test $i "(err)" FAIL "(ignored)";;
    *) echo Test $i "(err)" ERROR ;;
   esac
   rm $NAME.err
done 2>/dev/null


if [ $PASS -eq $COUNT ] ; then echo All $PASS tests passed; 
else echo FAILURE! Only $PASS of $COUNT tests passed; 
fi







