# $Id: Makefile,v 1.3 2001/01/31 04:38:59 pete Exp $

CFLAGS = -O2 -Wall -DFULLSCR

EO = \
	ed.o\
	expr.o\
	fullscr.o\
	screen.o\
	unassmbl.o\
	nasm/disasm.o\
	nasm/insnsd.o\
	nasm/sync.o\
	$E

.c.o:
	gcc $(CFLAGS) -o $*.o -c $*.c

all : fsdb

fsdb : $(EO) 
	gcc -v -o fsdb $(EO) -ldbg
	stubify fsdb

clean :
	-del nasm\*.o
	-del *.o
	-del fsdb
	-del fsdb.exe
	-del expr.c

# DEPENDENCIES

unassmbl.o : ed.h unassmbl.h

fullscr.o: ed.h unassmbl.h screen.h

screen.o: ed.h screen.h

expr.c: ed.h 

ed.o: ed.h debug.h 

nasm/disasm.o: nasm/nasm.h nasm/disasm.h nasm/sync.h nasm/insns.h nasm/names.c

nasm/insnsd.o: nasm/nasm.h nasm/insns.h

nasm/sync.o: nasm/sync.h

nasm/names.c: nasm/insnsn.c

nasm/nasm.h: nasm/insnsi.h
