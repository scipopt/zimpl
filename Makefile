# $Id: Makefile,v 1.20 2004/05/03 11:35:14 bzfkocht Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*   File....: Makefile                                                      *
#*   Name....: Zimpl Makefile                                                *
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
.PHONY:		depend clean lint doc check valgrind

ARCH            :=      $(shell uname -m | \
                        sed \
			-e s/sun../sparc/ \
			-e s/i.86/x86/ \
			-e s/IP../mips/ \
			-e s/9000..../hppa/ \
			-e s/Power\ Macintosh/ppc/ \
			-e s/00........../pwr4/)
OSTYPE          :=      $(shell uname -s | \
                        tr A-Z a-z | \
                        sed \
			-e s/cygwin.*/cygwin/ \
                        -e s/irix../irix/ )
HOSTNAME	:=      $(shell uname -n | tr A-Z a-z)

VERSION		=	2.02d
OPT		=	opt
COMP		=	gnu
CC		=	gcc
YACC		=	bison
LEX		=	flex
DCC		=	gcc
LINT		=	flexelint
AR		=	ar
RANLIB		=	ranlib
VALGRIND	=	valgrind --skin=memcheck --leak-check=yes \
			--show-reachable=yes 

SRCDIR		=	src
BINDIR		=	bin
LIBDIR		=	lib

CPPFLAGS	=	-I$(SRCDIR) -DVERSION='"$(VERSION)"'
CFLAGS		=	-O
LDFLAGS		=	-lgmp -lz -lm
YFLAGS		=	-d -t -v  
LFLAGS		=	-d
DFLAGS		=	-MM

GCCWARN		=	-Wall -W -Wpointer-arith -Wcast-align -Wwrite-strings \
			-Wstrict-prototypes -Wmissing-prototypes -Winline \
			-Wmissing-declarations -Wshadow -Waggregate-return \
			-Wno-unused -Wno-unknown-pragmas 

BASE		=	$(OSTYPE).$(ARCH).$(COMP).$(OPT)
OBJDIR		=	obj/O.$(BASE)
NAME		=	zimpl
TARGET		=	$(NAME)-$(VERSION).$(BASE)
BINARY		=	$(BINDIR)/$(TARGET)
DEPEND		=	$(SRCDIR)/depend

#-----------------------------------------------------------------------------

OBJECT  =       	bound.o code.o conname.o define.o elem.o entry.o \
			hash.o idxset.o iread.o list.o load.o local.o \
			mmlparse.o mmlscan.o numbgmp.o \
			prog.o rdefpar.o source.o \
			setempty.o setpseudo.o setlist.o setrange.o setprod.o \
			setmulti.o set4.o \
			stmt.o strstore.o symbol.o term.o tuple.o zimpl.o \
			ratlpstore.o ratlpfwrite.o ratmpswrite.o ratmstwrite.o \
			mshell.o inst.o ratordwrite.o xlpglue.o gmpmisc.o \
			ratpresolve.o rathumwrite.o vinst.o

OBJXXX		=	$(addprefix $(OBJDIR)/,$(OBJECT))
OBJSRC		=	$(addprefix $(SRCDIR)/,$(OBJECT:.o=.c))

#-----------------------------------------------------------------------------
include make/make.$(BASE)
-include make/local/make.$(HOSTNAME)
-include make/local/make.$(HOSTNAME).$(COMP)
-include make/local/make.$(HOSTNAME).$(COMP).$(OPT)
#-----------------------------------------------------------------------------

$(BINARY):	$(OBJDIR) $(BINDIR) $(OBJXXX)  
		$(CC) $(CFLAGS) $(OBJXXX) $(LDFLAGS) -o $@

$(SRCDIR)/mmlparse.c:	$(SRCDIR)/mmlparse.y $(SRCDIR)/mme.h
		$(YACC) $(YFLAGS) -o $@ $<

$(SRCDIR)/mmlscan.c:	$(SRCDIR)/mmlscan.l $(SRCDIR)/mme.h
		$(LEX) $(LFLAGS) -o$@ $< 

lint:		$(OBJSRC)
		$(LINT) $(SRCDIR)/project.lnt -os\(lint.out\) \
		$(CPPFLAGS) -UNDEBUG -Dinline= -DNO_MSHELL $^

doc:
		cd doc; make -f Makefile

check:
		cd check; \
		/bin/sh ./check.sh ../$(BINARY) 

valgrind:
		cd check; \
		/bin/sh ./check.sh "$(VALGRIND) ../$(BINARY)" 

$(OBJDIR):	
		-mkdir -p $(OBJDIR)

$(BINDIR):
		-mkdir -p $(BINDIR)

clean:
		-rm -rf $(OBJDIR)/* $(TARGET)

depend:
		$(SHELL) -ec '$(DCC) $(DFLAGS) $(CPPFLAGS) $(OBJSRC) \
		| sed '\''s|^\([0-9A-z\_]\{1,\}\)\.o|$$\(OBJDIR\)/\1.o|g'\'' \
		>$(DEPEND)'

-include	$(DEPEND)

$(OBJDIR)/%.o:	$(SRCDIR)/%.c
		$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# --- EOF ---------------------------------------------------------------------





