/* CodeView/32 - TNumericProcessorWindow Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: numproc.cpp,v 1.6 2001/03/22 03:30:12 pete Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#define Uses_TDialog
#define Uses_TInputLine
#define Uses_TButton
#define Uses_TRadioButtons
#define Uses_TSItem
#include <tv.h>

#include "numproc.h"

const int cmFPRegsChangeCmd = 1101;
const int cmFPRegsNegateCmd = 1102;
const int cmFPRegsZeroCmd = 1103;
const int cmFPRegsEmptyCmd = 1104;
const int cmFPFlagsNextCmd = 1201;
const int cmFPFlagsToggleCmd = 1202;
const int cmFPFlagsSetCmd = 1203;
const int cmFPFlagsResetCmd = 1204;

const int hcFPRegsChangeCmd = 1101;
const int hcFPRegsNegateCmd = 1102;
const int hcFPRegsZeroCmd = 1103;
const int hcFPRegsEmptyCmd = 1104;
const int hcFPFlagsNextCmd = 1201;
const int hcFPFlagsToggleCmd = 1202;
const int hcFPFlagsSetCmd = 1203;
const int hcFPFlagsResetCmd = 1204;

class TFPRegsViewer : public TListViewer
{
public:
    TFPRegsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
	TScrollBar *aVScrollBar);
    ~TFPRegsViewer() {};
    virtual TPalette &getPalette() const;
    virtual void getText(char *dest, ccIndex item, short maxLen);
    virtual void handleEvent(TEvent &event);

private:
    void Popup(TPoint p);
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

#define cpFPRegsViewer "\x06\x06\x07\x06"

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

void TFPRegsViewer::handleEvent(TEvent &event)
{
    TEvent savedEvent(event);
    enum { Nothing, Empty, Zero, Negate, Change } action = Nothing;

    TListViewer::handleEvent(event);

    if(savedEvent.what == evMouseDown) {
	if(savedEvent.mouse.buttons & mbRightButton) {
	    Popup(savedEvent.mouse.where);
	    return;
	}
    } else switch(event.what) {
	case evKeyDown:
	    switch(event.keyDown.keyCode) {
		case kbCtrlC:
		case kbCtrlE:
		case kbDel:
		    action = Empty;
		    break;
		case kbCtrlZ:
		    action = Zero;
		    break;
		case kbCtrlN:
		    action = Negate;
		    break;
		case kbEnter:
		    action = Change;
		    break;
		default:
		    return;
	    }
	    break;
	case evCommand:
	    switch(event.message.command) {
		case cmFPRegsChangeCmd:
		    action = Change;
		    break;
		case cmFPRegsNegateCmd:
		    action = Negate;
		    break;
		case cmFPRegsZeroCmd:
		    action = Zero;
		    break;
		case cmFPRegsEmptyCmd:
		    action = Empty;
		    break;
		default:
		    return;
	    }
	    break;
	default:
	    return;
    }

    int rotreg = (focused + (npx.status >> 11)) & 7;
    int tag = (npx.tag >> (rotreg * 2)) & 3;

    switch(action) {
	case Empty:
	    tag = 3;
	    memset (&npx.reg[focused], 0, sizeof (NPXREG));
	    break;
	case Zero:
	    tag = 1;
	    memset (&npx.reg[focused], 0, sizeof (NPXREG));
	    break;
	case Negate:
	    npx.reg[focused].sign = !npx.reg[focused].sign;
	    break;
	case Change:
	    {
	    static char textval[256];
	    static unsigned int typeval = 0;

	    // Build dialog box.  We build it ourselves instead of just
	    // calling inputBox() because we want a selection box between
	    // floating point and MMX values.
	    TRect r(0, 0, 60, 8);
	    r.move((TProgram::deskTop->size.x - r.b.x) / 2,
		(TProgram::deskTop->size.y - r.b.y) / 2);

	    TDialog *dialog = new TDialog(r, "Editing Numeric Processor value");

	    r = TRect(9 + strlen("Floating Point"), 2, dialog->size.x - 3, 3);
	    TInputLine *input = new TInputLine(r, 256);
	    dialog->insert(input);

	    r = TRect(2, 2, 8 + strlen("Floating Point"), 4);
	    TRadioButtons *type = new TRadioButtons(r,
		new TSItem("Floating Point", new TSItem("MMX (Hex)", 0)));
	    dialog->insert(type);

	    r = TRect(dialog->size.x - 24, dialog->size.y - 4,
		dialog->size.x - 14, dialog->size.y - 2);
	    dialog->insert(new TButton(r, "~O~K", cmOK, bfDefault));

	    r.a.x += 12;
	    r.b.x += 12;
	    dialog->insert(new TButton(r, "Cancel", cmCancel, bfNormal));

	    r.a.x += 12;
	    r.b.x += 12;
	    dialog->selectNext(False);

	    input->setData(textval);
	    type->setData(&typeval);

	    if(TProgram::deskTop->execView(dialog) == cmOK) {
		input->getData(textval);
		type->getData(&typeval);

		switch(typeval) {
		    case 0:	// floating point
			{
			char *endp, *p;
			long double d;

			p = textval;
			while (*p == ' ')
			    p++;
			if (*p == '\0')
			    break;
			strlwr (p);
			if (strcmp (p, "+inf") == 0 ||
			    strcmp (p, "inf") == 0 ||
			    strcmp (p, "-inf") == 0) {
			    tag = 2;
			    npx.reg[focused].exponent = 0x7fff;
			    npx.reg[focused].sig3 = 0x8000;
			    npx.reg[focused].sig2 =
				npx.reg[focused].sig1 =
				npx.reg[focused].sig0 = 0;
			    npx.reg[focused].sign = (*p == '-');
			} else {
			    d = _strtold (p, &endp);
			    if (*p != '\0' && *endp)
				messageBox("Expression not understood",
				    mfError | mfOKButton);
			    else {
				tag = (d == 0.0);
				*((long double *)(npx.reg + focused)) = d;
				npx.reg[focused].sign = (*p == '-');
			    }
			}
			break;
			}
		    case 1:	// MMX (hex)
			{
			bool valid = true;
			unsigned long long val = 0;
			char *p = textval;
			int numdigits = 0;

			while(*p != '\0') {
			    switch(*p) {
				case ' ':
				    break;	// ignore spaces
				case '0' ... '9':
				    val <<= 4;
				    val |= (unsigned long long)(*p-'0');
				    numdigits++;
				    break;
				case 'a' ... 'f':
				    val <<= 4;
				    val |= (unsigned long long)(*p-'a'+10);
				    numdigits++;
				    break;
				case 'A' ... 'F':
				    val <<= 4;
				    val |= (unsigned long long)(*p-'A'+10);
				    numdigits++;
				    break;
				default:	// error on any other char
				    valid = false;
				    *(p+1) = '\0';
				    break;
			    }
			    p++;
			}
			if(numdigits > 16) {
			    messageBox("Too many digits",
				mfError | mfOKButton);
			} else if(!valid) {
			    messageBox("Invalid character",
				mfError | mfOKButton);
			} else {
			    tag = 0;
			    npx.tag = 0;	// MMX write clears FP tag word
			    npx.reg[focused].sign = 1;
			    npx.reg[focused].exponent = 0x7fff;
			    npx.reg[focused].sig3 = (val >> 48) & 0xffff;
			    npx.reg[focused].sig2 = (val >> 32) & 0xffff;
			    npx.reg[focused].sig1 = (val >> 16) & 0xffff;
			    npx.reg[focused].sig0 = val & 0xffff;
			}
			break;
			}
		}
	    }
	    TObject::destroy(dialog);
	    break;
	    }
	case Nothing:
	    return;
    }

    npx.tag = (npx.tag & ~(3 << (rotreg * 2))) | (tag << (rotreg * 2));

    draw();
    clearEvent(event);
}

void TFPRegsViewer::Popup(TPoint p)
{
    TMenu *theMenu = new TMenu(
	*new TMenuItem("~C~hange...", cmFPRegsChangeCmd, kbEnter, hcFPRegsChangeCmd, "Enter",
	new TMenuItem("~N~egate", cmFPRegsNegateCmd, kbCtrlN, hcFPRegsNegateCmd, "Ctrl-N",
	new TMenuItem("~Z~ero", cmFPRegsZeroCmd, kbCtrlZ, hcFPRegsZeroCmd, "Ctrl-Z",
	new TMenuItem("~E~mpty", cmFPRegsEmptyCmd, kbDel, hcFPRegsEmptyCmd, "Del"
    )))));

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


class TFPFlagsViewer : public TListViewer
{
public:
    enum FlagsType { ControlFlags, StatusFlags };

    TFPFlagsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
	TScrollBar *aVScrollBar, FlagsType type);
    ~TFPFlagsViewer() {};
    virtual TPalette &getPalette() const;
    virtual void getText(char *dest, ccIndex item, short maxLen);
    virtual void handleEvent(TEvent &event);

private:
    FlagsType type;

    void Popup(TPoint p);
};

TFPFlagsViewer::TFPFlagsViewer(const TRect &bounds, TScrollBar *aHScrollBar,
    TScrollBar *aVScrollBar, FlagsType type_) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    type(type_)
{
    int range = 0;

    growMode = gfGrowLoX | gfGrowHiX | gfGrowHiY;
    switch(type_) {
	case ControlFlags:
	    range = 7;
	    break;
	case StatusFlags:
	    range = 8;
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

#define cpFPFlagsViewer "\x06\x06\x07\x06"

TPalette &TFPFlagsViewer::getPalette() const
{
  static TPalette pal(cpFPFlagsViewer, sizeof(cpFPFlagsViewer)-1);
  return pal;
}

void TFPFlagsViewer::getText(char *dest, ccIndex item, short maxLen)
{
    unsigned long flags = 0;
    char *buf = (char *)alloca(100);
    static char rtype[] = { 'R', '-', '+', '0' };
    char flagchar[] = { ' ', ' ' };

    switch(type) {
	case ControlFlags:
	    flags = npx.control;
	    flagchar[0] = 'N';
	    flagchar[1] = 'Y';
	    break;
	case StatusFlags:
	    flags = npx.status;
	    flagchar[0] = 'Y';
	    flagchar[1] = 'N';
	    break;
    }

    switch(item) {
	case 0:
	    sprintf(buf, "PR=%c", flagchar[(flags >> 5) & 1]);
	    break;
	case 1:
	    sprintf(buf, "UN=%c", flagchar[(flags >> 4) & 1]);
	    break;
	case 2:
	    sprintf(buf, "OV=%c", flagchar[(flags >> 3) & 1]);
	    break;
	case 3:
	    sprintf(buf, "ZD=%c", flagchar[(flags >> 2) & 1]);
	    break;
	case 4:
	    sprintf(buf, "DN=%c", flagchar[(flags >> 1) & 1]);
	    break;
	case 5:
	    sprintf(buf, "IV=%c", flagchar[flags & 1]);
	    break;
	case 6:
	    switch(type) {
		case ControlFlags:
		    sprintf(buf, "RD=%c", rtype[(flags >> 10) & 3]);
		    break;
		case StatusFlags:
		    sprintf(buf, "ST=%c", flagchar[(flags >> 6) & 1]);
		    break;
	    }
	    break;
	case 7:
	    if(type == StatusFlags)
		sprintf(buf, "C=%d%d%d%d",
		    (flags & (1 << 14)) != 0, (flags & (1 << 10)) != 0,
		    (flags & (1 <<  9)) != 0, (flags & (1 <<  8)) != 0);
	    break;
    }
    strncpy(dest, buf, maxLen);
    dest[maxLen] = EOS;
}

void TFPFlagsViewer::handleEvent(TEvent &event)
{
    TEvent savedEvent(event);
    enum { Nothing, Next, Toggle, Set, Reset } action = Nothing;

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
		    if(type == ControlFlags && focused == 6)
			action = Next;
		    else if(focused <= 6)
			action = Toggle;
		    else
			return;
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
		case cmFPFlagsNextCmd:
		    action = Next;
		    break;
		case cmFPFlagsToggleCmd:
		    action = Toggle;
		    break;
		case cmFPFlagsSetCmd:
		    action = Set;
		    break;
		case cmFPFlagsResetCmd:
		    action = Reset;
		    break;
		default:
		    return;
	    }
	    break;
	default:
	    return;
    }

    unsigned long *flags = 0;
    static char flagpos[] = {5, 4, 3, 2, 1, 0, 6};

    switch(type) {
	case ControlFlags:
	    flags = &npx.control;
	    break;
	case StatusFlags:
	    flags = &npx.status;
	    break;
    }

    switch(action) {
	case Next:
	    {
	    int val;
	    val = (*flags >> 10) & 3;
	    val = (val+1) & 3;
	    *flags &= ~(3 << 10);
	    *flags |= val << 10;
	    break;
	    }
	case Toggle:
	    *flags ^= 1 << flagpos[focused];
	    break;
	case Set:
	    if(type == StatusFlags)
		*flags &= ~(1 << flagpos[focused]);
	    else
		*flags |= 1 << flagpos[focused];
	    break;
	case Reset:
	    if(type == StatusFlags)
		*flags |= 1 << flagpos[focused];
	    else
		*flags &= ~(1 << flagpos[focused]);
	    break;
	case Nothing:
	    return;
    }

    draw();
    clearEvent(event);
}

void TFPFlagsViewer::Popup(TPoint p)
{
    TMenuItem *items;

    if(type == ControlFlags && focused == 6)
	items = new TMenuItem("~N~ext", cmFPFlagsNextCmd, kbEnter, hcFPFlagsNextCmd, "Enter");
    else if(focused <= 6)
	items = new TMenuItem("~T~oggle", cmFPFlagsToggleCmd, kbEnter, hcFPFlagsToggleCmd, "Enter");
    else
	return;

    items->append(
	new TMenuItem("~S~et", cmFPFlagsSetCmd, 0, hcFPFlagsSetCmd, "S",
	new TMenuItem("~R~eset", cmFPFlagsResetCmd, 0, hcFPFlagsResetCmd, "R"
    )));

    TMenu *theMenu = new TMenu(*items);

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
    regsviewer = new TFPRegsViewer(TRect(r), 0, 0);
    insert(regsviewer);

    r.a.x = r.b.x+1;
    r.b.x = r.a.x+8;

    vs = new TScrollBar(TRect(r.b.x-1, r.a.y, r.b.x, r.b.y));
    insert(vs);

    r.b.x--;
    insert(new TFPFlagsViewer(TRect(r), 0, 0, TFPFlagsViewer::ControlFlags));

    r.a.x = r.b.x+1;
    r.b.x = size.x;

//    vs = new TScrollBar(TRect(r.b.x-1, r.a.y, r.b.x, r.b.y));
//    insert(vs);

    r.b.x--;
    insert(new TFPFlagsViewer(TRect(r), 0, 0, TFPFlagsViewer::StatusFlags));

    regsviewer->select();
}

void TNumericProcessorWindow::sizeLimits(TPoint &min, TPoint &max)
{
  TWindow::sizeLimits(min, max);
  int minx = 16 + 10 + 2;
  min.x = minx > min.x ? minx : min.x;
  min.y = 10;
}

