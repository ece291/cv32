/* CodeView/32 - Debugger Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: debugger.cpp,v 1.3 2001/04/28 21:39:17 pete Exp $ */

#include <stdlib.h>
#include <string.h>
#include <debug/dbgcom.h>
#include <debug/syms.h>

#define Uses_MsgBox
#include <tv.h>

#include "debugger.h"
#include "disassembler.h"

extern "C" void *xmalloc(size_t);
extern "C" void *xrealloc(void *, size_t);

extern "C" int evaluate(char *, long *, char **);

const unsigned char Debugger::int03 = 0xcc;

Debugger::Debugger()
{
    main_entry = syms_name2val("_main");
    first_step = !undefined_symbol;

    terminated = false;

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

/* Let the child process run freely (or rather as freely as debug registers
   and breakpoints decide.  */

void Debugger::Go(bool enablebps)
{
    if(enablebps)
	ActivateBreakpoints();
    run_child();
    if(enablebps)
	DeactivateBreakpoints();
}

/* Let the child process loose in the specified way.  Try to optimize for
   reduced swapping of screens.  */
void Debugger::Step(STEP_TYPE kind)
{
    int i, b = -1, no, len;
    bool int03hit;
    char *inst = 0;
    bool tracing = (kind == R_Step1);
    word32 final = 0xFFFFFFFF;

    switch (kind) {
	case R_Step1:
	    kind = R_Step;
	    // Fall through.
	case R_Step:
	    inst = disassembler->Proper(a_tss.tss_eip, len);
	    if(strcmp(inst, "popf") == 0 || strcmp(inst, "pushf") == 0) {
		kind = R_Over;	// Push the right value of flags (no trace flag)
		final = a_tss.tss_eip + len;
	    }
	    break;
	case R_Over:
	    if(first_step)
		kind = R_RunMain;
	    else {
		inst = disassembler->Proper(a_tss.tss_eip, len);
		if(strncmp(inst, "loop", 4) && strncmp(inst, "call", 4) &&
		    strncmp(inst, "int", 3))
		    kind = R_Step;
		else
		    final = a_tss.tss_eip + len;
	    }
	    break;
	default:
	    ;	// Nothing.
    }

retry:
    // A free run at a non-disabled breakpoint is bad.
    if(kind != R_Step) {
	bool again = true;

	while(again && (b = GetBreakpoint(BP_Code, 0, a_tss.tss_eip)) != -1) {
	    breakpoint_table[b].disabled++;
	    Step(R_Step1);
	    breakpoint_table[b].disabled--;
	    again = !terminated && a_tss.tss_irqn == 1 &&
		final != a_tss.tss_eip;
	}
    }

    switch (kind) {
	case R_Step1:	// Can't happen, but keeps gcc happy.
	case R_Step:
	    // If we are referencing memory we should swap-in the user screen.
	    if(strchr(inst, '[')) {
		/* Assume that all access to code and stack segments are safe.
		   This should hold unless there's something extra-ordinarily
		   dirty going on. */
		if(((a_tss.tss_ds == a_tss.tss_cs) ||
		    (a_tss.tss_ds == a_tss.tss_ss)) &&
		    ((a_tss.tss_es == a_tss.tss_cs) ||
		    (a_tss.tss_es == a_tss.tss_ss)) &&
		    ((a_tss.tss_fs == a_tss.tss_cs) ||
		    (a_tss.tss_fs == a_tss.tss_ss) ||
		    (strstr (inst, "fs:") == 0)) &&
		    ((a_tss.tss_gs == a_tss.tss_cs) ||
		    (a_tss.tss_gs == a_tss.tss_ss) ||
		    (strstr (inst, "gs:") == 0)))
		    /* Nothing. */;
		else
		    UserScreen();
	    }
	    a_tss.tss_eflags |= 0x0100;
	    edi.dr[7] = 0;
	    Go(false);
	    a_tss.tss_eflags &= ~0x0100;
	    break;
	case R_Over:
	    UserScreen();
	    b = SetBreakpoint(BP_Code, 0, final);
	    Go(true);
	    break;
	case R_Run:
	    UserScreen();
	    Go(true);
	    break;
	case R_RunMain:
	    b = SetBreakpoint(BP_Code, 0, main_entry);
	    UserScreen();
	    Go(true);
	    ResetBreakpoint(b);
	    break;
    }
    first_step = false;
    i = a_tss.tss_irqn;
    if((terminated = (i == 0x21) && (a_tss.tss_eax & 0xff00) == 0x4c00))
	a_tss.tss_eip -= 2;	// point back to Int 21h
    if((int03hit = (i == 0x03) &&
	(GetBreakpoint(BP_Code, 0, a_tss.tss_eip-1) != -1 ||
	kind == R_RunMain)))
	a_tss.tss_eip--;	// point back to Int 3
    if(kind == R_Over && b >= 0)
	ResetBreakpoint(b);	// reset only after GetBreakpoint did its thing
    if(tracing)
	return;

    // Find out whether a breakpoint stopped us.
    no = -1;
    if(i == 1 || int03hit) {
	BP_ENTRY *bep;

	for(b=0; b <= 3; b++)
	    if((edi.dr[6] & (1 << b)) && (edi.dr[7] & (3 << (b*2)))) {
		no = GetBreakpoint((BP_TYPE)((edi.dr[7] >> (16 + 4*b)) & 3),
		    ((edi.dr[7] >> (18 + 4*b)) & 3)+1, edi.dr[b]);
		break;
	    }
	if(no == -1)
	    no = GetBreakpoint(BP_Code, 0, a_tss.tss_eip);

	bep = breakpoint_table + no;
	if(no != -1 && bep->type == BP_Code) {
	    if(bep->condition) {
		char *errtxt;
		word32 val;

		if(evaluate(bep->condition, (long *)&val, &errtxt)) {
		    messageBox(mfError | mfOKButton,
			"Hit breakpoint with invalid condition");
		    goto error;
		}
		if(val == 0)
		    // We hit a breakpoint, but its condition is not satisfied.
		    goto retry;
	    }
	    if(--(bep->count) > 0)
		// We hit a breakpoint but it is not yet mature.
		goto retry;
	    bep->count = 0;
	}
    error:
	;
    }

//    code_pane_goto(a_tss.tss_eip);
    DebuggerScreen();

    if(terminated)
	messageBox(mfInformation | mfOKButton,
	    "Program terminated normally, exit code is %d",
	    (word8)a_tss.tss_eax);
    else if(i == 1 || int03hit) {
	if(no != -1)
	    switch(breakpoint_table[no].type) {
		case BP_Code:
		    if(!breakpoint_table[no].temporary) {
			if(breakpoint_table[no].condition)
			    messageBox(mfInformation | mfOKButton,
				"Condition \"%s\" satisfied",
				breakpoint_table[no].condition);
			else
			    messageBox("Code breakpoint hit",
				mfInformation | mfOKButton);
		    }
		    break;
		case BP_Write:
		    messageBox(mfInformation | mfOKButton,
			"Data write breakpoint at 0x%08lx triggered by previous instruction",
			breakpoint_table[no].addr);
		    break;
		case BP_Read:
		    messageBox(mfInformation | mfOKButton,
			"Data read/write breakpoint at 0x%08lx triggered by previous instruction",
			breakpoint_table[no].addr);
		    break;
	    }
	else if(i == 1 && (edi.dr[6] & (1 << 13)))
	    messageBox("User program used debug registers",
		mfError | mfOKButton);
	else if (i == 1 && (edi.dr[6] & (1 << 15)))
	    messageBox("Task switch caused debug exception",
		mfError | mfOKButton);
    } else if(i == 3 && !int03hit)
	messageBox("Unexpected Int 3 hit", mfError | mfOKButton);
    else if(i == 0x79 || i == 0x09)
	messageBox("Keyboard interrupt", mfInformation | mfOKButton);
    else if(i == 0x7a)
	messageBox("QUIT key pressed", mfInformation | mfOKButton);
    else if(i == 0x75) {
	char *reason;

	if((npx.status & 0x0241) == 0x0241)
	    reason = "stack overflow";
	else if((npx.status & 0x0241) == 0x0041)
	    reason = "stack underflow";
	else if(npx.status & 1)
	    reason = "invalid operation";
	else if(npx.status & 2)
	    reason = "denormal operand";
	else if(npx.status & 4)
	    reason = "divide by zero";
	else if(npx.status & 8)
	    reason = "overflow";
	else if(npx.status & 16)
	    reason = "underflow";
	else if(npx.status & 32)
	    reason = "loss of precision";
	else
	    reason = "?";
	messageBox(mfError | mfOKButton,
	    "Numeric Exception (%s) at eip=0x%08lx",
	    reason, npx.eip);
    } else if (i == 8 || (i >= 10 && i <= 14) || i == 17)
	messageBox(mfError | mfOKButton,
	    "Exception %d (0x%02x) occurred, error code=%#lx", i, i,
	    a_tss.tss_error);
    else
	messageBox(mfError | mfOKButton, "Exception %d (0x%02x) occurred", i,
	    i);
}

/* Assuming EIP, ESP, and EBP as values for the obvious registers, track
   back one stack from.  Return true on success.  */
bool Debugger::StackTrace(word32 &eip, word32 &esp, word32 &ebp)
{
    /* We look directly at the bit pattern instead of using disassembly;
       the bit patterns are those generated by gcc.  In general we
       cannot expect to be able to follow a non-gcc stack anyway.  */
    unsigned char eipcode[4];

    if(!IsValidInstAddr(eip))
	return false;

    read_child(eip, eipcode, 3);
    if((eipcode[0] == 0x55 && eipcode[1] == 0x89 && eipcode[2] == 0xe5)
	|| eipcode[0] == 0xc3) {
	/* In this case where we are looking at `Push Ebp//Mov Ebp,Esp'
	   or `Ret', only the return address is on the stack; I believe
	   this only to happen in the innermost activation record.  */
	if(!IsValidAddr(esp, 4))
	    return false;
	read_child(esp, &eip, 4);
	esp += 4;
	return true;
    } else {
	if(!(eipcode[0] == 0x89 && eipcode[1] == 0xe5)) {
	    // We are *not* looking at `Mov Ebp,Esp'.
	    if(ebp < 0x80000000UL && ebp >= esp && IsValidAddr(ebp, 8))
		esp = ebp;
	    else
		return false;
	} else
	    if(!IsValidAddr(esp, 8))
		return false;
	read_child(esp, &ebp, 4);
	esp += 4;
	read_child(esp, &eip, 4);
	esp += 4;
	return true;
    }
}

/* Trace execution path until a source point is reached, execution jumps,
   or LEVEL call levels is processed.  */
word32 Debugger::TraceTillSource(word32 eip, int level, bool mustmove)
{
    char *inst;
    unsigned char eipcode[5];
    int len;
    int line;

    while(IsValidInstAddr(eip) && (mustmove || !syms_val2line(eip, &line, 1)))
    {
	signed long offset;
	word32 tryaddr;

	mustmove = false;
	read_child(eip, eipcode, 5);
	switch(eipcode[0]) {
	    case 0xea:
		offset = *((signed char *)(eipcode + 1));
		tryaddr = eip + 2 + offset;
		if(offset >= 0) {
		    eip = tryaddr;
		    continue;
		}
		break;
	    case 0xe8:
		offset = *((signed long *)(eipcode + 1));
		tryaddr = eip + 5 + offset;
		if(offset >= 0) {
		    eip = tryaddr;
		    continue;
		}
		break;
	    case 0xe9:
		offset = *((signed long *)(eipcode + 1));
		tryaddr = eip + 5 + offset;
		break;
	    default:
		tryaddr = 0;
	}

	if(tryaddr && level && syms_val2line(tryaddr, &line, 0)) {
	    // recurse another level
	    tryaddr = TraceTillSource(tryaddr, level - 1, 0);
	    if(tryaddr)
		return tryaddr;
	}

	inst = disassembler->Proper(eip, len);
	if(strncmp (inst, "ret", 3) && strncmp (inst, "call", 4) &&
	    strncmp (inst, "int", 3) && strncmp (inst, "iret", 4) &&
	    strncmp (inst, "loop", 4) && inst[0] != 'j')
	    eip += len;
	else
	    break;
    }
    if(syms_val2line(eip, &line, 1))
	return eip;
    else
	return 0;
}

