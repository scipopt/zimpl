# $Id: Makefile,v 1.32 2003/03/18 09:37:03 bzfkocht Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*   File....: Makefile                                                      *
#*   Name....: ZIMPL makefile                                                *
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

.PHONY:		clean lint

NAME		=	zimpl
CC		=	gcc 
YACC		=	bison
LEX		=	flex
CPPFLAGS	=	-I./ 
OPTI_FLAGS      =       -march=pentiumpro -O#3 
WARN_FLAGS	=	-Wall -W -Wno-unused -Wno-char-subscripts -Wshadow \
			-Wcast-align -Wwrite-strings -Waggregate-return \
			-Wmissing-prototypes -Wmissing-declarations -Winline
DEBU_FLAGS      =       -g #-DNDEBUG -DNO_MSHELL #-DFREEMEM -DTRACE  
LDFLAGS		=	-lm 
DFLAGS		=	-MM 
YFLAGS		=	-d -t -v  
LFLAGS		=	-d
LINT		=	flexelint
#
# -----------------------------------------------------------------------------
# --- nothing to change from here on                                        ---
#------------------------------------------------------------------------------
#
CFLAGS	=	$(OPTI_FLAGS) $(DEBU_FLAGS) $(PROG_FLAGS) $(WARN_FLAGS) 
OBJECT  =       code.o conname.o elem.o entry.o hash.o idxset.o inst.o \
		iread.o list.o load.o local.o lpstore.o lpfwrite.o \
		mmlparse.o mmlscan.o mpswrite.o ordwrite.o \
		prog.o rdefpar.o set.o source.o \
		stmt.o strstore.o symbol.o term.o tuple.o zimpl.o \
		mshell.o

$(NAME):	$(OBJECT)
		$(CC) $(CFLAGS) $(OBJECT) $(LDFLAGS) -o $@ 

mmlparse.c:	mmlparse.y mme.h
		$(YACC) $(YFLAGS) -o $@ $<

mmlscan.c:	mmlscan.l mme.h
		$(LEX) $(LFLAGS) -o$@ $< 

-include	$(OBJECT:.o=.d)

lint:		$(OBJECT:.o=.c)
		$(LINT) project.lnt -os\(lint.out\) $(CPPFLAGS) \
		-UNDEBUG -DNO_MSHELL $^ 

clean:
		-rm -f *.o *.d $(NAME) mmlscan.c mmlparse.[ch] \
		mmlparse.output lint.out 

%.d:		%.c
		$(SHELL) -ec '$(CC) $(DFLAGS) $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' >$@; \
		[ -s $@ ] || rm -f $@'

# --- EOF ---------------------------------------------------------------------

