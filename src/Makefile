# $Id: Makefile,v 1.2 2001/01/31 03:24:06 pete Exp $

CFLAGS = -O2 -Wall -DFULLSCR

EO = \
	ed.o\
	expr.o\
	fullscr.o\
	screen.o\
	unassmbl.o\
	$E

.c.o:
	gcc $(CFLAGS) -c $*.c

all : fsdb

fsdb : $(EO) 
	gcc -v -o fsdb $(EO) -ldbg
	stubify fsdb

clean :
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
