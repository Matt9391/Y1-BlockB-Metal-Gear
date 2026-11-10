@echo off
setlocal

rem Capture the script's own folder before parsing arguments: "shift" below
rem shifts %0 along with the rest, after which %~dp0 no longer points here but
rem at whatever directory the script was started from.
set "SCRIPT_DIR=%~dp0"
if not exist "%SCRIPT_DIR%CMakeLists.txt" set "SCRIPT_DIR=%~dp0..\..\..\"

set "CONFIG=Debug"
set "CONFIG_ARG=-debug"
set "GAME=metal_gear"
set "FORWARD="

:parse_args
if "%~1"=="" goto args_done

if /i "%~1"=="-debug" (
    set "CONFIG=Debug"
    set "CONFIG_ARG=-debug"
    shift
    goto parse_args
)

if /i "%~1"=="-release" (
    set "CONFIG=Release"
    set "CONFIG_ARG=-release"
    shift
    goto parse_args
)

rem cmd.exe splits "-game=name" into two args at the "=", so accept both
rem the quoted form ("-game=name" as a single %1) and the unquoted one.
if /i "%~1"=="-game" (
    if "%~2"=="" (
        echo Usage: %~nx0 [-debug^|-release] [-game=name] [-setting=value ...]
        exit /b 1
    )
    set "GAME=%~2"
    shift
    shift
    goto parse_args
)

set "ARG=%~1"
if /i "%ARG:~0,6%"=="-game=" (
    set "GAME=%ARG:~6%"
    shift
    goto parse_args
)

rem Anything else (e.g. -window_width=1920) is an engine setting; hand it to
rem the game executable, which overrides engine_settings with it. Like -game
rem above, cmd.exe splits an unquoted "-name=value" at the "=" into two args,
rem so rejoin a "-name" token with the value token that follows it. The next
rem token is a value only when it does not itself start with "-".
set "NEXT=%~2"
if not "%NEXT%"=="" if not "%NEXT:~0,1%"=="-" (
    set "FORWARD=%FORWARD% %~1=%~2"
    shift
    shift
    goto parse_args
)

set "FORWARD=%FORWARD% %~1"
shift
goto parse_args

:args_done

set "EXE_DIR=%SCRIPT_DIR%build\%CONFIG%"
set "EXE_PATH=%EXE_DIR%\%GAME%.exe"

if not exist "%EXE_PATH%" (
    set "EXE_PATH=%EXE_DIR%\%GAME%_d.exe"
)

if not exist "%EXE_PATH%" (
    echo Could not find %GAME%.exe in "%EXE_DIR%".
    echo Run build.bat %CONFIG_ARG% first.
    exit /b 1
)

pushd "%EXE_DIR%" >nul

"%EXE_PATH%"%FORWARD%
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul
exit /b %EXIT_CODE%
