/* CodeView/32 - Debugger Header File */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: debugger.h,v 1.2 2001/04/27 18:27:12 pete Exp $ */
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

typedef unsigned long word32;
typedef unsigned short word16;
typedef unsigned char word8;
//typedef signed long int32;
typedef signed short int16;
typedef signed char int8;

// debugging functions from dbgcom.c (libdbg)
extern "C" int invalid_sel_addr(short sel, unsigned a, unsigned len, char for_write);

class Debugger
{
public:
    typedef enum { BP_Code = 0, BP_Write = 1, BP_Read = 3 } BP_TYPE;
    static const int MAXINSTLEN = 16;
    bool first_step;
    word32 main_entry;

    Debugger();
    ~Debugger();

    // Memory functions
    bool IsValidAddr(word32 vaddr, int len)
    {
	return !invalid_sel_addr(a_tss.tss_ds, vaddr, len, 0);
    }
    bool IsValidInstAddr(word32 vaddr)
    {
	return IsValidAddr(vaddr, MAXINSTLEN);
    }

    // Breakpoint functions
    void ActivateBreakpoints();
    void DeactivateBreakpoints();
    int GetBreakpoint(BP_TYPE, int, word32);
    int SetBreakpoint(BP_TYPE, int, word32);
    void ResetBreakpoint(int);

private:
    typedef struct {
	word32 addr;		    // Linear address of breakpoint.
	int count;		    // #hits left before code bp triggers.
	char *condition;	    // Condition for code breakpoints.
	BP_TYPE type;		    // What kind of breakpoint?
	unsigned char length;	    // 1, 2, or 4 bytes.
	unsigned char saved;	    // Non-zero for code bps using int 0x03.
	unsigned char savedbyte;    // Patched-away byte.
	unsigned char disabled;	    // Non-zero means don't put into cpu regs.
	unsigned char temporary;    // A temporary like those used for F4.
    } BP_ENTRY;

    int breakpoint_count;
    BP_ENTRY *breakpoint_table;
    static const unsigned char int03;
};

extern Debugger *debugger;

#endif

