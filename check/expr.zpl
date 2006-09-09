# $Id: expr.zpl,v 1.12 2006/09/09 10:00:21 bzfkocht Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*   File....: expr.zpl                                                      *
#*   Name....: Expression test                                               *
#*   Author..: Thorsten Koch                                                 *
#*   Copyright by Author, All rights reserved                                *
#*                                                                           *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*
#* Copyright (C) 2006 by Thorsten Koch <koch@zib.de>
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

param a[<i> in I] := i + 3;

var x[I];

subto c01: (3 * a[1] + 5) * x[1] >= a[2] * 2 - 4;
subto c02: a[1] mod 2 >= x[1] / 7;
subto c03: a[1] div 2 >= x[1] - 3;
subto c04: card(I) * x[1] >= abs(a[1] - a[2]);
subto c05: a[1]^a[2] <= x[1] * 3!;
subto c06: floor(a[1] / 3) <= x[2] * ceil(a[2] / 7);
subto c07: -exp(ln(a[4])) >= -x[3];
subto c08: x[3] + log(10) == -6;
subto c09: (min <i> in I : a[i]) * x[1] >= x[2] * max <i> in I : a[i];
subto c10: x[1] * if sum <i> in I : a[i] > 20 then 2 else -4 end >= 5;
subto c11: a[1]^-a[2] >= x[2] / 100;
subto c12: x[3] >= min(6, 2/7, a[1], a[3]);
subto c13: x[3] <= max(6, 2/7, a[1], a[3]);
subto c14: sum <i> in {1..10} do i mod 8 * x[i] >= 5;
subto c15: sum <i> in I do sgn(5 - i) * x[i] >= 3;
subto c16: sum <i> in I do random(10,20) * x[i] >= random(50,1000);
subto c17: x[min I] <= x[max I];
