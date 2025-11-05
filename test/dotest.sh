# $Id: dotest.sh,v 1.1 2001/05/06 11:43:21 thor Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*   File....: dotest.sh                                                     *
#*   Name....: ZIMPL Testscript                                              *
#*   Author..: Thorsten Koch                                                 *
#*   Copyright by Author, All rights reserved                                *
#*                                                                           *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#
#    Copyright (C) 2001-2025 by Thorsten Koch <koch@zib.de>
# 
#    This program is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License
#    as published by the Free Software Foundation; either version 2
#    of the License, or (at your option) any later version.
# 
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
# 
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
ZPL=../src/zimpl

for i in ex?.zpl testall.zpl mcaflow1.zpl
do
    NAME=`basename $i .zpl`
    echo Testing $NAME
    $ZPL -t lp $i
    diff -q $NAME.lp $NAME.lp.ok
    diff -q $NAME.tbl $NAME.tbl.ok
    $ZPL -t mps $i
    diff -q $NAME.mps $NAME.mps.ok
    diff -q $NAME.tbl $NAME.tbl.ok
done


