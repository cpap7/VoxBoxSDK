@echo off
setlocal

:: Find and call VS Developer environment
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64
    goto :run_python
)
echo ERROR: Visual Studio not found
exit /b 1

:run_python
python "%~dp0Setup.py" %*

pause