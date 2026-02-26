@echo off
REM Nova Forge - Windows Build Script
REM Builds the C++ client and server using CMake

setlocal

REM --- Resolve project root (parent of scripts/) ---
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
pushd "%PROJECT_ROOT%"
set "PROJECT_ROOT=%CD%"
popd

REM --- Logging Setup ---
if not exist "%PROJECT_ROOT%\logs" mkdir "%PROJECT_ROOT%\logs"
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value') do set "datetime=%%I"
set "LOG_FILE=%PROJECT_ROOT%\logs\build_%datetime:~0,8%_%datetime:~8,6%.log"
echo Build log will be saved to: %LOG_FILE%
call :main %* > "%LOG_FILE%" 2>&1
set "EXIT_CODE=%ERRORLEVEL%"
echo Build finished. Log saved to: %LOG_FILE%
pause
exit /b %EXIT_CODE%

:main
echo Log file: %LOG_FILE%
echo.

echo ================================================
echo Nova Forge - Build
echo ================================================
echo.

REM Check for CMake
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

set "BUILD_TYPE=Release"
if /i "%~1"=="--debug" (set "BUILD_TYPE=Debug")

REM Create build directory
cd /d "%PROJECT_ROOT%"
if not exist build mkdir build
cd build

REM Configure
echo Configuring CMake (%BUILD_TYPE%)...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DUSE_SYSTEM_LIBS=ON -DBUILD_ATLAS_EDITOR=ON

if errorlevel 1 (
    echo.
    echo ================================================
    echo CMAKE CONFIGURATION FAILED
    echo ================================================
    echo.
    echo Install dependencies via vcpkg:
    echo   vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE%

if errorlevel 1 (
    echo.
    echo ================================================
    echo BUILD FAILED
    echo ================================================
    exit /b 1
) else (
    echo.
    echo ================================================
    echo BUILD SUCCESSFUL
    echo ================================================
    exit /b 0
)
