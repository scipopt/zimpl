#--- $Id: make.rei,v 1.2 2004/04/14 11:56:40 bzfkocht Exp $
CPPFLAGS        +=      -I/tmp/gmp/include
LDFLAGS         =       -L/tmp/gmp/lib -lgmp -lz -lm -static
