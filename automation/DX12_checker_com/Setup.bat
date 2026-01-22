@echo off
setlocal EnableDelayedExpansion

echo =============================================
echo  Shader Model 6.9 Checker - One-Click Setup
echo =============================================
echo.

REM Get script directory (portable)
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

REM Check for admin rights
net session >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo This script requires Administrator privileges.
    echo.
    echo Right-click Setup.bat and select "Run as administrator"
    echo.
    pause
    exit /b 1
)

echo Running as Administrator - OK
echo.

REM Step 1: Build
echo Step 1: Building DLL...
echo.
call "%SCRIPT_DIR%Compile.bat" /nobuild
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

REM Check if DLL was built


REM Step 2: Register
echo.
echo Step 2: Registering COM DLL...
regsvr32 /s "%SCRIPT_DIR%ShaderChecker.dll"
if %ERRORLEVEL% NEQ 0 (
    echo Registration failed!
    pause
    exit /b 1
)
echo   Registered successfully!

REM Step 3: Test with Python (if available)
echo.
echo Step 3: Testing...
python "%SCRIPT_DIR%client.py"

echo.
echo =============================================
echo  Setup Complete!
echo =============================================
echo.

endlocal
pause