#!/bin/sh
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#*                                                                           */
#*   File....: check.sh                                                      */
#*   Name....: check script                                                  */
#*   Author..: Thorsten Koch                                                 */
#*   Copyright by Author, All rights reserved                                */
#*                                                                           */
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#*
#* Copyright (C) 2001 by Thorsten Koch <koch@zib.de>
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
echo XXX $1 XXX
for i in expr param set subto
do
   $1 $i.zpl
   diff $i.lp $i.lp.ref >/dev/null
   case $? in
    0) echo Test $i "(lp)" OK; PASS=`expr $PASS + 1` ;;
    1) echo Test $i "(lp)" FAIL ;;
    *) echo Test $i "(lp)" ERROR ;;
   esac
   diff $i.tbl $i.tbl.ref >/dev/null
   case $? in
    0) echo Test $i "(tbl)" OK; PASS=`expr $PASS + 1`  ;;
    1) echo Test $i "(tbl)" FAIL ;;
    *) echo Test $i "(tbl)" ERROR ;;
   esac
   rm $i.tbl $i.lp
done 
if [ $PASS -eq 8 ] ; then echo All test passed; 
else echo FAILURE!; 
fi