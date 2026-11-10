@echo off
setlocal enabledelayedexpansion

set "SCRIPT=%~nx0"

rem Capture the script's own folder before parsing arguments: "shift" below
rem shifts %0 along with the rest, after which %~dp0 no longer points here.
set "SCRIPT_DIR=%~dp0"
if not exist "%SCRIPT_DIR%CMakeLists.txt" set "SCRIPT_DIR=%~dp0..\..\..\"

set "PRESET=vs2026"
set "TARGET_GEN=Visual Studio 18 2026"
set "OPEN_SOLUTION="

:parse_args
if "%~1"=="" goto args_done

set "ARG=%~1"
if /i "%ARG%"=="-open" (
    rem Opt-in: open the generated solution in Visual Studio when we are done.
    rem A bare generate.bat stays non-interactive so CI can call it too.
    set "OPEN_SOLUTION=1"
) else if not "%ARG:~0,1%"=="-" (
    rem A preset from CMakePresets.json, e.g. vs2026-d3d12 to draw with
    rem DirectX 12 instead of OpenGL. In a template every preset configures
    rem build/, so build.bat and run.bat work whichever one was picked last.
    set "PRESET=%ARG%"
) else (
    echo Usage: %SCRIPT% [-open] [preset]
    exit /b 1
)

shift
goto parse_args

:args_done

rem CMake does the actual work, so find a copy before anything else.
call :find_cmake
if errorlevel 1 exit /b 1

rem The "Visual Studio 18 2026" generator was added in CMake 4.2; fail early
rem with a clear message instead of a cryptic CMake error on older versions.
call :require_cmake_42
if errorlevel 1 exit /b 1

rem CMake cannot reconfigure an existing cache with a different generator, so
rem wipe the disposable build/ tree when the cached generator does not match.
if exist "%SCRIPT_DIR%build\CMakeCache.txt" (
    set "CACHED_GEN="
    for /f "tokens=2 delims==" %%a in ('findstr /b /c:"CMAKE_GENERATOR:INTERNAL=" "%SCRIPT_DIR%build\CMakeCache.txt"') do set "CACHED_GEN=%%a"
    if not "!CACHED_GEN!"=="%TARGET_GEN%" (
        echo Switching generator ^(!CACHED_GEN! -^> %TARGET_GEN%^), cleaning build/
        rmdir /s /q "%SCRIPT_DIR%build"
    )
)

pushd "%SCRIPT_DIR%" >nul

"%CMAKE_EXE%" --preset %PRESET%
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul

if not "%EXIT_CODE%"=="0" exit /b %EXIT_CODE%

rem Tell the reader where the solution landed (Visual Studio 2026
rem uses the .slnx solution format).
set "SOLUTION="
for %%f in ("%SCRIPT_DIR%build\*.slnx") do (
    if not defined SOLUTION if exist "%%~ff" set "SOLUTION=%%~ff"
)

if not defined SOLUTION (
    echo.
    echo Warning: configure succeeded but no solution was found in "%SCRIPT_DIR%build".
    exit /b 0
)

echo.
echo Generated: !SOLUTION!

if defined OPEN_SOLUTION (
    echo Opening it in Visual Studio...
    start "" "!SOLUTION!"
) else (
    echo Open it in Visual Studio ^(or re-run: %SCRIPT% -open^), or run build.bat -debug
)

exit /b 0

rem ---------------------------------------------------------------------------
rem Locate CMake and leave the path in CMAKE_EXE.
rem
rem Preferred: a CMake the student installed themselves, on the PATH. Fallback:
rem the copy Visual Studio installs as part of the "Desktop development with
rem C++" workload. That copy is not on the PATH outside the Developer Command
rem Prompt, but vswhere (always installed alongside Visual Studio, at a fixed
rem location) can point us straight at it - so a machine that can build the
rem engine at all never needs a separate CMake install.
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

rem ---------------------------------------------------------------------------
rem Verify CMAKE_EXE is new enough for the Visual Studio 2026 generator.
rem ---------------------------------------------------------------------------
:require_cmake_42
setlocal
set "CMAKE_VER="
for /f "usebackq tokens=3" %%v in (`"!CMAKE_EXE!" --version`) do (
    set "CMAKE_VER=%%v"
    goto :got_cmake_version
)

:got_cmake_version
if not defined CMAKE_VER (
    echo Could not determine the version of "!CMAKE_EXE!".
    endlocal & exit /b 1
)
rem A bundled CMake reports something like "4.3.1-msvc1"; only major.minor matter.
for /f "tokens=1,2 delims=.-" %%a in ("!CMAKE_VER!") do (
    set "CMAKE_MAJOR=%%a"
    set "CMAKE_MINOR=%%b"
)
set "CMAKE_OK="
if !CMAKE_MAJOR! GTR 4 set "CMAKE_OK=1"
if !CMAKE_MAJOR! EQU 4 if !CMAKE_MINOR! GEQ 2 set "CMAKE_OK=1"
if not defined CMAKE_OK (
    echo Visual Studio 2026 generation requires CMake 4.2 or newer ^(found !CMAKE_VER!^).
    echo Upgrade CMake or Visual Studio.
    endlocal & exit /b 1
)
endlocal & exit /b 0
