/* CodeView/32 - Menu Implementation */
/* Copyright (c) 2001 by Peter Johnson, pete@bilogic.org */
/* $Id: cvmenu.cpp,v 1.2 2001/03/22 19:51:59 pete Exp $ */
 
#define Uses_TRect
#define Uses_TMenuBar
#define Uses_TSubMenu
#define Uses_TMenuItem
#define Uses_TKeys
#define Uses_TApplication

#include <tv.h>

#include "debugapp.h"
#include "cvconst.h"

TMenuBar *TDebugApp::initMenuBar(TRect r)
{
    TSubMenu &SystemMenu = *new TSubMenu("~\360~", 0) +
	*new TMenuItem("~R~epaint desktop", cmRepaintDesktopCmd, kbNoKey, hcRepaintDesktopCmd) +
	newLine() +
	*new TMenuItem("~A~bout...", cmAboutCmd, kbNoKey, hcAboutCmd);

    TSubMenu &FileMenu = *new TSubMenu("~F~ile", kbAltF) +
	*new TMenuItem("~O~pen...", cmOpenCmd, kbNoKey, hcOpenCmd) +
	*new TMenuItem( "~C~hange Dir...", cmChDirCmd, kbNoKey, hcChDirCmd) +
	*new TMenuItem("~G~et info...", cmGetInfoCmd, kbNoKey, hcGetInfoCmd) +
	newLine() +
	*new TMenuItem("~Q~uit", cmQuit, kbAltX, hcQuit, "Alt-X");

    TMenuItem &AnotherSub = *new TSubMenu("~A~nother", kbAltA) +
	*new TMenuItem("~M~odule...", cmAnotherModuleCmd, kbNoKey, hcAnotherModuleCmd) +
	*new TMenuItem("~D~ump", cmAnotherDumpCmd, kbNoKey, hcAnotherDumpCmd) +
	*new TMenuItem("~F~ile...", cmAnotherFileCmd, kbNoKey, hcAnotherFileCmd);

    TSubMenu &ViewMenu = *new TSubMenu("~V~iew", kbAltV) +
	*new TMenuItem("~B~reakpoints", cmBreakpointsCmd, kbNoKey, hcBreakpointsCmd) +
	*new TMenuItem("~S~tack", cmStackCmd, kbNoKey, hcStackCmd) +
	*new TMenuItem("L~o~g", cmLogCmd, kbNoKey, hcLogCmd) +
	*new TMenuItem("~W~atches", cmWatchesCmd, kbNoKey, hcWatchesCmd) +
	*new TMenuItem("~V~ariables", cmVariablesCmd, kbNoKey, hcVariablesCmd) +
	*new TMenuItem("~M~odule...", cmModuleCmd, kbF3, hcModuleCmd, "F3") +
	*new TMenuItem("~F~ile...", cmFileCmd, kbNoKey, hcFileCmd) +
	*new TMenuItem("~D~isassembly", cmDisassemblyCmd, kbNoKey, hcDisassemblyCmd) +
	*new TMenuItem("~L~DT", cmLDTCmd, kbNoKey, hcLDTCmd) +
	*new TMenuItem("~D~ump", cmDumpCmd, kbNoKey, hcDumpCmd) +
	*new TMenuItem("~R~egisters", cmRegistersCmd, kbNoKey, hcRegistersCmd) +
	*new TMenuItem("~N~umeric processor", cmNumericProcessorCmd, kbNoKey, hcNumericProcessorCmd) +
	AnotherSub;

    TSubMenu &RunMenu = *new TSubMenu("~R~un", kbAltR) +
	*new TMenuItem("~R~un", cmRunCmd, kbF9, hcRunCmd, "F9") +
	*new TMenuItem("~G~o to cursor", cmGoToCursorCmd, kbF4, hcGoToCursorCmd, "F4") +
	*new TMenuItem("~T~race into", cmTraceIntoCmd, kbF7, hcTraceIntoCmd, "F7") +
	*new TMenuItem("~S~tep over", cmStepOverCmd, kbF8, hcStepOverCmd, "F8") +
	*new TMenuItem("~E~xecute to...", cmExecuteToCmd, kbAltF9, hcExecuteToCmd, "Alt-F9") +
	*new TMenuItem("~U~ntil return", cmUntilReturnCmd, kbAltF8, hcUntilReturnCmd, "Alt-F8") +
	newLine() +
	*new TMenuItem("~A~rguments...", cmArgumentsCmd, kbNoKey, hcArgumentsCmd) +
	*new TMenuItem("~P~rogram reset", cmProgramResetCmd, kbCtrlF2, hcProgramResetCmd, "Ctrl-F2");

    TSubMenu &BreakpointsMenu = *new TSubMenu("~B~reakpoints", kbAltB) +
	*new TMenuItem("~T~oggle", cmBreakpointToggleCmd, kbF2, hcBreakpointToggleCmd, "F2") +
	*new TMenuItem("~A~t...", cmBreakpointAtCmd, kbAltF2, hcBreakpointAtCmd, "Alt-F2") +
	*new TMenuItem("~D~elete all", cmBreakpointDeleteAllCmd, kbNoKey, hcBreakpointDeleteAllCmd);

    TSubMenu &DataMenu = *new TSubMenu("~D~ata", kbAltD) +
	*new TMenuItem("~I~nspect...", cmInspectCmd, kbNoKey, hcInspectCmd) +
	*new TMenuItem("~E~valuate/modify...", cmEvaluateModifyCmd, kbCtrlF4, hcEvaluateModifyCmd, "Ctrl-F4") +
	*new TMenuItem("Add ~w~atch...", cmAddWatchCmd, kbCtrlF7, hcAddWatchCmd, "Ctrl-F7");

    TSubMenu &OptionsMenu = *new TSubMenu("~O~ptions", kbAltO) +
	*new TMenuItem("~D~isplay options...", cmDisplayOptionsCmd, kbNoKey, hcDisplayOptionsCmd) +
	*new TMenuItem("~P~ath for source...", cmPathForSourceCmd, kbNoKey, hcPathForSourceCmd);

    TSubMenu &WindowMenu = *new TSubMenu("~W~indow", kbAltW) +
	*new TMenuItem("~Z~oom", cmZoom, kbF5, hcZoom, "F5") +
	*new TMenuItem("~N~ext", cmNext, kbF6, hcNext, "F6") +
	*new TMenuItem("~S~ize/move", cmResize, kbCtrlF5, hcResize, "Ctrl-F5") +
	*new TMenuItem("~C~lose", cmClose, kbAltF3, hcClose, "Alt-F3") +
	newLine() +
	*new TMenuItem("U~s~er screen", cmUserScreenCmd, kbAltF5, hcUserScreenCmd, "Alt-F5");

    r.b.y = r.a.y + 1;
    return (new TMenuBar(r, SystemMenu + FileMenu + ViewMenu + RunMenu +
	BreakpointsMenu + DataMenu + OptionsMenu + WindowMenu));
}

