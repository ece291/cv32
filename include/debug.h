/* Copyright (C) 1993 DJ Delorie, see COPYING.DJ for details */
/* $Id: debug.h,v 1.2 2001/01/31 03:24:06 pete Exp $ */
extern int can_longjmp;
extern jmp_buf debugger_jmpbuf;
void debugger(void);
