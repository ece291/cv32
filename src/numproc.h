/* CodeView/32 - TNumericProcessorWindow Header File */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: numproc.h,v 1.2 2001/03/22 03:29:57 pete Exp $ */

#if !defined( __NUMERICPROCESSORVIEWER_H )
#define __NUMERICPROCESSORVIEWER_H

#define Uses_TWindow
#include <tv.h>

class TNumericProcessorWindow : public TWindow
{
public:
    TNumericProcessorWindow(const TRect &bounds);
    virtual void sizeLimits(TPoint &min, TPoint &max);
};

#endif
