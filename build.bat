@echo off
rem Build Batch file for CV32
rem  By Peter Johnson, 2001
rem
rem $Id: build.bat,v 1.2 2001/04/22 22:31:50 pete Exp $
make cv32objs
make2 cv32.exe
make cv32tvobjs
make2 cv32tv.exe
