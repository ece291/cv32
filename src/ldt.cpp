/* CodeView/32 - TLDTViewer Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: ldt.cpp,v 1.1 2001/02/03 23:43:38 pete Exp $ */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <debug/dbgcom.h>

#define Uses_TNSCollection
#define Uses_TListViewer
#define Uses_TScroller
#define Uses_TPalette
#define Uses_TWindow
#include <tv.h>

#include "ldt.h"

typedef unsigned short word16;
typedef unsigned char word8;

typedef struct GDT_S {
    word16 lim0;
    word16 base0;
    word8 base1;
    word8 stype;    /* type, DT, DPL, present */
    word8 lim1;	    /* limit, granularity */
    word8 base2;
} GDT_S;

typedef struct IDT_S {
    word16 offset0;
    word16 selector;
    word16 stype;
    word16 offset1;
} IDT;

TLDTViewer::TLDTViewer(const TRect &bounds, TScrollBar *aHScrollBar,
    TScrollBar *aVScrollBar) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar)
{
    int count;
#if 0
    int ldt;

    asm volatile
	("xorl  %%eax,%%eax
	sldt  %%ax
	movl  %%eax,%0"
	: "=r" ((int)(ldt))
	: /* no inputs.  */
	: "%eax");

    if (ldt / 8 == 0) {
	LDTExists = false;
	count = 1;
    } else {
#endif
	LDTExists = true;
	count = 0x400 / 8;
//    }

    growMode = gfGrowHiX | gfGrowHiY;
    setRange(count);
}

/*
 01 Normal text (active)
 02 Normal text (not active)
 03 Focused text
 04 Selected text
*/

#define cpLDTViewer "\x06\x06\x07\x08"

TPalette &TLDTViewer::getPalette() const
{
  static TPalette pal(cpLDTViewer, sizeof(cpLDTViewer)-1);
  return pal;
}

void TLDTViewer::getText(char *dest, ccIndex item, short maxLen)
{
    if(!LDTExists) {
	if(item == 0) {
	    strncpy(dest, "There is no local descriptor table.", maxLen);
	    dest[maxLen] = EOS;
	} else
	    *dest = EOS;
    } else {
	char *buf = (char *)alloca(100);
	char *buf1;
	GDT_S descr;

	if (getLDTDescriptor(item*8 | ((a_tss.tss_cs & 3) | 4), &descr) < 0)
	    buf1 = "Read failure.";
	else {
	    buf1 = (char *)alloca (80);
	    describeDescriptor (buf1, &descr);
	}
	sprintf(buf, "%03x: %s", item*8 | ((a_tss.tss_cs & 3) | 4), buf1);
	strncpy(dest, buf, maxLen);
	dest[maxLen] = EOS;
    }
}

int TLDTViewer::getLDTDescriptor(int no, void *descr)
{
    union REGS regs;
    struct SREGS sregs;

    regs.w.ax = 0x000B;
    regs.w.bx = no;
    sregs.cs = _go32_my_cs ();
    sregs.ds = _go32_my_ds ();
    sregs.es = _go32_my_ds ();
    sregs.fs = _go32_my_ds ();
    sregs.gs = _go32_my_ds ();
    regs.d.edi = (unsigned)descr;
    int86x (0x31, &regs, &regs, &sregs);
    if (regs.w.cflag)
	return -1;
    return ((GDT_S *)descr)->stype & 0x1f;
}

/* At BUF place the textual representation of DESCR.  */

void TLDTViewer::describeDescriptor(char *buf, void *descr)
{
    GDT_S *gdtp = (GDT_S *)descr;
    int typ = gdtp->stype & 0x1f;
    signed long limit = ((gdtp->lim1 & 0x0f) << 16) | gdtp->lim0;

    if (gdtp->lim1 & 0x80)
	limit = (limit << 12) | 0xfff;

    if ((gdtp->stype & 0x80) == 0)
	sprintf (buf, "Not present.");
    else if (((0xffffdafaL >> typ) & 1) == 0)	// hardwired to LDT
	sprintf (buf, "Invalid in context (%02x)", typ);
    else {
	switch (typ) {
	    case 0x10 ... 0x17:
		sprintf (buf, "%s-bit data %02x%02x%04x+%08lx %s %s %s",
		    gdtp->lim1 & 0x40 ? "32" : "16",
		    gdtp->base2, gdtp->base1, gdtp->base0,
		    limit,
		    typ & 0x04 ? "E\031" : "E\030",
		    typ & 0x02 ? "RW" : "RO",
		    typ & 0x01 ? "A+" : "A-");
		break;
	    case 0x18 ... 0x1f:
		sprintf (buf, "%s-bit code %02x%02x%04x+%08lx %s %s %s",
		    gdtp->lim1 & 0x40 ? "32" : "16",
		    gdtp->base2, gdtp->base1, gdtp->base0,
		    limit,
		    typ & 0x04 ? "C+" : "C-",
		    typ & 0x02 ? "R+" : "R-",
		    typ & 0x01 ? "A+" : "A-");
		break;
	    default:
		sprintf (buf, "Invalid (%02x)", typ);
	}
    }
}

static short winNumber = 0;

TLDTWindow::TLDTWindow(const TRect &bounds) :
    TWindow(bounds, "LDT", winNumber++),
    TWindowInit(&TLDTWindow::initFrame)
{
    TRect r(bounds);
    r.grow(-1, -1);
    insert(new TLDTViewer(r, 0,
        standardScrollBar(sbVertical | sbHandleKeyboard)));
}

