# $Id: Makefile,v 1.13 2001/04/22 22:13:45 pete Exp $

CFLAGS = -O2 -Wall -DFULLSCR -Iinclude

CPPFLAGS = -fno-exceptions -fno-rtti -O2 -Wall -Iinclude

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
LINK_EO = $(addprefix obj/, $(EO))

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
LINK_EO_TV = $(addprefix obj/, $(EO_TV))

all : cv32.exe cv32tv.exe

cv32.exe : $(LINK_EO)
	echo $(LINK_EO) > .files
	gcc -v -o $@ @.files -ldbg
	-rm -f .files

cv32tv.exe : $(LINK_EO_TV)
	echo $(LINK_EO_TV) > .files
	gcc -o $@ @.files -ldbg -lrhtv -lstdcxx
	-rm -f .files

obj/%.o : src/%.c
	gcc $(CFLAGS) -c $< -o $@

obj/%.o : src/%.cpp
	gpp $(CPPFLAGS) -c $< -o $@

.PHONY: clean veryclean

clean :
	-rm -f obj/*.o
	-rm -f cv32.exe
	-rm -f cv32tv.exe

veryclean : clean
	-rm -f obj/nasm/*.o
	-rm -f src/expr.c

# DEPENDENCIES

obj/unassmbl.o : include/ed.h include/unassmbl.h
obj/fullscr.o: include/ed.h include/unassmbl.h include/screen.h
obj/screen.o: include/ed.h include/screen.h
src/expr.c: include/ed.h 
obj/ed.o: include/ed.h include/debug.h 
obj/nasm/disasm.o: src/nasm/nasm.h src/nasm/disasm.h src/nasm/sync.h \
	src/nasm/insns.h src/nasm/names.c
obj/nasm/insnsd.o: src/nasm/nasm.h src/nasm/insns.h
obj/nasm/sync.o: src/nasm/sync.h
src/nasm/names.c: src/nasm/insnsn.c
src/nasm/nasm.h: src/nasm/insnsi.h

obj/debugapp.o: include/debugapp.h include/fileview.h include/ldt.h \
	include/numproc.h include/register.h include/cvconst.h
obj/cvmenu.o: include/debugapp.h include/cvconst.h
obj/cvstatus.o: include/debugapp.h include/cvhint.h include/hintstat.h \
	include/cvconst.h
obj/cvhint.o: include/cvhint.h include/cvconst.h
obj/fileview.o: include/fileview.h include/cvconst.h
obj/ldt.o: include/ldt.h
obj/numproc.o: include/numproc.h include/cvconst.h
obj/register.o: include/register.h include/cvconst.h

