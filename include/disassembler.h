/* CodeView/32 - Disassembler Header File */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: disassembler.h,v 1.1 2001/04/28 21:35:25 pete Exp $ */
#ifndef _DISASSEMBLER_H_
#define _DISASSEMBLER_H_

#include "debugger.h"

class Disassembler
{
public:
    Disassembler();
    ~Disassembler();

    char *Proper(word32 v, int &len);
    char *Source(word32 v);

private:
    char *ubuf, *ubufp;

    void uputchar(char c);
};

extern Disassembler *disassembler;

#endif

