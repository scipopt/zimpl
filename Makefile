# $Id: Makefile,v 1.1 2001/05/06 11:43:18 thor Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*   File....: Makefile                                                      *
#*   Name....: ZIMPL Top level makefile                                      *
#*   Author..: Thorsten Koch                                                 *
#*   Copyright by Author, All rights reserved                                *
#*                                                                           *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#
#    Copyright (C) 2001 by Thorsten Koch <koch@zib.de>
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
.PHONY:		all clean

all:
		cd src; $(MAKE) -f Makefile; cd ..
		cd doc; $(MAKE) -f Makefile; cd ..
		cd test; $(MAKE) -f Makefile; cd ..

clean:
		cd src; $(MAKE) -f Makefile clean; cd ..
		cd doc; $(MAKE) -f Makefile clean; cd ..
		cd test; $(MAKE) -f Makefile clean; cd ..

