@echo off
::echo "run bash : %cd%"
::echo "bash arg : %1"
for /f "usebackq tokens=*" %%a in (`git --exec-path`) do set "gitpath=%%a"
set "gitbash=%gitpath%\..\..\.."

setlocal
set "script_path=%~dp0configure_vcpkg.sh"
::echo %script_path%
start "" "%gitbash%\git-bash.exe" "%script_path%" %1 