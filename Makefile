# $Id: Makefile,v 1.18 2001/04/27 06:27:23 pete Exp $

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
	debugger.o\
	cvmenu.o\
	cvstatus.o\
	cvhint.o\
	fileview.o\
	ldt.o\
	numproc.o\
	register.o\
	$E
LINK_EO_TV = $(addprefix obj/, $(EO_TV))

cv32objs: $(LINK_EO)

cv32.exe : $(LINK_EO)
	gcc -o $@ $(LINK_EO) -ldbg

cv32tvobjs: $(LINK_EO_TV)

cv32tv.exe : $(LINK_EO_TV)
	gcc -o $@ $(LINK_EO_TV) -ldbg -lrhtv -lstdcxx

obj/%.o : src/%.c
	gcc $(CFLAGS) -c $< -o $@

obj/%.o : src/%.cpp
	gpp $(CPPFLAGS) -c $< -o $@

obj/%.o : tests/%.asm
	nasm -f coff -o $@ $<

.PHONY: tests clean veryclean cv32objs cv32tvobjs

tests: tests/test01.exe tests/test02.exe

tests/%.exe : obj/%.o
	gcc -o $@ $<

clean :
	-rm -f obj/*.o
	-rm -f cv32.exe
	-rm -f cv32tv.exe

veryclean : clean
	-rm -f obj/nasm/*.o
	-rm -f src/expr.c
	-rm -f tests/*.exe

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
	include/debugger.h
obj/debugger.o: include/debugger.h
obj/cvmenu.o: include/debugapp.h include/cvconst.h
obj/cvstatus.o: include/debugapp.h include/cvhint.h include/hintstat.h \
	include/cvconst.h
obj/cvhint.o: include/cvhint.h include/cvconst.h
obj/fileview.o: include/fileview.h include/cvconst.h
obj/ldt.o: include/ldt.h include/debugger.h
obj/numproc.o: include/numproc.h include/cvconst.h
obj/register.o: include/register.h include/cvconst.h include/debugger.h

