# $Id: bool.zpl,v 1.2 2003/09/19 08:30:14 bzfkocht Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#*                                                                           */
#*   File....: bool.zpl                                                      */
#*   Name....: bool test                                                     */
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
set A := { 1 .. 10 };
set B := { 5 .. 15 };

param a1 := if A == B or A <= B then 1 else 2 end;
param a2 := if A != B and A < B then 1 else 2 end;
param a3 := if not A > B and not A < B then 1 else 2 end;
param a4 := if a1 < a2 or a2 != a3 then 1 else 2 end;

var x[A];

maximize c1: a1 * x[1] + a2 * x[2] + a3 * x[3] + a4 * x[4];
 








