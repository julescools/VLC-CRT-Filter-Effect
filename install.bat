@echo off
REM ============================================================================
REM  Install CRT Scanline plugin + Lua controller to VLC
REM  Right-click > Run as administrator
REM ============================================================================

set PLUGIN_NAME=libcrt_scanline_plugin.dll
set LUA_NAME=crt_scanline_controller.lua
set BUILD_DIR=%~dp0build
set LUA_DIR=%~dp0lua
set VLC_DIR=C:\Program Files\VideoLAN\VLC

if not exist "%BUILD_DIR%\%PLUGIN_NAME%" (
    echo ERROR: Plugin not built yet. Run build.bat first.
    pause
    exit /b 1
)

echo Closing VLC if running...
taskkill /f /im vlc.exe >nul 2>&1
timeout /t 2 >nul

echo.
echo Installing video filter plugin...
copy /y "%BUILD_DIR%\%PLUGIN_NAME%" "%VLC_DIR%\plugins\video_filter\" >nul
if errorlevel 1 (
    echo ERROR: Copy failed. Run this script as administrator.
    pause
    exit /b 1
)

echo Installing Lua controller extension...
if not exist "%VLC_DIR%\lua\extensions" mkdir "%VLC_DIR%\lua\extensions"
copy /y "%LUA_DIR%\%LUA_NAME%" "%VLC_DIR%\lua\extensions\" >nul
if errorlevel 1 (
    echo WARNING: Could not copy Lua extension.
)

echo Deleting plugin cache...
if exist "%VLC_DIR%\plugins\plugins.dat" del "%VLC_DIR%\plugins\plugins.dat"

echo.
echo ============================================
echo  INSTALLED SUCCESSFULLY
echo ============================================
echo.
echo Setup (one-time):
echo   1. Start VLC
echo   2. Disable hardware decoding:
echo      Tools ^> Preferences ^> Input/Codecs ^> Hardware-accelerated decoding ^> Disable
echo   3. Enable the filter:
echo      Tools ^> Preferences ^> Show settings: All ^> Video ^> Filters
echo      Check "CRT Scanline video filter" ^> Save
echo   4. Restart VLC
echo.
echo Live control:
echo   View ^> CRT Scanline Controller
echo   (adjust darkness, spacing, blend mode while video plays)
echo.
pause
