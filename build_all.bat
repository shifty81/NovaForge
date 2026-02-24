@echo off
REM Nova Forge - Unified Windows Build Script
REM Builds engine, editor, client, and server into a single output directory.
REM The resulting build\bin\ folder can be zipped and shared for testing.

setlocal

REM --- Logging Setup ---
set "SCRIPT_DIR=%~dp0"
if not exist "%SCRIPT_DIR%logs" mkdir "%SCRIPT_DIR%logs"
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value') do set "datetime=%%I"
set "LOG_FILE=%SCRIPT_DIR%logs\build_all_%datetime:~0,8%_%datetime:~8,6%.log"
echo Build log will be saved to: %LOG_FILE%
call :main %* 2>&1 | powershell -Command "$input | Tee-Object -FilePath '%LOG_FILE%'"
set "EXIT_CODE=%ERRORLEVEL%"
echo Build finished. Log saved to: %LOG_FILE%
exit /b %EXIT_CODE%

:main
echo Log file: %LOG_FILE%
echo.

echo ================================================
echo Nova Forge - Unified Build (All Targets)
echo ================================================
echo.

REM --- Dependency check ---
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)
echo   CMake found

REM --- Parse arguments ---
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=0"

:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="--debug" (set "BUILD_TYPE=Debug" & shift & goto :parse_args)
if /i "%~1"=="Debug"   (set "BUILD_TYPE=Debug" & shift & goto :parse_args)
if /i "%~1"=="--clean" (set "CLEAN_BUILD=1"    & shift & goto :parse_args)
if /i "%~1"=="--help"  (goto :show_help)
if /i "%~1"=="-h"      (goto :show_help)
shift
goto :parse_args

:show_help
echo Usage: %~0 [Release^|Debug] [--debug] [--clean]
echo.
echo   Release    Build optimised binaries (default)
echo   --debug    Build with debug symbols
echo   --clean    Remove build directory before building
exit /b 0

:done_parsing

REM --- Clean if requested ---
if "%CLEAN_BUILD%"=="1" (
    echo Cleaning build directory...
    if exist build rmdir /S /Q build
    echo.
)

REM --- Configure ---
if not exist build mkdir build
cd build

echo Configuration:
echo   Build Type:   %BUILD_TYPE%
echo   Build Dir:    %SCRIPT_DIR%build
echo   Output Dir:   %SCRIPT_DIR%build\bin
echo.

echo Configuring CMake (%BUILD_TYPE%) - all targets...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DUSE_SYSTEM_LIBS=ON -DBUILD_ATLAS_ENGINE=ON -DBUILD_ATLAS_EDITOR=ON -DBUILD_CLIENT=ON -DBUILD_SERVER=ON -DBUILD_ATLAS_TESTS=ON

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

REM --- Build ---
echo.
echo Building all targets...
cmake --build . --config %BUILD_TYPE%

if errorlevel 1 (
    echo.
    echo ================================================
    echo BUILD FAILED
    echo ================================================
    echo.
    echo Log: %LOG_FILE%
    exit /b 1
)

echo.
echo ================================================
echo BUILD SUCCESSFUL
echo ================================================
echo.
echo Output directory: %SCRIPT_DIR%build\bin\
echo.

REM --- Copy data alongside binaries for a portable package ---
if not exist "bin\data" (
    if exist "%SCRIPT_DIR%data" (
        echo Copying game data into output directory...
        xcopy /E /I /Q "%SCRIPT_DIR%data" "bin\data" >nul
        echo   data\ copied
        echo.
    )
)

echo To share for testing:
echo   cd %SCRIPT_DIR%
echo   powershell Compress-Archive -Path build\bin\* -DestinationPath novaforge_build.zip
echo.
echo Log: %LOG_FILE%
exit /b 0
