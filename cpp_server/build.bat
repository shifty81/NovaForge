@echo off
REM Build script for Nova Forge C++ Dedicated Server (Windows)

setlocal

REM --- Logging Setup ---
set "SCRIPT_DIR=%~dp0"
if not exist "%SCRIPT_DIR%logs" mkdir "%SCRIPT_DIR%logs"
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value') do set "datetime=%%I"
set "LOG_FILE=%SCRIPT_DIR%logs\server_build_%datetime:~0,8%_%datetime:~8,6%.log"
echo Build log will be saved to: %LOG_FILE%
call :main %* > "%LOG_FILE%" 2>&1
set "EXIT_CODE=%ERRORLEVEL%"
echo Build finished. Log saved to: %LOG_FILE%
exit /b %EXIT_CODE%

:main
echo Log file: %LOG_FILE%
echo.

echo ==================================
echo Nova Forge Server Build Script
echo ==================================
echo.

REM Check if CMake is installed
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake is not installed
    echo Please install CMake 3.15 or higher
    exit /b 1
)

REM Parse arguments
set USE_STEAM=ON
set BUILD_TYPE=Release

:parse_args
if "%~1"=="" goto :done_parsing
if "%~1"=="--no-steam" (
    set USE_STEAM=OFF
    shift
    goto :parse_args
)
if "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
echo Unknown option: %~1
echo Usage: %~0 [--no-steam] [--debug]
exit /b 1

:done_parsing

echo Configuration:
echo   Build Type: %BUILD_TYPE%
echo   Steam SDK: %USE_STEAM%
echo.

REM Create build directory
if not exist build mkdir build
cd build

REM Run CMake
echo Running CMake...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DUSE_STEAM_SDK=%USE_STEAM%
if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed
    exit /b 1
)

echo.
echo Building...

REM Build
cmake --build . --config %BUILD_TYPE%
if %ERRORLEVEL% NEQ 0 (
    echo Build failed
    exit /b 1
)

echo.
echo ==================================
echo Build complete!
echo ==================================
echo.
echo Executable: build\bin\%BUILD_TYPE%\novaforge_server.exe
echo Config: build\bin\config\server.json
echo.
echo To run the server:
echo   cd build\bin\%BUILD_TYPE%
echo   novaforge_server.exe
echo.

cd ..
