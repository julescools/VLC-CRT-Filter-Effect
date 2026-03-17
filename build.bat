@echo off
REM ============================================================================
REM  Build script for CRT Scanline VLC plugin
REM  Run this from a "x64 Native Tools Command Prompt for VS 2026"
REM  (or "Developer Command Prompt" — must be 64-bit)
REM ============================================================================

setlocal

REM --- Configuration ---
set VLC_SDK=C:\vlc-sdk
set SRC_DIR=%~dp0src
set OUT_DIR=%~dp0build
set PLUGIN_NAME=libcrt_scanline_plugin

REM --- Verify SDK exists ---
if not exist "%VLC_SDK%\include\vlc\plugins\vlc_common.h" (
    echo ERROR: VLC SDK not found at %VLC_SDK%
    echo Make sure you extracted the VLC SDK to C:\vlc-sdk
    exit /b 1
)

REM --- Verify compiler is available ---
where cl >nul 2>&1
if errorlevel 1 (
    echo ERROR: cl.exe not found.
    echo Run this script from "x64 Native Tools Command Prompt for VS 2026"
    echo   Start Menu ^> Visual Studio 2026 ^> x64 Native Tools Command Prompt
    exit /b 1
)

REM --- Create output directory ---
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

echo.
echo ============================================
echo  Building CRT Scanline Plugin for VLC 3.0.x
echo ============================================
echo.

REM --- Compile and link as DLL ---
cl.exe /nologo /O2 /W3 /LD ^
    /std:c11 ^
    /D__PLUGIN__ ^
    /DMODULE_STRING=\"crtscanline\" ^
    /D_CRT_SECURE_NO_WARNINGS ^
    /D_CRT_NONSTDC_NO_DEPRECATE ^
    /FI"%SRC_DIR%\msvc_compat.h" ^
    /I"%VLC_SDK%\include\vlc\plugins" ^
    /I"%SRC_DIR%" ^
    "%SRC_DIR%\crt_scanline.c" ^
    /Fe"%OUT_DIR%\%PLUGIN_NAME%.dll" ^
    /link ^
    /LIBPATH:"%VLC_SDK%\lib" ^
    libvlccore.lib

if errorlevel 1 (
    echo.
    echo BUILD FAILED
    exit /b 1
)

REM --- Clean up intermediate files ---
if exist "%OUT_DIR%\%PLUGIN_NAME%.exp" del "%OUT_DIR%\%PLUGIN_NAME%.exp"
if exist "%OUT_DIR%\%PLUGIN_NAME%.lib" del "%OUT_DIR%\%PLUGIN_NAME%.lib"
if exist "%OUT_DIR%\crt_scanline.obj"  del "%OUT_DIR%\crt_scanline.obj"

echo.
echo ============================================
echo  BUILD SUCCEEDED
echo ============================================
echo.
echo Output: %OUT_DIR%\%PLUGIN_NAME%.dll
echo.
echo To install:
echo   1. Close VLC completely
echo   2. Copy the DLL to your VLC plugins folder:
echo      copy "%OUT_DIR%\%PLUGIN_NAME%.dll" "C:\Program Files\VideoLAN\VLC\plugins\video_filter\"
echo   3. Delete the plugin cache so VLC rescans:
echo      del "C:\Program Files\VideoLAN\VLC\plugins\plugins.dat"
echo   4. Start VLC
echo   5. Go to: Tools ^> Preferences ^> Show settings: All
echo      Then: Video ^> Filters ^> check "CRT Scanline video filter"
echo      Click Save, restart VLC
echo.

endlocal
