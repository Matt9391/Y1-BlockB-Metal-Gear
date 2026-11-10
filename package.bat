@echo off
setlocal enabledelayedexpansion

rem Packages a game to share or hand in: package\<game>-windows.zip holds a
rem Release build and its content folder, ready to run on another machine.
rem With -web, package\<game>-web.zip is the game as a web page instead.
rem Packages are shipping builds (no logging) unless -development is given.
rem   package.bat [-game=name] [-web] [-development]
rem The work happens in cmake\package.cmake.

rem Capture the script's own folder before parsing arguments: "shift" below
rem shifts %0 along with the rest, after which %~dp0 no longer points here.
set "SCRIPT_DIR=%~dp0"
if not exist "%SCRIPT_DIR%CMakeLists.txt" set "SCRIPT_DIR=%~dp0..\..\..\"

set "GAME=sample_2d"
set "PLATFORM=windows"
set "BUILD=shipping"

:parse_args
if "%~1"=="" goto args_done

rem cmd.exe splits "-game=name" into two args at the "=", so accept both
rem the quoted form ("-game=name" as a single %1) and the unquoted one.
if /i "%~1"=="-game" (
    if "%~2"=="" goto usage
    set "GAME=%~2"
    shift
    shift
    goto parse_args
)

if /i "%~1"=="-web" (
    set "PLATFORM=web"
    shift
    goto parse_args
)

if /i "%~1"=="-development" (
    set "BUILD=development"
    shift
    goto parse_args
)

set "ARG=%~1"
if /i "%ARG:~0,6%"=="-game=" (
    set "GAME=%ARG:~6%"
    shift
    goto parse_args
)

:usage
echo Usage: %~nx0 [-game=name] [-web] [-development]
exit /b 1

:args_done

rem CMake does the actual work, so find a copy before anything else.
call :find_cmake
if errorlevel 1 exit /b 1

"%CMAKE_EXE%" -DGAME=%GAME% -DPLATFORM=%PLATFORM% -DBUILD=%BUILD% -P "%SCRIPT_DIR%cmake\package.cmake"
exit /b %ERRORLEVEL%

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
