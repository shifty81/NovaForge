@echo off
REM Nova Forge - Windows Build Script (All Targets)
REM Builds engine, editor, client, server, and tests using Visual Studio.
REM The resulting build\bin\ folder can be zipped and shared for testing.
REM
REM This is the primary build entry point for Windows development.
REM It detects your Visual Studio installation and vcpkg dependencies
REM automatically, then builds every target so you can run the editor,
REM play-test the client, or launch the dedicated server.
REM
REM Usage:
REM   build_all.bat                   Build all targets (Release)
REM   build_all.bat --debug           Build with debug symbols
REM   build_all.bat --clean           Clean before building
REM   build_all.bat --test            Run tests after build
REM   build_all.bat --open            Open Visual Studio solution after build
REM   build_all.bat --clean --debug --test   Combine flags

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
set "LOG_FILE=%PROJECT_ROOT%\logs\build_all_%datetime:~0,8%_%datetime:~8,6%.log"
echo Build log will be saved to: %LOG_FILE%
call :main %* 2>&1 | powershell -Command "$input | Tee-Object -FilePath '%LOG_FILE%'"
set "EXIT_CODE=%ERRORLEVEL%"
echo Build finished. Log saved to: %LOG_FILE%
exit /b %EXIT_CODE%

:main
echo Log file: %LOG_FILE%
echo.

echo ================================================
echo Nova Forge - Windows Build (All Targets)
echo ================================================
echo.

REM --- Parse arguments ---
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=0"
set "RUN_TESTS=0"
set "OPEN_VS=0"

:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="--debug"  (set "BUILD_TYPE=Debug" & shift & goto :parse_args)
if /i "%~1"=="Debug"    (set "BUILD_TYPE=Debug" & shift & goto :parse_args)
if /i "%~1"=="--clean"  (set "CLEAN_BUILD=1"    & shift & goto :parse_args)
if /i "%~1"=="--test"   (set "RUN_TESTS=1"      & shift & goto :parse_args)
if /i "%~1"=="--open"   (set "OPEN_VS=1"        & shift & goto :parse_args)
if /i "%~1"=="--help"   (goto :show_help)
if /i "%~1"=="-h"       (goto :show_help)
shift
goto :parse_args

:show_help
echo Usage: %~0 [Release^|Debug] [--debug] [--clean] [--test] [--open]
echo.
echo   Release    Build optimised binaries (default)
echo   --debug    Build with debug symbols
echo   --clean    Remove build directory before building
echo   --test     Run engine and server tests after a successful build
echo   --open     Open the Visual Studio solution after build
echo.
echo Prerequisites:
echo   - Visual Studio 2019 or 2022 with "Desktop development with C++" workload
echo   - CMake (bundled with VS or standalone from https://cmake.org/download/)
echo   - vcpkg with: glfw3 glm glew nlohmann-json (x64-windows)
exit /b 0

:done_parsing

REM --- Dependency check: CMake ---
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found!
    echo   Install CMake from https://cmake.org/download/
    echo   Or use the one bundled with Visual Studio (enable it in the VS Installer).
    exit /b 1
)
for /f "tokens=3" %%V in ('cmake --version 2^>^&1 ^| findstr /i "version"') do set "CMAKE_VER=%%V"
echo   CMake %CMAKE_VER% found

REM --- Detect Visual Studio generator ---
set "VS_GENERATOR="

REM Check VS 2022 first (all editions)
for %%E in (Community Professional Enterprise BuildTools) do (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvarsall.bat" (
        set "VS_GENERATOR=Visual Studio 17 2022"
    )
)

REM Fall back to VS 2019
if "%VS_GENERATOR%"=="" (
    for %%E in (Community Professional Enterprise BuildTools) do (
        if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\%%E\VC\Auxiliary\Build\vcvarsall.bat" (
            set "VS_GENERATOR=Visual Studio 16 2019"
        )
    )
)

if "%VS_GENERATOR%"=="" (
    echo ERROR: Visual Studio 2019 or 2022 not found!
    echo   Install Visual Studio with the "Desktop development with C++" workload.
    echo   https://visualstudio.microsoft.com/downloads/
    exit /b 1
)
echo   Visual Studio: %VS_GENERATOR%

REM --- Detect vcpkg toolchain ---
set "VCPKG_TOOLCHAIN="
set "VCPKG_FOUND=0"

if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    set "VCPKG_FOUND=1"
    echo   vcpkg: C:\vcpkg
) else if exist "%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%USERPROFILE%/vcpkg/scripts/buildsystems/vcpkg.cmake"
    set "VCPKG_FOUND=1"
    echo   vcpkg: %USERPROFILE%\vcpkg
) else if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"
        set "VCPKG_FOUND=1"
        echo   vcpkg: %VCPKG_ROOT%
    )
)

if "%VCPKG_FOUND%"=="0" (
    echo   vcpkg: NOT FOUND (dependencies must be installed manually)
    echo.
    echo   Recommended: install vcpkg and the required packages:
    echo     cd C:\
    echo     git clone https://github.com/microsoft/vcpkg.git
    echo     cd vcpkg ^&^& bootstrap-vcpkg.bat
    echo     vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows
    echo.
)

echo.

REM --- Clean if requested ---
if "%CLEAN_BUILD%"=="1" (
    echo Cleaning build directory...
    if exist build rmdir /S /Q build
    echo.
)

REM --- Configure ---
cd /d "%PROJECT_ROOT%"
if not exist build mkdir build
cd build

REM Always clear CMake cache to avoid generator conflicts
if exist CMakeCache.txt (
    echo Clearing CMake cache to avoid generator conflicts...
    del /f CMakeCache.txt
    if exist CMakeFiles rmdir /S /Q CMakeFiles
    echo.
)

echo Configuration:
echo   Generator:    %VS_GENERATOR%
echo   Architecture: x64
echo   Build Type:   %BUILD_TYPE%
echo   Build Dir:    %PROJECT_ROOT%\build
echo   Output Dir:   %PROJECT_ROOT%\build\bin
echo   Run Tests:    %RUN_TESTS%
echo.

echo Configuring CMake (%BUILD_TYPE%) — all targets...
cmake .. ^
    -G "%VS_GENERATOR%" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DUSE_SYSTEM_LIBS=ON ^
    -DBUILD_ATLAS_ENGINE=ON ^
    -DBUILD_ATLAS_EDITOR=ON ^
    -DBUILD_CLIENT=ON ^
    -DBUILD_SERVER=ON ^
    -DBUILD_ATLAS_TESTS=ON ^
    -DBUILD_TESTS=ON ^
    %VCPKG_TOOLCHAIN%

if errorlevel 1 (
    echo.
    echo ================================================
    echo CMAKE CONFIGURATION FAILED
    echo ================================================
    echo.
    echo Most likely cause: missing dependencies.
    echo.
    echo Install ALL required packages via vcpkg:
    echo   vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows
    echo.
    echo Optional audio support:
    echo   vcpkg install openal-soft:x64-windows
    echo.
    echo For detailed setup instructions see: docs\guides\VS2022_SETUP_GUIDE.md
    exit /b 1
)

REM --- Build ---
echo.
echo Building all targets (Engine, Editor, Client, Server, Tests)...
cmake --build . --config %BUILD_TYPE% -- /m

if errorlevel 1 (
    echo.
    echo ================================================
    echo BUILD FAILED
    echo ================================================
    echo.
    echo Check the log for details: %LOG_FILE%
    exit /b 1
)

echo.
echo ================================================
echo BUILD SUCCESSFUL
echo ================================================
echo.

REM --- List built executables ---
echo Built executables:
if exist "bin\%BUILD_TYPE%" (
    for %%F in (bin\%BUILD_TYPE%\*.exe) do echo   %%~nxF
) else if exist "bin" (
    for %%F in (bin\*.exe) do echo   %%~nxF
)
echo.

REM --- Copy game data alongside binaries for a portable package ---
set "BIN_DIR=bin\%BUILD_TYPE%"
if not exist "%BIN_DIR%" set "BIN_DIR=bin"

if not exist "%BIN_DIR%\data" (
    if exist "%PROJECT_ROOT%\data" (
        echo Copying game data into output directory...
        xcopy /E /I /Q "%PROJECT_ROOT%\data" "%BIN_DIR%\data" >nul
        echo   data\ copied
    )
)
echo.

REM --- Run tests if requested ---
if "%RUN_TESTS%"=="1" (
    echo ================================================
    echo Running Tests
    echo ================================================
    echo.
    set "TEST_FAIL=0"

    if exist "%BIN_DIR%\AtlasTests.exe" (
        echo --- Atlas Engine Tests ---
        "%BIN_DIR%\AtlasTests.exe"
        if errorlevel 1 set "TEST_FAIL=1"
        echo.
    ) else (
        echo AtlasTests.exe not found — skipping engine tests
    )

    if exist "%BIN_DIR%\test_systems.exe" (
        echo --- Server System Tests ---
        "%BIN_DIR%\test_systems.exe"
        if errorlevel 1 set "TEST_FAIL=1"
        echo.
    ) else (
        echo test_systems.exe not found — skipping server tests
    )

    if "%TEST_FAIL%"=="1" (
        echo.
        echo WARNING: Some tests failed. Check output above.
    ) else (
        echo All tests passed.
    )
    echo.
)

REM --- Open Visual Studio solution if requested ---
if "%OPEN_VS%"=="1" (
    echo Opening Visual Studio solution...
    for %%F in (*.sln) do (
        start "" "%%F"
        goto :found_sln
    )
    echo WARNING: No .sln file found in build directory
    :found_sln
    echo.
)

echo ================================================
echo Quick Start
echo ================================================
echo.
echo   Editor:   %BIN_DIR%\AtlasEditor.exe
echo   Client:   %BIN_DIR%\atlas_client.exe
echo   Server:   %BIN_DIR%\atlas_dedicated_server.exe
echo.
echo   Open in Visual Studio:
echo     start build\Atlas.sln
echo.
echo   Package for sharing:
echo     powershell Compress-Archive -Path build\%BIN_DIR%\* -DestinationPath novaforge_build.zip
echo.
echo Log: %LOG_FILE%
exit /b 0
