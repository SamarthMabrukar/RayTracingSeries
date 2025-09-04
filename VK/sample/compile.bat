echo off
del SamLogFile.txt
del resource.res
del Shader.vert.spv
del Shader.frag.spv
del FirstWindow.exe


echo Vertex Shader Build:
glslangValidator.exe -V -H -o Shader.vert.spv Shader.vert
if %errorlevel% equ 0 (
    echo [SUCCESS] Vertex shader compiled successfully
) else (
    echo [FAILED] Vertex shader compilation failed with error %errorlevel%
    exit /b %errorlevel%
)

echo.
echo Fragment Shader Build:
glslangValidator.exe -V -H -o Shader.frag.spv Shader.frag
if %errorlevel% equ 0 (
    echo [SUCCESS] Fragment shader compiled successfully
) else (
    echo [FAILED] Fragment shader compilation failed with error %errorlevel%
    exit /b %errorlevel%
)

echo.
echo Both Shaders compiled successfully!

cl /EHsc /c /nologo VKWindow.cpp /I "%VULKAN_SDK%/Include"
if %errorlevel% equ 0 (
    echo [SUCCESS] Code Compilation successful.
) else (
    echo [FAILED] Code Compilation failed with error %errorlevel%
    exit /b %errorlevel%
)


link VKWindow.obj /SUBSYSTEM:WINDOWS /LIBPATH:"%VULKAN_SDK%/Lib" /OUT:FirstWindow.exe /nologo user32.lib gdi32.lib
if %errorlevel% equ 0 (
    echo [SUCCESS] Code Linking successful.
) else (
    echo [FAILED] Code Linking failed with error %errorlevel%
    exit /b %errorlevel%
)
