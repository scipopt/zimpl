# $Id: queens1.zpl,v 1.2 2003/10/27 17:19:33 bzfkocht Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*   File....: queens1.zpl                                                   *
#*   Name....: n-Queens Problem                                              *
#*   Author..: Thorsten Koch                                                 *
#*   Copyright by Author, All rights reserved                                *
#*                                                                           *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
#
# This is a formulation of the n queens problem using general integer
# variables. Please note that this particular formulation only works,
# if a queen can be placed in each row, so queens should be greater
# equal to 4
#
param queens := 8;

set I := { 1 .. queens };
set P := { <i,j> in I * I with i < j };
 
var x[I] integer >= 1 <= queens;

# All x have to be different 
#
subto c1: forall <i,j> in P do vabs(x[i] - x[j]) >= 1;

# Block diagonals => 
# never the same distance between two queens in x and y direction =>
# abs(x[i] - x[j]) != abs(i - j) =>
# a != b modeled as abs(a - b) >= 1
#
subto c2: forall <i,j> in P do vabs(vabs(x[i] - x[j]) - abs(i - j)) >= 1;
