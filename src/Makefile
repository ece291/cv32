# $Id: Makefile,v 1.6 2001/02/02 17:03:33 pete Exp $

CFLAGS = -O2 -Wall -DFULLSCR

CPPFLAGS = -fno-exceptions -fno-rtti -O2 -Wall

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

EO_TV = \
	debugapp.o\
	cvmenu.o\
	cvstatus.o\
	cvhint.o\
	fileview.o\
	$E

.c.o:
	gcc $(CFLAGS) -o $*.o -c $*.c

.cpp.o:
	gpp $(CPPFLAGS) -o $*.o -c $*.cpp

all : cv32 cv32tv

cv32 : $(EO) 
	gcc -v -o cv32 $(EO) -ldbg
	stubify cv32

cv32tv : $(EO_TV)
	gcc -o cv32tv $(EO_TV) -ldbg -lrhtv -lstdcxx
	stubify cv32tv

clean :
	-del *.o
	-del cv32
	-del cv32.exe
	-del cv32tv
	-del cv32tv.exe

veryclean : clean
	-del nasm\*.o
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
