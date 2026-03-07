@echo off
REM ============================================================
REM  DEPRECATED – use the cross-platform shell script instead:
REM
REM    bash scripts/build_all.sh %*
REM
REM  build_all.sh builds all targets including the dedicated
REM  server.  For a server-only build you can also use:
REM
REM    bash cpp_server/build.sh [--debug] [--no-steam] [--clean]
REM
REM  This wrapper attempts to locate bash and forward automatically.
REM ============================================================

setlocal

set "SCRIPT_DIR=%~dp0"

REM --- Try Git Bash (typical Windows install) ---
where bash >nul 2>&1
if not errorlevel 1 (
    echo [cpp_server/build.bat] DEPRECATED: forwarding to cpp_server/build.sh ...
    bash "%SCRIPT_DIR%build.sh" %*
    exit /b %ERRORLEVEL%
)

REM --- Try common Git-for-Windows paths ---
if exist "C:\Program Files\Git\bin\bash.exe" (
    echo [cpp_server/build.bat] DEPRECATED: forwarding to cpp_server/build.sh ...
    "C:\Program Files\Git\bin\bash.exe" "%SCRIPT_DIR%build.sh" %*
    exit /b %ERRORLEVEL%
)
if exist "C:\Program Files (x86)\Git\bin\bash.exe" (
    echo [cpp_server/build.bat] DEPRECATED: forwarding to cpp_server/build.sh ...
    "C:\Program Files (x86)\Git\bin\bash.exe" "%SCRIPT_DIR%build.sh" %*
    exit /b %ERRORLEVEL%
)

REM --- Could not find bash ---
echo.
echo ================================================================
echo  cpp_server/build.bat is DEPRECATED
echo ================================================================
echo.
echo  This script has been replaced by the cross-platform build
echo  scripts.  To build the server on Windows:
echo.
echo    1. Install Git for Windows (https://gitforwindows.org/)
echo    2. Open Git Bash and run:
echo.
echo         ./scripts/build_all.sh          # build everything
echo         ./cpp_server/build.sh           # server only
echo.
exit /b 1
