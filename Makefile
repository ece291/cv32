# $Id: Makefile,v 1.21 2001/12/12 07:31:23 pete Exp $

CFLAGS = -march=pentiumpro -O2 -Wall -DFULLSCR -Iinclude

CPPFLAGS = -march=pentiumpro -fno-exceptions -fno-rtti -O2 -Wall -Iinclude

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
	disassembler.o\
	cvmenu.o\
	cvstatus.o\
	cvhint.o\
	fileview.o\
	ldt.o\
	numproc.o\
	register.o\
	expr.o\
	nasm/disasm.o\
	nasm/insnsd.o\
	nasm/sync.o\
	$E
LINK_EO_TV = $(addprefix obj/, $(EO_TV))

all : cv32.exe cv32tv.exe

cv32.exe : $(LINK_EO)
	gcc -o $@ $(LINK_EO) -ldbg

cv32tv.exe : $(LINK_EO_TV)
	gcc -o $@ $(LINK_EO_TV) -ldbg -lrhtv -lstdcxx

obj/%.o : src/%.c
	gcc $(CFLAGS) -c $< -o $@

src/%.c : src/%.y
	bison -o $@ $<

obj/%.o : src/%.cpp
	gpp $(CPPFLAGS) -c $< -o $@

obj/%.o : tests/%.asm
	nasm -f coff -o $@ $<

.PHONY: tests clean veryclean cv32objs cv32tvobjs

tests: tests/test01.exe tests/test02.exe

tests/%.exe : obj/%.o
	gcc -o $@ $<

clean :
	-del obj\*.o
	-del cv32.exe
	-del cv32tv.exe

veryclean : clean
	-del obj\nasm\*.o
	-del src\expr.c
	-del tests\*.exe

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
	include/numproc.h include/register.h include/cvconst.h \
	include/debugger.h include/disassembler.h
obj/debugger.o: include/debugger.h include/disassembler.h
obj/disassembler.o: include/disassembler.h
obj/cvmenu.o: include/debugapp.h include/cvconst.h
obj/cvstatus.o: include/debugapp.h include/cvhint.h include/hintstat.h \
	include/cvconst.h
obj/cvhint.o: include/cvhint.h include/cvconst.h
obj/fileview.o: include/fileview.h include/cvconst.h
obj/ldt.o: include/ldt.h include/debugger.h
obj/numproc.o: include/numproc.h include/cvconst.h
obj/register.o: include/register.h include/cvconst.h include/debugger.h
include/disassembler.h: include/debugger.h
