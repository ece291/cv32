/* CodeView/32 - TNumericProcessorWindow Header File */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: numproc.h,v 1.3 2001/03/22 03:33:46 pete Exp $ */

#if !defined( __NUMERICPROCESSORWINDOW_H )
#define __NUMERICPROCESSORWINDOW_H

#define Uses_TWindow
#include <tv.h>

class TNumericProcessorWindow : public TWindow
{
public:
    TNumericProcessorWindow(const TRect &bounds);
    virtual void sizeLimits(TPoint &min, TPoint &max);
};

#endif
