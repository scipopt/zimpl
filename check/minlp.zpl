# $Id: minlp.zpl,v 1.2 2011/10/25 08:18:01 bzfkocht Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*   File....: minlp.zpl                                                     *
#*   Name....: MINLP test                                                    *
#*   Author..: Thorsten Koch                                                 *
#*   Copyright by Author, All rights reserved                                *
#*                                                                           *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*
#* Copyright (C) 2011 by Thorsten Koch <koch@zib.de>
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
set I := { 1 to 9 };

var x[I] binary;
var y binary;
var z binary;

subto c1: x[1]^2 == x[1]^1 + x[1]^0;
subto c2: 2 * x[1] * 3 * x[2] * 5 * x[3] <= 77;
subto c3: y * z == y^2 * z^3 * 4;
subto c4: sum <i> in I : i*x[i]^i == 8;
subto c5: (y + 5) * (z + 2) == 3;
subto c6: (4/2 * x[1] + 2/3 * x[2])^2 == 9;
