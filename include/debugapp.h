/* CodeView/32 - TDebugApp Header File */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: debugapp.h,v 1.1 2001/02/02 17:02:43 pete Exp $ */

class TStatusLine;
class TMenuBar;
class TEvent;
class TPalette;
class fpstream;

class TDebugApp : public TApplication 
{
public:
    TDebugApp(int argc, char **argv);
    static TStatusLine *initStatusLine(TRect r);
    static TMenuBar *initMenuBar(TRect r);
    virtual void handleEvent(TEvent& Event);

private:
    void aboutDlgBox();               // "About" box
    void openFile(char *fileSpec);    // File Viewer
    void changeDir();                 // Change directory
};
