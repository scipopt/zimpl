#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#*                                                                           */
#*   File....: param.zpl                                                     */
#*   Name....: Param test                                                    */
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
set I := { 1 .. 10 };
set J := { "a", "b", "c", "x", "y", "z" };

param a := 5;
param b := "hallo";
param c[I] := <1> "a", <2> "b", <3> "c";
param d[I] := <2> "x", <3> "y" default "z";
param e[<i> in I with i > 5] := i + 2;
param f[<5,j> in I * { "a", "b" }] := j;
param g[J] := <"a"> 1, <"b"> 2, <"c"> 3, <"x"> 4, <"y"> 5, <"z"> 9; 

var x[I];

subto a1: a * x[1] >= 0;
subto b2: if b == "hallo" then 5 else 3 end * x[1] >= 0;
subto c1 : sum <i> in I with i < 4: g[c[i]] * x[i] >= 0;
subto d1 : sum <i> in I : g[d[i]] * x[i] >= 0;
subto e1 : sum <i> in I with i > 5: e[i] * x[i] >= 0;
subto f1 : g[f[5,"a"]] * x[1] + g[f[5,"b"]] * x[2] >= 0;

