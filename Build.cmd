@echo off
setlocal

set TOOLS=..\RomWBW\Tools
set PATH=%TOOLS%\tasm32;%PATH%
set TASMTABS=%TOOLS%\tasm32

tasm -t80 -g3 -fFF -dWBW vibetune.asm vtune.com vtune.lst || exit /b
copy /Y vtune.com ..\RomWBW\Binary\Apps\ || exit /b
