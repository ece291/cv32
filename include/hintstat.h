/* CodeView/32 - THintStatusLine Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: hintstat.h,v 1.1 2001/02/02 17:02:43 pete Exp $ */
#if defined( Uses_THintStatusLine ) && !defined( __THintStatusLine )
#define __THintStatusLine

class TRect;
class TStatusDef;

class THintStatusLine : public TStatusLine
{
public:
    const char * (*HintFunction)(ushort);
    THintStatusLine(const TRect &r,TStatusDef &def,
	const char * (*hintfunction)(ushort) = NULL) :
	TStatusLine(r,def),
	HintFunction(hintfunction) {}
    virtual const char *hint(ushort);
};

inline const char *THintStatusLine::hint(ushort ctx)
{
    if (!HintFunction)
	return "";
    return HintFunction(ctx);
}

#endif

