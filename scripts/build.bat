@echo off
REM ============================================================
REM  DEPRECATED – use the cross-platform shell script instead:
REM
REM    bash scripts/build_all.sh %*
REM
REM  build_all.sh works on Windows (Git Bash / MSYS2), Linux,
REM  and macOS.  This wrapper attempts to locate bash and
REM  forward automatically; if it cannot, it prints instructions.
REM ============================================================

setlocal

set "SCRIPT_DIR=%~dp0"

REM --- Try Git Bash (typical Windows install) ---
where bash >nul 2>&1
if not errorlevel 1 (
    echo [build.bat] DEPRECATED: forwarding to build_all.sh ...
    bash "%SCRIPT_DIR%build_all.sh" %*
    exit /b %ERRORLEVEL%
)

REM --- Try common Git-for-Windows paths ---
if exist "C:\Program Files\Git\bin\bash.exe" (
    echo [build.bat] DEPRECATED: forwarding to build_all.sh ...
    "C:\Program Files\Git\bin\bash.exe" "%SCRIPT_DIR%build_all.sh" %*
    exit /b %ERRORLEVEL%
)
if exist "C:\Program Files (x86)\Git\bin\bash.exe" (
    echo [build.bat] DEPRECATED: forwarding to build_all.sh ...
    "C:\Program Files (x86)\Git\bin\bash.exe" "%SCRIPT_DIR%build_all.sh" %*
    exit /b %ERRORLEVEL%
)

REM --- Could not find bash ---
echo.
echo ================================================================
echo  build.bat is DEPRECATED
echo ================================================================
echo.
echo  This script has been replaced by the cross-platform build
echo  script scripts/build_all.sh, which works on Windows (via
echo  Git Bash or MSYS2), Linux, and macOS.
echo.
echo  To build Nova Forge on Windows:
echo.
echo    1. Install Git for Windows (https://gitforwindows.org/)
echo    2. Open Git Bash and run:
echo.
echo         ./scripts/build_all.sh
echo.
exit /b 1
