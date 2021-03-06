/* CodeView/32 - TDebugApp Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: debugapp.cpp,v 1.11 2001/04/28 21:37:34 pete Exp $ */

#include <stdio.h>
#include <string.h>
#include <debug/dbgcom.h>
#include <debug/v2load.h>
#include <debug/syms.h>

#define Uses_TView
#define Uses_TRect
#define Uses_MsgBox
#define Uses_fpstream
#define Uses_TEvent
#define Uses_TDeskTop
#define Uses_TPalette
#define Uses_MsgBox
#define Uses_TFileDialog
#define Uses_TApplication
#define Uses_TWindow
#define Uses_TStaticText
#define Uses_TDialog
#define Uses_TEventQueue
#define Uses_TChDirDialog
#define Uses_TButton

#include <tv.h>

#include "debugger.h"
#include "disassembler.h"
#include "debugapp.h"
#include "fileview.h"
#include "ldt.h"
#include "numproc.h"
#include "register.h"
#include "cvconst.h"

Debugger *debugger;
Disassembler *disassembler;

int main(int argc, char **argv)
{
    jmp_buf start_state;
    char cmdline[128];
    char *name = argv[1];
    char temp[256];
    FILE *f;
    int i;

    if(argc == 1) {
	fprintf(stderr, "Usage: %s debug-image\n", argv[0]);
	exit(1);
    }

    f = fopen(name, "rb");
    if(f) {
	fclose(f);
    } else {
	// append .exe and try again
	strcpy(temp, name);
	strcat(temp, ".exe");
	name = temp;
	f = fopen(name, "rb");
	if(f) {
	    fclose(f);
	} else {
	    fprintf(stderr, "Could not find %s or %s\n", argv[1], name);
	    exit(1);
	}
    }

    syms_init(name);

    // build command line
    cmdline[1] = 0;
    for(i=2; argv[i]; i++) {
	strcat(cmdline+1, " ");
	strcat(cmdline+1, argv[i]);
    }
    i = strlen(cmdline+1);
    cmdline[0] = i;
    cmdline[i+1] = 13;
    if(v2loadimage(name, cmdline, start_state)) {
	fprintf(stderr, "Load failed for image %s\n", argv[1]);
	exit(1);
    }

    edi_init(start_state);

    debugger = new Debugger();
    disassembler = new Disassembler();

    TDebugApp *debugProgram = new TDebugApp(argc, argv);

    debugProgram->run();

    TObject::destroy(debugProgram);

    delete disassembler;
    delete debugger;

    return 0;
}

extern "C" int valid_addr(word32 vaddr, int len)
{
    return (int)debugger->IsValidAddr(vaddr, len);
}

TDebugApp::TDebugApp(int argc, char **argv) :
    TProgInit(&TDebugApp::initStatusLine, &TDebugApp::initMenuBar,
	&TDebugApp::initDeskTop)
{
    TApplication::setScreenMode(0x503);	    // switch into 80x50 display mode
}

void TDebugApp::handleEvent(TEvent &event)
{
    TView *w;
    TRect r = TProgram::deskTop->getExtent();

    TApplication::handleEvent(event);

    if (event.what == evCommand) {
        switch (event.message.command) {
	    case cmAboutCmd:            //  About Dialog Box
                aboutDlgBox();
                break;

            case cmChDirCmd:            //  Change directory
                changeDir();
                break;

            case cmFileCmd:             //  View a file
                openFile("*.*");
                break;

	    case cmLDTCmd:
		w = validView(new TLDTWindow(TProgram::deskTop->getExtent()));
		if(w != 0) {
		    r.b.x = r.a.x + 47;
		    r.b.y = r.a.y + 20;
		    w->changeBounds(r);
		    deskTop->insert(w);
		}
		break;

	    case cmNumericProcessorCmd:
		w = validView(new TNumericProcessorWindow(TRect(r)));
		if(w != 0) {
		    r.a.y = r.b.y - 10;
		    w->changeBounds(r);
		    deskTop->insert(w);
		}
		break;

	    case cmRegistersCmd:
		w = validView(new TRegistersWindow(TRect(r)));
		if(w != 0) {
		    r.a.x = r.b.x - 19;
		    r.a.y = r.b.y - 18;
		    w->changeBounds(r);
		    deskTop->insert(w);
		}
		break;

            default:                    //  Unknown command
                return;

        }
	clearEvent (event);
    }
}

ushort executeDialog(TDialog *pD, void *data=0)
{
    ushort c = cmCancel;

    if(TProgram::application->validView(pD))
    {
        if (data)
	    pD->setData(data);
        c = TProgram::deskTop->execView(pD);
        if ((c != cmCancel) && (data))
            pD->getData(data);
        destroy(pD);
    }

    return c;
}

void TDebugApp::aboutDlgBox()
{
    TDialog *aboutBox = new TDialog(TRect(0, 0, 39, 11), "About");

    aboutBox->insert(
	new TStaticText(TRect(1, 2, 38, 7),
	    "\003CodeView/32\n\n\n\n"
	    "\003Version 1.0\n\n"
	    "\003Copyright (c) 2000-2001\n\n"
	    "\003Peter Johnson")
	);

    aboutBox->insert(
	new TButton(TRect(17, 8, 23, 10), "O~K~", cmOK, bfDefault)
	);

    aboutBox->options |= ofCentered;

    executeDialog(aboutBox);
}

void TDebugApp::openFile(char *fileSpec)
{
    TFileDialog *d = (TFileDialog *)validView(
	new TFileDialog(fileSpec, "Open a File", "~N~ame", fdOpenButton, 100));

    if(d != 0 && deskTop->execView(d) != cmCancel) {
        char fileName[PATH_MAX];
        d->getFileName(fileName);
        TView *w= validView(new TFileWindow(fileName));
        if(w != 0)
            deskTop->insert(w);
    }
    destroy(d);
}

void TDebugApp::changeDir()
{
    TView *d = validView(new TChDirDialog(0, cmChangeDir));

    if(d != 0) {
        deskTop->execView(d);
        destroy(d);
    }
}

