/* CodeView/32 - TRegistersWindow Header File */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: register.h,v 1.1 2001/03/22 06:14:03 pete Exp $ */

#if !defined( __REGISTERSWINDOW_H )
#define __REGISTERSWINDOW_H

#define Uses_TWindow
#include <tv.h>

class TRegistersWindow : public TWindow
{
public:
    TRegistersWindow(const TRect &bounds);
    virtual void sizeLimits(TPoint &min, TPoint &max);
};

#endif
