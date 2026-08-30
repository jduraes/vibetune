@echo off
setlocal

set TOOLS=..\RomWBW\Tools
set PATH=%TOOLS%\tasm32;%PATH%
set TASMTABS=%TOOLS%\tasm32

rem Default build omits MYM (size). Pass MYM as first arg to include mymeng.inc:
rem   Build.cmd MYM
set MYMFLAG=
if /I "%~1"=="MYM" set MYMFLAG=-dMYM

tasm -t80 -g3 -fFF -dWBW %MYMFLAG% vibetune.asm vtune.com vtune.lst || exit /b
copy /Y vtune.com ..\RomWBW\Binary\Apps\ || exit /b
tasm -t80 -g3 -fFF -dWBW vtunecfg.asm vtunecfg.com vtunecfg.lst || exit /b
copy /Y vtunecfg.com ..\RomWBW\Binary\Apps\ || exit /b
