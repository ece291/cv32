/* CodeView/32 - TDebugApp Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: debugapp.cpp,v 1.1 2001/02/02 17:02:43 pete Exp $ */
 
#include <string.h>

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

#include "debugapp.h"
#include "fileview.h"
#include "cvconst.h"

int main(int argc, char **argv)
{
    TDebugApp *debugProgram = new TDebugApp(argc, argv);

    debugProgram->run();

    TObject::destroy(debugProgram);
    return 0;
}


TDebugApp::TDebugApp(int argc, char **argv) :
    TProgInit(&TDebugApp::initStatusLine, &TDebugApp::initMenuBar,
	&TDebugApp::initDeskTop)
{
    char fileSpec[128];
    int len;

    while(--argc > 0) {
        strcpy( fileSpec, *++argv );
        len = strlen( fileSpec );

        if(fileSpec[len-1] == '/')
            strcat(fileSpec, "*.*");
        if(strchr(fileSpec, '*') || strchr(fileSpec, '?'))
            openFile(fileSpec);
        else {
	    TView *w;
            w = validView(new TFileWindow(fileSpec));
            if(w != 0)
                deskTop->insert(w);
        }
    }
}

void TDebugApp::handleEvent(TEvent &event)
{
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

