@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0"
if not exist "%ROOT%CMakeLists.txt" set "ROOT=%~dp0..\..\..\"

set "CONFIG=debug"

if not "%~2"=="" (
    echo Usage: %~nx0 [-debug^|-release]
    exit /b 1
)

if /i "%~1"=="-debug" (
    set "CONFIG=debug"
) else if /i "%~1"=="-release" (
    set "CONFIG=release"
) else if not "%~1"=="" (
    echo Usage: %~nx0 [-debug^|-release]
    exit /b 1
)

if not exist "%ROOT%build\CMakeCache.txt" (
    echo No build/ folder yet - run generate.bat first.
    exit /b 1
)

rem CMake does the actual work, so find a copy before anything else.
call :find_cmake
if errorlevel 1 exit /b 1

set "PRESET=%CONFIG%"

pushd "%ROOT%" >nul

"%CMAKE_EXE%" --build --preset %PRESET%
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul
exit /b %EXIT_CODE%

rem ---------------------------------------------------------------------------
rem Locate CMake and leave the path in CMAKE_EXE.
rem
rem Preferred: a CMake the student installed themselves, on the PATH. Fallback:
rem the copy Visual Studio installs as part of the "Desktop development with
rem C++" workload. That copy is not on the PATH outside the Developer Command
rem Prompt, but vswhere (always installed alongside Visual Studio, at a fixed
rem location) can point us straight at it - so a machine that can build the
rem engine at all never needs a separate CMake install.
rem
rem Kept in sync with the identical routine in generate.bat.
rem ---------------------------------------------------------------------------
:find_cmake
set "CMAKE_EXE="

where cmake >nul 2>&1
if not errorlevel 1 (
    set "CMAKE_EXE=cmake"
    exit /b 0
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        if exist "%%i\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
            set "CMAKE_EXE=%%i\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        )
    )
)

if defined CMAKE_EXE (
    echo Using the CMake bundled with Visual Studio:
    echo   !CMAKE_EXE!
    exit /b 0
)

echo Could not find CMake.
echo.
echo Install it with:  winget install Kitware.CMake
echo or install the "Desktop development with C++" workload in Visual Studio,
echo which ships its own copy that this script can find on its own.
exit /b 1
