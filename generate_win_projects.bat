@echo off
@setlocal enableextensions
@cd /d "%~dp0"
setlocal

:: Execute a premake command
call vendor\premake\bin\premake5.exe vs2022

:: Get the current directory of the batch file, removing trailing backslash
set "BATCH_DIR=%~dp0"
set "BATCH_DIR=%BATCH_DIR:~0,-1%"

:: Display the full path for confirmation
echo Batch file directory: %BATCH_DIR%

:: Create or update HVE_ROOT_DIR environment variable
echo Setting HVE_ROOT_DIR environment variable...
setx HVE_ROOT_DIR "%BATCH_DIR%" /M

endlocal
PAUSE