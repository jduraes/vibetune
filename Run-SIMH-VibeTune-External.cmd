@echo off
setlocal

set ROMWBW=..\RomWBW
set SIMH_DIR=%ROMWBW%\Tools\simh
set SIMH_EXE=%SIMH_DIR%\altairz80.exe
set SIMH_CFG_BASE=%SIMH_DIR%\Sim.cfg
set VTUNE_BIN=vtune.com
set ROM_NAME=

if not "%~1"=="" set ROM_NAME=%~1

if "%ROM_NAME%"=="" set ROM_NAME=SBC_simh_std

if not exist %VTUNE_BIN% (
  echo vtune.com not found in current folder.
  echo Run Build.cmd first, then retry.
  exit /b 1
)

if not exist %SIMH_EXE% (
  echo SIMH executable not found: %SIMH_EXE%
  exit /b 1
)

if not exist %SIMH_CFG_BASE% (
  echo SIMH base config not found: %SIMH_CFG_BASE%
  exit /b 1
)

echo %ROM_NAME% | findstr /I "simh" >nul
if errorlevel 1 (
  echo ROM "%ROM_NAME%" is not a SIMH ROM profile.
  echo Sim.cfg expects a *_simh_* ROM. Non-SIMH ROMs can appear to hang.
  echo Example: Run-SIMH-VibeTune-External.cmd SBC_simh_std
  exit /b 1
)

if not exist %ROMWBW%\Binary\%ROM_NAME%.rom (
  echo ROM image not found: %ROMWBW%\Binary\%ROM_NAME%.rom
  echo Build it with:
  echo   pushd ..\RomWBW\Source\HBIOS ^&^& Build SBC simh_std ^&^& popd
  echo Then retry:
  echo   Run-SIMH-VibeTune-External.cmd SBC_simh_std
  exit /b 1
)

copy /Y %VTUNE_BIN% %ROMWBW%\Binary\Apps\vtune.com >nul
if errorlevel 1 (
  echo Failed to copy vtune.com into RomWBW Binary\Apps.
  exit /b 1
)

if exist vtunecfg.com (
  copy /Y vtunecfg.com %ROMWBW%\Binary\Apps\vtunecfg.com >nul
)

powershell -NoProfile -ExecutionPolicy Bypass -File .\Sync-SIMH-Content.ps1
if errorlevel 1 (
  echo Failed to synchronize SIMH disk image content.
  exit /b 1
)

pushd %ROMWBW%\Tools\simh
echo Launching external SIMH console with ROM: %ROM_NAME%.rom
echo Using stock Sim.cfg disk mappings.
echo For ZPM3 testing at boot prompt, enter: 3.4
echo Close that window when done.
start "SIMH VibeTune" cmd /k "cd /d %CD% && altairz80.exe Sim.cfg ..\..\Binary\%ROM_NAME%.rom"
popd

exit /b 0
