del SamLogFile.txt
del resource.res
del FirstWindow.exe

glslangValidator.exe -V -H -o Shader.vert.spv Shader.vert

glslangValidator.exe -V -H -o Shader.frag.spv Shader.frag

rc /fo resource.res /nologo VK.rc
cl /EHsc /c /nologo VKWindow.c /I "%VULKAN_SDK%/Include"
link VKWindow.obj resource.res /SUBSYSTEM:WINDOWS /LIBPATH:"%VULKAN_SDK%/Lib" /OUT:FirstWindow.exe /nologo user32.lib gdi32.lib
