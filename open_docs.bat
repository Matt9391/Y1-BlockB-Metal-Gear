@echo off
setlocal

rem Opens the documentation (the guides and the API reference) in your
rem browser. It is a folder of web pages in docs\, so it works offline.
set "ROOT=%~dp0"
if not exist "%ROOT%CMakeLists.txt" set "ROOT=%~dp0..\..\..\"

rem A template has the built site in docs\; the engine's own repository
rem builds it into docs\html\.
set "PAGE=%ROOT%docs\index.html"
if not exist "%PAGE%" set "PAGE=%ROOT%docs\html\index.html"
if not exist "%PAGE%" (
    echo The documentation is missing: there is no docs\index.html in this template.
    exit /b 1
)

start "" "%PAGE%"
