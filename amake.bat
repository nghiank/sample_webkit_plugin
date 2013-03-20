@echo off
@rem (C) Copyright 2001, 2005 by Autodesk, Inc.  All Rights Reserved.


if "%OS%" == "Windows_NT" setlocal


  :: Setup custom environment
  set APP=npapi_layout_test_plugin
  set EXT=dll


  :: Set the build configuration string
  set CFG=Current


  :: If we're doing a dependent build pass, skip over the code module build  
  if "%WWB%" == "dep" goto Dep_Build


  :: Build the project
  :: Filter out ingorable warnings (VS2010 doesn't honor pragma directives 
  :: and /ignore command line to ignore these)
  ::  "warning LNK4099: PDB 'vc100.pdb' was not found with ... linking object as if no debug info"
  ::  "warning LNK4049: locally defined symbol ..."
  ::  "warning LNK4217: locally defined symbol ..."
  call %TBIN%\build.bat %1 %2 %3 %4 %5 %6 %7 %8 %9 | findstr /v "LNK4099 LNK4049 LNK4217"

:Dep_Build


:Done
  :: Cleanup env
  set CFG=
  set EXT=
  set APP=
  if "%OS%" == "Windows_NT" endlocal

