@echo off
rem Build Batch file for CV32
rem  By Peter Johnson, 2001
rem
rem $Id: build.bat,v 1.1 2001/04/22 22:20:32 pete Exp $
make cv32objs
make cv32tvobjs
make2 all
