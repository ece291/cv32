/* CodeView/32 - Disassembler Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: disassembler.cpp,v 1.1 2001/04/28 21:35:35 pete Exp $ */

#include <stdlib.h>
#include <string.h>
#include <debug/dbgcom.h>
#include <debug/syms.h>

#include "disassembler.h"

extern "C" {
#include "nasm/disasm.h"
}

Disassembler::Disassembler()
{
    ubuf = (char *)malloc(4000);
}

Disassembler::~Disassembler()
{
    free(ubuf);
}

void Disassembler::uputchar(char c)
{
    if (c == '\t') {
	do {
	    *ubufp++ = ' ';
	} while ((ubufp-ubuf) % 8);
    } else
	*ubufp++ = c;
    *ubufp = 0;
}

char *Disassembler::Proper(word32 v, int &len)
{
    char unasm_buf[1000];
    word8 buf[20];
    char *p;

    read_child(v, buf, 20);
    len = disasm(buf, unasm_buf, 32, v, 0, 0);
    ubufp = ubuf;
    p = unasm_buf;
    while(*p)
	uputchar(*p++);
    return ubuf;
}

char *Disassembler::Source(word32 v)
{
    return 0;
}

