/* CodeView/32 - Debugger Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: debugger.cpp,v 1.2 2001/04/27 18:27:03 pete Exp $ */

#include <stdlib.h>
#include <string.h>
#include <debug/dbgcom.h>
#include <debug/syms.h>

#include "debugger.h"

extern "C" void *xmalloc(size_t);
extern "C" void *xrealloc(void *, size_t);

const unsigned char Debugger::int03 = 0xcc;

Debugger::Debugger()
{
    main_entry = syms_name2val("_main");
    first_step = !undefined_symbol;

    breakpoint_count = 0;
    breakpoint_table = (BP_ENTRY *)xmalloc(0);
}

Debugger::~Debugger()
{
    free(breakpoint_table);
}

/* Set physical breakpoint registers from virtual ones.	 Only the first
   four data breakpoints are honoured.  When the debug registers are
   full, extra code breakpoints are set by patching "Int 3" into the
   code.  */

void Debugger::ActivateBreakpoints()
{
    int b, no;
    BP_ENTRY *bep;

    no = 4;	// disable hardware breakpoints.
    edi.dr[7] = 0;

    // First handle data breakpoints.
    for(b=0, bep=breakpoint_table; b < breakpoint_count; b++, bep++)
	if(!bep->disabled && no <= 3 && bep->type != BP_Code) {
	    bep->saved = 0;
	    edi.dr[7] |= ((bep->type + ((bep->length-1) << 2)) << (16+4*no)
		| (2 << (2*no)));
	    edi.dr[no] = bep->addr;
	    no++;
	}

    // Now handle code breakpoints.
    for(b=0, bep=breakpoint_table; b < breakpoint_count; b++, bep++)
	if (!bep->disabled && bep->type == BP_Code) {
	    if (no <= 3) {
		bep->saved = 0;
		edi.dr[7] |= ((BP_Code << (16+4*no)) | (2 << (2*no)));
		edi.dr[no] = bep->addr;
		no++;
		edi.dr[7] |= 0x00000300L;   // For 386s we set GE & LE bits.
	    } else {
		bep->saved = IsValidAddr(bep->addr, 1);
		if(bep->saved) {
		    read_child(bep->addr, &bep->savedbyte, 1);
		    write_child(bep->addr, (void *)&int03, 1);
		}
	    }
	}
}

/* Un-patch code.  This means restore the instruction under any "Int 3"
   that was patched-in by previous function.  */
void Debugger::DeactivateBreakpoints()
{
    int b;
    BP_ENTRY *bep;

    for(b=0, bep=breakpoint_table; b < breakpoint_count; b++, bep++)
	if(!bep->disabled && bep->saved)
	    write_child(bep->addr, &bep->savedbyte, 1);
}

/* Get the serial number of a breakpoint of TYPE and LENGTH at ADDR.  Return
   -1 if no such breakpoint is set.  */
int Debugger::GetBreakpoint(BP_TYPE type, int length, word32 addr)
{
    for(int b=0; b < breakpoint_count; b++)
	if(breakpoint_table[b].type == type &&
	    breakpoint_table[b].length == length &&
	    breakpoint_table[b].addr == addr)
	    return b;
    return -1;
}

/* Set a breakpoint of TYPE and LENGTH at ADDR and return the serial number
   of the new breakpoint.  */
int Debugger::SetBreakpoint(BP_TYPE type, int length, word32 addr)
{
    int b = breakpoint_count;
    breakpoint_table = (BP_ENTRY *)xrealloc(breakpoint_table,
	++breakpoint_count*sizeof(BP_ENTRY));
    bzero(breakpoint_table+b, sizeof(BP_ENTRY));
    breakpoint_table[b].addr = addr;
    breakpoint_table[b].type = type;
    breakpoint_table[b].length = length;
    return b;
}

/* Delete breakpoint number B.  */
void Debugger::ResetBreakpoint(int b)
{
    if(breakpoint_table[b].condition)
	free(breakpoint_table[b].condition);
    breakpoint_table[b] = breakpoint_table[--breakpoint_count];
    breakpoint_table = (BP_ENTRY *)xrealloc(breakpoint_table,
	breakpoint_count*sizeof(BP_ENTRY));
}

