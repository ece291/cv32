/* CodeView/32 - TRegistersWindow Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: register.cpp,v 1.1 2001/03/22 06:14:03 pete Exp $ */

#include <stdio.h>
#include <string.h>
#include <debug/dbgcom.h>

#define Uses_TKeys
#define Uses_TEvent
#define Uses_TMenuItem
#define Uses_TMenu
#define Uses_TMenuBox
#define Uses_TListViewer
#define Uses_TScrollBar
#define Uses_TPalette
#define Uses_TWindow
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TPoint
#define Uses_MsgBox
#include <tv.h>

#include "register.h"

const int cmRegsChangeCmd = 1301;
const int cmRegsIncrementCmd = 1302;
const int cmRegsDecrementCmd = 1303;
const int cmRegsNegateCmd = 1304;
const int cmRegsZeroCmd = 1305;
const int cmFlagsToggleCmd = 1401;
const int cmFlagsSetCmd = 1402;
const int cmFlagsResetCmd = 1403;

const int hcRegsChangeCmd = 1301;
const int hcRegsIncrementCmd = 1302;
const int hcRegsDecrementCmd = 1303;
const int hcRegsNegateCmd = 1304;
const int hcRegsZeroCmd = 1305;
const int hcFlagsToggleCmd = 1401;
const int hcFlagsSetCmd = 1402;
const int hcFlagsResetCmd = 1403;

class TRegsViewer : public TListViewer
{
public:
    TRegsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
	TScrollBar *aVScrollBar);
    ~TRegsViewer() {};
    virtual TPalette &getPalette() const;
    virtual void getText(char *dest, ccIndex item, short maxLen);
    virtual void handleEvent(TEvent &event);

private:
    static const char * const regs_names[];
    static unsigned long * const regs_addr[];
    static const char regs_type[];

    void Popup(TPoint p);
    unsigned long SetRegister(int reg, bool rel, unsigned long val);
};

const char * const TRegsViewer::regs_names[] = {
    "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp",
    "cs",   "ds",  "es",  "fs",  "gs",  "ss",
    "eip", "flg" };
unsigned long * const TRegsViewer::regs_addr[] = {
    &a_tss.tss_eax, &a_tss.tss_ebx, &a_tss.tss_ecx, &a_tss.tss_edx,
    &a_tss.tss_esi, &a_tss.tss_edi, &a_tss.tss_ebp, &a_tss.tss_esp,
    (unsigned long *)&a_tss.tss_cs, (unsigned long *)&a_tss.tss_ds,
    (unsigned long *)&a_tss.tss_es, (unsigned long *)&a_tss.tss_fs,
    (unsigned long *)&a_tss.tss_gs, (unsigned long *)&a_tss.tss_ss,
    &a_tss.tss_eip, &a_tss.tss_eflags };
/* g: general, !: special, s: segment, f: flags, \0: end-of-table */
const char TRegsViewer::regs_type[] = "ggggggg!ssssss!f";

TRegsViewer::TRegsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
    TScrollBar *aVScrollBar) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar)
{
    growMode = gfGrowHiX | gfGrowHiY;
    setRange(16);
}

/*
 01 Normal text (active)
 02 Normal text (not active)
 03 Focused text
 04 Selected text
*/

#define cpRegsViewer "\x06\x06\x07\x06"

TPalette &TRegsViewer::getPalette() const
{
  static TPalette pal(cpRegsViewer, sizeof(cpRegsViewer)-1);
  return pal;
}

void TRegsViewer::getText(char *dest, ccIndex item, short maxLen)
{
    char *buf = (char *)alloca(100);

    if (regs_type[item] == 's')
	sprintf(buf, "  %s %04x", regs_names[item],
	    *(unsigned short *)(regs_addr[item]));
    else
	sprintf(buf, "%s %08lx", regs_names[item], *(regs_addr[item]));

    strncpy(dest, buf, maxLen);
    dest[maxLen] = EOS;
}

void TRegsViewer::handleEvent(TEvent &event)
{
    TEvent savedEvent(event);
    enum { Nothing, Change, Increment, Decrement, Negate, Zero } action =
	Nothing;

    TListViewer::handleEvent(event);

    if(savedEvent.what == evMouseDown) {
	if(savedEvent.mouse.buttons & mbRightButton) {
	    Popup(savedEvent.mouse.where);
	    return;
	}
    } else switch(event.what) {
	case evKeyDown:
	    switch(event.keyDown.keyCode) {
		case kbEnter:
		    action = Change;
		    break;
		case kbCtrlI:
		    action = Increment;
		    break;
		case kbCtrlD:
		    action = Decrement;
		    break;
		case kbCtrlN:
		    action = Negate;
		    break;
		case kbCtrlZ:
		    action = Zero;
		    break;
		default:
		    return;
	    }
	    break;
	case evCommand:
	    switch(event.message.command) {
		case cmRegsChangeCmd:
		    action = Change;
		    break;
		case cmRegsIncrementCmd:
		    action = Increment;
		    break;
		case cmRegsDecrementCmd:
		    action = Decrement;
		    break;
		case cmRegsNegateCmd:
		    action = Negate;
		    break;
		case cmRegsZeroCmd:
		    action = Zero;
		    break;
		default:
		    return;
	    }
	    break;
	default:
	    return;
    }

    switch(action) {
	case Change:
	    break;
	case Increment:
	    SetRegister(focused, true, +1);
	    break;
	case Decrement:
	    SetRegister(focused, true, -1);
	    break;
	case Negate:
	    SetRegister(focused, false, -SetRegister(focused, 1, 0));
	    break;
	case Zero:
	    SetRegister(focused, false, 0);
	    break;
	case Nothing:
	    return;
    }

    draw();
    clearEvent(event);
}

void TRegsViewer::Popup(TPoint p)
{
    TMenu *theMenu = new TMenu(
	*new TMenuItem("~C~hange...", cmRegsChangeCmd, kbEnter, hcRegsChangeCmd, "Enter",
	new TMenuItem("~I~ncrement", cmRegsIncrementCmd, kbCtrlI, hcRegsIncrementCmd, "Ctrl-I",
	new TMenuItem("~D~ecrement", cmRegsDecrementCmd, kbCtrlD, hcRegsDecrementCmd, "Ctrl-D",
	new TMenuItem("~N~egate", cmRegsNegateCmd, kbCtrlN, hcRegsNegateCmd, "Ctrl-N",
	new TMenuItem("~Z~ero", cmRegsZeroCmd, kbCtrlZ, hcRegsZeroCmd, "Ctrl-Z"
    ))))));

    TMenuBox *popup = new TMenuBox(TRect(0, 0, 0, 0), theMenu, 0);

    // make sure it doesn't go off the screen
    TRect rp = popup->getExtent();
    TRect rd = TProgram::deskTop->getExtent();
    if(p.x+rp.b.x > rd.b.x)
	p.x = rd.b.x - rp.b.x;
    if(p.y+rp.b.y > rd.b.y)
	p.y = rd.b.y - rp.b.y;
    popup->moveTo(p.x, p.y);

    int result = 0;
    if(TProgram::application->validView(popup)) {
	result = TProgram::deskTop->execView(popup);
	destroy(popup);
    }

    delete theMenu;

    // Create an event
    if(result) {
	TEvent event;
	event.what = evCommand;
	event.message.command = result;
	putEvent(event);
	clearEvent(event);
    }
}

unsigned long TRegsViewer::SetRegister(int reg, bool rel, unsigned long val)
{
    switch (regs_type[reg]) {
	case 's':
	    if(rel)
		val += *(unsigned short *)(regs_addr[reg]);
	    return *(unsigned short *)(regs_addr[reg]) = (unsigned short)val;
	case 'f':
	    if(rel)
		val += *(regs_addr[reg]);
	    return a_tss.tss_eflags =
		(a_tss.tss_eflags & ~0xed5) | (val & 0xed5);
	default:
	    if(rel)
		val += *(regs_addr[reg]);
	    return *regs_addr[reg] = val;
    }
}


class TFlagsViewer : public TListViewer
{
public:
    TFlagsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
	TScrollBar *aVScrollBar);
    ~TFlagsViewer() {};
    virtual TPalette &getPalette() const;
    virtual void getText(char *dest, ccIndex item, short maxLen);
    virtual void handleEvent(TEvent &event);

private:
    static const char flag_names[];
    static const unsigned flag_bits[];

    void Popup(TPoint p);
};

const char TFlagsViewer::flag_names[] = "czsopaid";
const unsigned TFlagsViewer::flag_bits[] =
    { 0x0001, 0x0040, 0x0080, 0x0800, 0x0004, 0x0010, 0x0200, 0x0400 };

TFlagsViewer::TFlagsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
    TScrollBar *aVScrollBar) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar)
{
    growMode = gfGrowLoX | gfGrowHiX | gfGrowHiY;

    setRange(8);
}

/*
 01 Normal text (active)
 02 Normal text (not active)
 03 Focused text
 04 Selected text
*/

#define cpFlagsViewer "\x06\x06\x07\x06"

TPalette &TFlagsViewer::getPalette() const
{
  static TPalette pal(cpFlagsViewer, sizeof(cpFlagsViewer)-1);
  return pal;
}

void TFlagsViewer::getText(char *dest, ccIndex item, short maxLen)
{
    char *buf = (char *)alloca(100);

    sprintf(buf, "%c=%d", flag_names[item],
	(a_tss.tss_eflags & flag_bits[item]) != 0);

    strncpy(dest, buf, maxLen);
    dest[maxLen] = EOS;
}

void TFlagsViewer::handleEvent(TEvent &event)
{
    TEvent savedEvent(event);
    enum { Nothing, Toggle, Set, Reset } action = Nothing;

    TListViewer::handleEvent(event);

    if(savedEvent.what == evMouseDown) {
	if(savedEvent.mouse.buttons & mbRightButton) {
	    Popup(savedEvent.mouse.where);
	    return;
	}
    } else switch(event.what) {
	case evKeyDown:
	    switch(event.keyDown.keyCode) {
		case kbEnter:
		    action = Toggle;
		    break;
		default:
		    switch(event.keyDown.charScan.charCode) {
			case '+':
			case '-':
			case 't':
			case 'T':
			    action = Toggle;
			    break;
			case '1':
			case 's':
			case 'S':
			    action = Set;
			    break;
			case '0':
			case 'r':
			case 'R':
			    action = Reset;
			    break;
			default:
			    return;
		    }
		    break;
	    }
	    break;
	case evCommand:
	    switch(event.message.command) {
		case cmFlagsToggleCmd:
		    action = Toggle;
		    break;
		case cmFlagsSetCmd:
		    action = Set;
		    break;
		case cmFlagsResetCmd:
		    action = Reset;
		    break;
		default:
		    return;
	    }
	    break;
	default:
	    return;
    }

    switch(action) {
	case Toggle:
	    a_tss.tss_eflags ^= flag_bits[focused];
	    break;
	case Set:
	    a_tss.tss_eflags |= flag_bits[focused];
	    break;
	case Reset:
	    a_tss.tss_eflags &= ~flag_bits[focused];
	    break;
	case Nothing:
	    return;
    }

    draw();
    clearEvent(event);
}

void TFlagsViewer::Popup(TPoint p)
{
    TMenu *theMenu = new TMenu(
	*new TMenuItem("~T~oggle", cmFlagsToggleCmd, kbEnter, hcFlagsToggleCmd, "Enter",
	new TMenuItem("~S~et", cmFlagsSetCmd, 0, hcFlagsSetCmd, "S",
	new TMenuItem("~R~eset", cmFlagsResetCmd, 0, hcFlagsResetCmd, "R"
    ))));

    TMenuBox *popup = new TMenuBox(TRect(0, 0, 0, 0), theMenu, 0);

    // make sure it doesn't go off the screen
    TRect rp = popup->getExtent();
    TRect rd = TProgram::deskTop->getExtent();
    if(p.x+rp.b.x > rd.b.x)
	p.x = rd.b.x - rp.b.x;
    if(p.y+rp.b.y > rd.b.y)
	p.y = rd.b.y - rp.b.y;
    popup->moveTo(p.x, p.y);

    int result = 0;
    if(TProgram::application->validView(popup)) {
	result = TProgram::deskTop->execView(popup);
	destroy(popup);
    }

    delete theMenu;

    // Create an event
    if(result) {
	TEvent event;
	event.what = evCommand;
	event.message.command = result;
	putEvent(event);
	clearEvent(event);
    }
}


static short winNumber = 0;

TRegistersWindow::TRegistersWindow(const TRect &bounds) :
    TWindow(bounds, "Regs", winNumber++),
    TWindowInit(&TRegistersWindow::initFrame)
{
    TRect r(bounds);
    TScrollBar *vs;
    TRegsViewer *regsviewer;

    r.grow(-1, -1);
    r.b.x -= 3;

    vs = new TScrollBar(TRect(r.b.x-1, r.a.y, r.b.x, r.b.y));

    r.b.x--;
    regsviewer = new TRegsViewer(TRect(r), 0, vs);
    insert(regsviewer);

    r.a.x = r.b.x;
    r.b.x = size.x;

//    vs = new TScrollBar(TRect(r.b.x-1, r.a.y, r.b.x, r.b.y));
//    insert(vs);

    r.b.x--;
    insert(new TFlagsViewer(TRect(r), 0, 0));

    insert(vs);

    regsviewer->select();
}

void TRegistersWindow::sizeLimits(TPoint &min, TPoint &max)
{
    TWindow::sizeLimits(min, max);
    int minx = 13 + 4 + 2;
    min.x = minx > min.x ? minx : min.x;
    min.y = 8;
}

