/* CodeView/32 - Status Bar Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: cvstatus.cpp,v 1.1 2001/02/02 17:02:43 pete Exp $ */
 
#define Uses_TRect
#define Uses_THintStatusLine
#define Uses_TStatusLine
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TKeys
#define Uses_TApplication

#include <tv.h>

#include "debugapp.h"
#include "cvhint.h"
#include "hintstat.h"
#include "cvconst.h"

TStatusLine *TDebugApp::initStatusLine(TRect r)
{
    r.a.y = r.b.y - 1;

    return (new THintStatusLine(r,
	*new TStatusDef(0, 50) +
	    *new TStatusItem("~Alt-X~ Exit", kbAltX, cmQuit) +
	    *new TStatusItem(0, kbAltF3, cmClose) +
	    *new TStatusItem(0, kbF10, cmMenu) +
	    *new TStatusItem(0, kbF5, cmZoom) +
	    *new TStatusItem(0, kbCtrlF5, cmResize)
	, CVHint));
}

