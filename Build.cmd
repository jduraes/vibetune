@echo off
setlocal

rem Dual-context: RomWBW Source/Apps/VibeTune (junction) vs sibling ../VibeTune
set INROMWBW=
if exist "..\..\..\Tools\tasm32" (
  set TOOLS=..\..\..\Tools
  set DEST=..\..\..\Binary\Apps
  set INROMWBW=-dVT_IN_ROMWBW
) else (
  set TOOLS=..\RomWBW\Tools
  set DEST=..\RomWBW\Binary\Apps
)

set PATH=%TOOLS%\tasm32;%PATH%
set TASMTABS=%TOOLS%\tasm32

rem Default build omits MYM (size). Pass MYM as first arg to include mymeng.inc:
rem   Build.cmd MYM
set MYMFLAG=
if /I "%~1"=="MYM" set MYMFLAG=-dMYM

tasm -t80 -g3 -fFF -dWBW %INROMWBW% %MYMFLAG% vibetune.asm vtune.com vtune.lst || exit /b
copy /Y vtune.com %DEST%\ || exit /b
tasm -t80 -g3 -fFF -dWBW %INROMWBW% vtunecfg.asm vtunecfg.com vtunecfg.lst || exit /b
copy /Y vtunecfg.com %DEST%\ || exit /b
