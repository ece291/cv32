/* CodeView/32 - TNumericProcessorViewer Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: numproc.cpp,v 1.1 2001/02/05 08:30:22 pete Exp $ */

#include <stdio.h>
#include <string.h>
#include <debug/dbgcom.h>

#define Uses_TListViewer
#define Uses_TScrollBar
#define Uses_TPalette
#define Uses_TWindow
#include <tv.h>

#include "numproc.h"

class TFPRegsViewer : public TListViewer
{
public:
    TFPRegsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
	TScrollBar *aVScrollBar);
    ~TFPRegsViewer() {};
    virtual TPalette &getPalette() const;
    virtual void getText(char *dest, ccIndex item, short maxLen);
};

TFPRegsViewer::TFPRegsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
    TScrollBar *aVScrollBar) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar)
{
    growMode = gfGrowHiX | gfGrowHiY;
    setRange(8);
}

/*
 01 Normal text (active)
 02 Normal text (not active)
 03 Focused text
 04 Selected text
*/

#define cpFPRegsViewer "\x06\x06\x07\x08"

TPalette &TFPRegsViewer::getPalette() const
{
  static TPalette pal(cpFPRegsViewer, sizeof(cpFPRegsViewer)-1);
  return pal;
}

void TFPRegsViewer::getText(char *dest, ccIndex item, short maxLen)
{
    /* Assume that `long double' is the same type as npx.reg[i].
	It would be sensible to check that the sizes match, but they
	don't!  For alignment reasons, `sizeof(long double)' is 12. */
    long double d;
    int tag;
    int tos = (npx.status >> 11) & 7;
    int exp = (int)npx.reg[item].exponent - 16382;
    char *dstr = (char *)alloca(30);
    char *buf = (char *)alloca(100);

    dstr[0] = (npx.reg[item].sign) ? '-' : '+';
    dstr[1] = '\0';
    tag = (npx.tag >> (((item + tos) & 7) * 2)) & 3;
    switch(tag) {
	case 0:
	    if(abs(exp) < 1000) {
		d = *((long double*)(npx.reg + item));
		sprintf(dstr, "%+.19Lg", d);
	    } else
		sprintf(dstr, "Valid, %s, and %s",
		    npx.reg[item].sign ? "negative" : "positive",
		    exp > 0 ? "huge" : "tiny");
	    break;
	case 1:
	    strcat(dstr, "Zero");
	    break;
	case 2:
	    if(npx.reg[item].exponent == 0x7fff) {
		if(npx.reg[item].sig3 == 0x8000
		    && npx.reg[item].sig2 == 0x0000
		    && npx.reg[item].sig1 == 0x0000
		    && npx.reg[item].sig0 == 0x0000)
		    strcat(dstr, "Inf");
		else
		    strcat(dstr, "NaN");
	    } else
		dstr = "Special";
	    break;
	case 3:
	    dstr = "Empty";
	    break;
    }
    sprintf(buf, "st%d: %d %04x %04x%04x%04x%04x %s", item,
	npx.reg[item].sign, npx.reg[item].exponent,
	npx.reg[item].sig3, npx.reg[item].sig2,
	npx.reg[item].sig1, npx.reg[item].sig0, dstr);
    strncpy(dest, buf, maxLen);
    dest[maxLen] = EOS;
}


class TFPFlagsViewer : public TListViewer
{
public:
    enum FlagsType { ControlFlags, StatusFlags };

    TFPFlagsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
	TScrollBar *aVScrollBar, FlagsType type);
    ~TFPFlagsViewer() {};
    virtual TPalette &getPalette() const;
    virtual void getText(char *dest, ccIndex item, short maxLen);

private:
    FlagsType type;
};

TFPFlagsViewer::TFPFlagsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
    TScrollBar *aVScrollBar, FlagsType type_) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    type(type_)
{
    int range = 0;

    growMode = gfGrowHiX | gfGrowHiY;
    switch(type_) {
	case ControlFlags:
	    range = 8;
	    break;
	case StatusFlags:
	    range = 9;
	    break;
    }

    setRange(range);
}

/*
 01 Normal text (active)
 02 Normal text (not active)
 03 Focused text
 04 Selected text
*/

#define cpFPFlagsViewer "\x06\x06\x07\x08"

TPalette &TFPFlagsViewer::getPalette() const
{
  static TPalette pal(cpFPFlagsViewer, sizeof(cpFPFlagsViewer)-1);
  return pal;
}

void TFPFlagsViewer::getText(char *dest, ccIndex item, short maxLen)
{
    unsigned long flags = 0;
    char *buf = (char *)alloca(100);
    static char *rtype[] = { "R", "-", "+", "0" };

    switch(type) {
	case ControlFlags:
	    flags = npx.control;
	    break;
	case StatusFlags:
	    flags = npx.status;
	    break;
    }

    switch(item) {
	case 0:
	    sprintf(buf, "%04lx", flags & 0xffff);
	    break;
	case 1:
	    sprintf(buf, "PR=%s", (flags & (1 << 5)) ? "N" : "Y");
	    break;
	case 2:
	    sprintf(buf, "UN=%s", (flags & (1 << 4)) ? "N" : "Y");
	    break;
	case 3:
	    sprintf(buf, "OV=%s", (flags & (1 << 3)) ? "N" : "Y");
	    break;
	case 4:
	    sprintf(buf, "ZD=%s", (flags & (1 << 2)) ? "N" : "Y");
	    break;
	case 5:
	    sprintf(buf, "DN=%s", (flags & (1 << 1)) ? "N" : "Y");
	    break;
	case 6:
	    sprintf(buf, "IV=%s", (flags & (1 << 0)) ? "N" : "Y");
	    break;
	case 7:
	    switch(type) {
		case ControlFlags:
		    sprintf(buf, "RD=%s", rtype[(flags >> 10) & 3]);
		    break;
		case StatusFlags:
		    sprintf(buf, "ST=%s", (flags & (1 << 6)) ? "Y" : "N");
		    break;
	    }
	    break;
	case 8:
	    if(type == StatusFlags)
		sprintf(buf, "C=%d%d%d%d",
		    (flags & (1 << 14)) != 0, (flags & (1 << 10)) != 0,
		    (flags & (1 <<  9)) != 0, (flags & (1 <<  8)) != 0);
	    break;
    }
    strncpy(dest, buf, maxLen);
    dest[maxLen] = EOS;
}

static short winNumber = 0;

TNumericProcessorWindow::TNumericProcessorWindow(const TRect &bounds) :
    TWindow(bounds, "Numeric Processor", winNumber++),
    TWindowInit(&TNumericProcessorWindow::initFrame)
{
    TRect r(bounds);
    TScrollBar *vs;
    TFPRegsViewer *regsviewer;

    r.grow(-1, -1);
    r.b.x -= 8+8;

    vs = new TScrollBar(TRect(r.b.x-1, r.a.y, r.b.x, r.b.y));
    insert(vs);

    r.b.x--;
    regsviewer = new TFPRegsViewer(r, 0, vs);
    insert(regsviewer);

    r.a.x = r.b.x+1;
    r.b.x = r.a.x+8;

    vs = new TScrollBar(TRect(r.b.x-1, r.a.y, r.b.x, r.b.y));
    insert(vs);

    r.b.x--;
    insert(new TFPFlagsViewer(r, 0, vs, TFPFlagsViewer::ControlFlags));

    r.a.x = r.b.x+1;
    r.b.x = size.x;

    vs = new TScrollBar(TRect(r.b.x-1, r.a.y, r.b.x, r.b.y));
    insert(vs);

    r.b.x--;
    insert(new TFPFlagsViewer(r, 0, vs, TFPFlagsViewer::StatusFlags));

    regsviewer->select();
}

void TNumericProcessorWindow::sizeLimits(TPoint &min, TPoint &max)
{
  TWindow::sizeLimits(min, max);
  int minx = 16 + 10 + 2;
  min.x = minx > min.x ? minx : min.x;
}

