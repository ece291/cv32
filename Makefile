# $Id: Makefile,v 1.11 2001/03/22 19:51:11 pete Exp $

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
	ldt.o\
	numproc.o\
	register.o\
	$E

all : cv32.exe cv32tv.exe

cv32 : $(EO)
	echo $(EO) > .files
	gcc -v -o cv32 @.files -ldbg
	-rm -f .files

cv32tv : $(EO_TV)
	echo $(EO_TV) > .files
	gcc -o cv32tv @.files -ldbg -lrhtv -lstdcxx
	-rm -f .files

%.o : %.c
	gcc $(CFLAGS) -c $< -o $@

%.o : %.cpp
	gpp $(CPPFLAGS) -c $< -o $@

%.exe : %
	stubify $<

.PHONY: clean veryclean

clean :
	-rm -f *.o
	-rm -f cv32 cv32.exe
	-rm -f cv32tv cv32tv.exe

veryclean : clean
	-rm -f nasm\*.o
	-rm -f expr.c

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

debugapp.o: debugapp.h fileview.h ldt.h numproc.h register.h cvconst.h
cvmenu.o: debugapp.h cvconst.h
cvstatus.o: debugapp.h cvhint.h hintstat.h cvconst.h
cvhint.o: cvhint.h cvconst.h
fileview.o: fileview.h cvconst.h
ldt.o: ldt.h
numproc.o: numproc.h
register.o: register.h

