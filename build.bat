@echo off
rem Build Batch file for CV32
rem  By Peter Johnson, 2001
rem
rem $Id: build.bat,v 1.3 2001/04/23 03:31:07 pete Exp $
make cv32objs
make2 cv32.exe
make cv32tvobjs
make2 cv32tv.exe
make tests
