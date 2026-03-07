@echo off
REM ============================================================
REM  DEPRECATED – use the cross-platform shell script instead:
REM
REM    bash scripts/build_all.sh %*
REM
REM  build_all.sh auto-detects Visual Studio on Windows and
REM  generates the solution as part of the CMake configure step.
REM  This wrapper attempts to locate bash and forward automatically.
REM ============================================================

setlocal

set "SCRIPT_DIR=%~dp0"

REM --- Try Git Bash (typical Windows install) ---
where bash >nul 2>&1
if not errorlevel 1 (
    echo [generate_solution.bat] DEPRECATED: forwarding to build_all.sh ...
    bash "%SCRIPT_DIR%build_all.sh" %*
    exit /b %ERRORLEVEL%
)

REM --- Try common Git-for-Windows paths ---
if exist "C:\Program Files\Git\bin\bash.exe" (
    echo [generate_solution.bat] DEPRECATED: forwarding to build_all.sh ...
    "C:\Program Files\Git\bin\bash.exe" "%SCRIPT_DIR%build_all.sh" %*
    exit /b %ERRORLEVEL%
)
if exist "C:\Program Files (x86)\Git\bin\bash.exe" (
    echo [generate_solution.bat] DEPRECATED: forwarding to build_all.sh ...
    "C:\Program Files (x86)\Git\bin\bash.exe" "%SCRIPT_DIR%build_all.sh" %*
    exit /b %ERRORLEVEL%
)

REM --- Could not find bash ---
echo.
echo ================================================================
echo  generate_solution.bat is DEPRECATED
echo ================================================================
echo.
echo  This script has been replaced by the cross-platform build
echo  script scripts/build_all.sh, which auto-detects Visual Studio
echo  on Windows and generates the solution during the CMake
echo  configure step.
echo.
echo  To build Nova Forge on Windows:
echo.
echo    1. Install Git for Windows (https://gitforwindows.org/)
echo    2. Open Git Bash and run:
echo.
echo         ./scripts/build_all.sh
echo.
echo  The .sln file will be generated at build/Atlas.sln
echo.
exit /b 1
