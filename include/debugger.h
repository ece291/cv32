/* CodeView/32 - Debugger Header File */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: debugger.h,v 1.3 2001/04/28 21:38:58 pete Exp $ */
#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#define Uses_TProgram
#include <tv.h>

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
    typedef enum { R_Step, R_Step1, R_Over, R_Run, R_RunMain } STEP_TYPE;
    static const int MAXINSTLEN = 16;

    Debugger();
    ~Debugger();

    // Memory functions
    static bool IsValidAddr(word32 vaddr, int len)
    {
	return !invalid_sel_addr(a_tss.tss_ds, vaddr, len, 0);
    }
    static bool IsValidInstAddr(word32 vaddr)
    {
	return IsValidAddr(vaddr, MAXINSTLEN);
    }

    // Breakpoint functions
    int GetBreakpoint(BP_TYPE, int, word32);
    int SetBreakpoint(BP_TYPE, int, word32);
    void ResetBreakpoint(int);

    // Execution functions
    void Go(bool enablebps);
    void Step(STEP_TYPE);
    bool IsFirstStep() { return first_step; }

    // Code tracing functions
    bool StackTrace(word32 &eip, word32 &esp, word32 &ebp);
    word32 TraceTillSource(word32 eip, int level, bool mustmove);

    // Screen management functions
    static void Repaint()
    {
	TProgram::application->Redraw();
    }
    static void UserScreen()
    {
	TProgram::application->suspend();
    }
    static void DebuggerScreen()
    {
	TProgram::application->resume();
	Repaint();
    }
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

    bool first_step;
    word32 main_entry;
    bool terminated;

    int breakpoint_count;
    BP_ENTRY *breakpoint_table;
    static const unsigned char int03;

    void ActivateBreakpoints();
    void DeactivateBreakpoints();
};

extern Debugger *debugger;

#endif

