/* CodeView/32 - Hint Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: cvhint.cpp,v 1.1 2001/02/02 17:02:43 pete Exp $ */

#include <tv.h>
 
#include "cvhint.h"
#include "cvconst.h"

#define hint(ctx, str)	    case hc##ctx: return str;

const char *CVHint(ushort ctx)
{
    switch(ctx) {
hint(Quit, "Exit from CodeView/32")
hint(Zoom, "Zoom or unzoom the current window")
hint(Next, "Go to next window")
hint(Resize, "Move or change size of window")
hint(Close, "Remove current window")
hint(RepaintDesktopCmd, "Redisplay entire screen")
hint(AboutCmd, "Get information about CodeView/32")
hint(OpenCmd, "Load a new program to debug")
hint(ChDirCmd, "Change to a new current directory")
hint(GetInfoCmd, "Display information about program being debugged")
hint(BreakpointsCmd, "Open a breakpoints window")
hint(StackCmd, "Open a stack window")
hint(LogCmd, "Open a log window")
hint(WatchesCmd, "Open a watches window")
hint(VariablesCmd, "Open a variables window")
hint(ModuleCmd, "Open a module window")
hint(FileCmd, "Open a file window")
hint(LDTCmd, "Open a LDT window")
hint(DumpCmd, "Open a dump window")
hint(RegistersCmd, "Open a CPU registers window")
hint(NumericProcessorCmd, "Open a numeric processor (FP/MMX/SSE) window")
hint(AnotherModuleCmd, "Open another module window")
hint(AnotherDumpCmd, "Open another dump window")
hint(AnotherFileCmd, "Open another file window")
hint(RunCmd, "Run program at full speed")
hint(GoToCursorCmd, "Run to current location")
hint(TraceIntoCmd, "Execute a single instruction")
hint(StepOverCmd, "Execute an instruction, skipping calls")
hint(ExecuteToCmd, "Run to a specified location")
hint(UntilReturnCmd, "Run until current routine returns to its caller")
hint(ArgumentsCmd, "Set command line arguments for debugged program")
hint(ProgramResetCmd, "Reload program from disk")
hint(BreakpointToggleCmd, "Toggle breakpoint at current location")
hint(BreakpointAtCmd, "Set breakpoint at specified address")
hint(BreakpointDeleteAllCmd, "Remove all breakpoints") 
hint(InspectCmd, "Open inspector on specified variable or expression")
hint(EvaluateModifyCmd, "Evaluate and display value of expression")
hint(AddWatchCmd, "Add a new expression to watches window")
hint(DisplayOptionsCmd, "Set the appearance of the screen display")
hint(PathForSourceCmd, "Specify directories to search for source files")
hint(UserScreenCmd, "Look at the screen of the program being debugged")
    default:
	return "";
    }
}

