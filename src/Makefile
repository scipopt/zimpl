# $Id: Makefile,v 1.6 2001/01/30 08:23:44 thor Exp $
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*   Type....: Makefile                                                      *
#*   File....: Makefile                                                      *
#*   Name....: MME makefile                                                  *
#*   Author..: Thorsten Koch                                                 *
#*   Copyright by Author, All rights reserved                                *
#*                                                                           *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

.PHONY:		clean lint

NAME		=	mme
CC		=	gcc 
YACC		=	bison
LEX		=	flex
CPPFLAGS	=	-I./ 
OPTI_FLAGS      =       -O
WARN_FLAGS	=	-Wall -W -Wno-unused -Wno-char-subscripts -Wshadow \
			-Wcast-align -Wwrite-strings -Waggregate-return \
			-Wmissing-prototypes -Wmissing-declarations -Winline
DEBU_FLAGS      =       -g #-DNDEBUG #-pg -DNO_MSHELL #  
LDFLAGS		=	-lm
DFLAGS		=	-MM
YFLAGS		=	-v -d -t
LFLAGS		=	-d
LINT		=	lintgcc
#
# -----------------------------------------------------------------------------
# --- nothing to change from here on                                        ---
#------------------------------------------------------------------------------
#
CFLAGS	=	$(OPTI_FLAGS) $(DEBU_FLAGS) $(PROG_FLAGS) $(WARN_FLAGS) 
OBJECT  =       code2.o elem.o entry.o hash.o idxset.o ineq.o inst.o list.o \
		load.o local.o mme.o mmlparse.o mmlscan.o prog.o set.o \
		stmt.o strstore.o symbol.o term.o tuple.o var.o \
		mshell.o

$(NAME):	$(OBJECT)
		$(CC) $(CFLAGS) $(OBJECT) $(LDFLAGS) -o $@ 

mmlparse.c:	mmlparse.y mme.h
		$(YACC) $(YFLAGS) -o $@ $<

mmlscan.c:	mmlscan.l mme.h
		$(LEX) $(LFLAGS) -o$@ $< 

include		$(OBJECT:.o=.d)

lint:		$(OBJECT:.o=.c)
		$(LINT) project.lnt -os\(lint.out\) $(CPPFLAGS) -DNO_MSHELL $^ 

clean:
		-rm -f *.o *.d $(NAME) mmlscan.c mmlparse.[ch] lint.out 

%.d:		%.c
		$(SHELL) -ec '$(CC) $(DFLAGS) $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' >$@; \
		[ -s $@ ] || rm -f $@'

# --- EOF ---------------------------------------------------------------------

