/* CodeView/32 - TLDTViewer Header File */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: ldt.h,v 1.1 2001/02/03 23:43:38 pete Exp $ */

#if !defined( __LDTVIEWER_H )
#define __LDTVIEWER_H

#define Uses_TListViewer
#define Uses_TWindow
#include <tv.h>

class TPalette;

class TLDTViewer : public TListViewer
{
private:
    bool LDTExists;

public:
    TLDTViewer(const TRect &bounds, TScrollBar *aHScrollBar,
	TScrollBar *aVScrollBar);
    ~TLDTViewer() {};
    virtual TPalette &getPalette() const;
    virtual void getText(char *dest, ccIndex item, short maxLen);

private:
    int getLDTDescriptor(int no, void *descr);
    void describeDescriptor(char *buf, void *descr);
};

class TLDTWindow : public TWindow
{
public:
    TLDTWindow(const TRect &bounds);
};

#endif
