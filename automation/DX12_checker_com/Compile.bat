@echo off
setlocal EnableDelayedExpansion

echo =============================================
echo  ShaderChecker COM Component - Build Script
echo  Portable Version - Works on any machine
echo =============================================
echo.

REM Get the directory where this script is located (portable)
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

echo Working directory: %CD%
echo.

REM Set relative paths (works when folder is copied anywhere)
set "SDK_INCLUDE=%SCRIPT_DIR%microsoft.direct3d.d3d12.1.717.1-preview\build\native\include"

REM Check if source files exist
if not exist "%SCRIPT_DIR%ShaderChecker.cpp" (
    echo ERROR: ShaderChecker.cpp not found!
    echo   Expected at: %SCRIPT_DIR%ShaderChecker.cpp
    pause
    exit /b 1
)

if not exist "%SDK_INCLUDE%\d3d12.h" (
    echo ERROR: Agility SDK headers not found!
    echo   Expected at: %SDK_INCLUDE%
    echo.
    echo   Make sure the microsoft.direct3d.d3d12.1.717.1-preview folder exists.
    pause
    exit /b 1
)

REM Try to find Visual Studio automatically
set "VS_FOUND=0"

REM Check if already in VS environment
where cl.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Found: cl.exe already in PATH
    set "VS_FOUND=1"
    goto :build
)

REM Try VS 2022 Community
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    echo Found: Visual Studio 2022 Community
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
    set "VS_FOUND=1"
    goto :build
)

REM Try VS 2022 Professional
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    echo Found: Visual Studio 2022 Professional
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
    set "VS_FOUND=1"
    goto :build
)

REM Try VS 2022 Enterprise
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
    echo Found: Visual Studio 2022 Enterprise
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
    set "VS_FOUND=1"
    goto :build
)

REM Try VS 2019 Community
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    echo Found: Visual Studio 2019 Community
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
    set "VS_FOUND=1"
    goto :build
)

REM Try VS 2019 Professional
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    echo Found: Visual Studio 2019 Professional
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
    set "VS_FOUND=1"
    goto :build
)

REM Try Build Tools
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    echo Found: Visual Studio 2022 Build Tools
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
    set "VS_FOUND=1"
    goto :build
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    echo Found: Visual Studio 2019 Build Tools
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
    set "VS_FOUND=1"
    goto :build
)

echo.
echo ERROR: Visual Studio not found!
echo.
echo Please install one of:
echo   - Visual Studio 2022 (any edition)
echo   - Visual Studio 2019 (any edition)  
echo   - Visual Studio Build Tools
echo.
echo Or run this script from a Developer Command Prompt.
pause
exit /b 1

:build
echo.

REM Clean old files
echo Cleaning old files...
del "%SCRIPT_DIR%ShaderChecker.obj" 2>nul
del "%SCRIPT_DIR%ShaderChecker.dll" 2>nul
del "%SCRIPT_DIR%ShaderChecker.lib" 2>nul
del "%SCRIPT_DIR%ShaderChecker.exp" 2>nul
del "%SCRIPT_DIR%ShaderChecker.tlb" 2>nul
del "%SCRIPT_DIR%ShaderChecker.res" 2>nul

REM Step 1: Generate type library from IDL
echo.
echo Step 1: Generating type library from IDL...
midl.exe "%SCRIPT_DIR%ShaderChecker.idl" /tlb "%SCRIPT_DIR%ShaderChecker.tlb" /h "%SCRIPT_DIR%ShaderChecker_i.h" /iid "%SCRIPT_DIR%ShaderChecker_i.c"
if %ERRORLEVEL% NEQ 0 (
    echo MIDL compilation failed!
    pause
    exit /b 1
)
echo   OK

REM Step 2: Compile resource file (embeds type library)
echo.
echo Step 2: Compiling resources...
rc.exe /fo"%SCRIPT_DIR%ShaderChecker.res" "%SCRIPT_DIR%ShaderChecker.rc"
if %ERRORLEVEL% NEQ 0 (
    echo Resource compilation failed!
    echo   Continuing without embedded type library...
    set "RES_FILE="
) else (
    echo   OK
    set "RES_FILE=%SCRIPT_DIR%ShaderChecker.res"
)

REM Step 3: Compile C++ source
echo.
echo Step 3: Compiling ShaderChecker.cpp...
cl.exe /c "%SCRIPT_DIR%ShaderChecker.cpp" ^
    /EHsc ^
    /W3 ^
    /O2 ^
    /I "%SCRIPT_DIR%" ^
    /I "%SDK_INCLUDE%" ^
    /D "WIN32" ^
    /D "_WINDOWS" ^
    /D "UNICODE" ^
    /D "_UNICODE" ^
    /Fo"%SCRIPT_DIR%ShaderChecker.obj"

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    pause
    exit /b 1
)
echo   OK

REM Step 4: Link DLL
echo.
echo Step 4: Linking ShaderChecker.dll...
link.exe "%SCRIPT_DIR%ShaderChecker.obj" %RES_FILE% ^
    /DLL ^
    /DEF:"%SCRIPT_DIR%ShaderChecker.def" ^
    /OUT:"%SCRIPT_DIR%ShaderChecker.dll" ^
    d3d12.lib dxgi.lib ole32.lib oleaut32.lib user32.lib advapi32.lib uuid.lib

if %ERRORLEVEL% NEQ 0 (
    echo Linking failed!
    pause
    exit /b 1
)
echo   OK

REM Verify type library is accessible
echo.
echo Verifying type library...
if exist "%SCRIPT_DIR%ShaderChecker.tlb" (
    echo   ShaderChecker.tlb exists - OK
) else (
    echo   WARNING: ShaderChecker.tlb not found!
)

if exist "%SCRIPT_DIR%ShaderChecker.dll" (
    echo.
    echo =============================================
    echo  BUILD SUCCESSFUL!
    echo =============================================
    echo.
    echo Created: %SCRIPT_DIR%ShaderChecker.dll
    echo.
    echo Next steps (run as Administrator):
    echo   regsvr32 "%SCRIPT_DIR%ShaderChecker.dll"
    echo   python "%SCRIPT_DIR%client.py"
    echo.
    echo Or use the Python client:
    echo   python client.py register
    echo   python client.py
    echo.
) else (
    echo.
    echo BUILD FAILED - DLL not created
)

endlocal
pause
