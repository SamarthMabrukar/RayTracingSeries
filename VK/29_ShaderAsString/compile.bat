del SamLogFile.txt
del resource.res
del FirstWindow.exe

rc /fo resource.res /nologo VK.rc

cl /EHsc /c /nologo /MD VKWindow.cpp /I "%VULKAN_SDK%/Include" /I "%VULKAN_SDK%/Include/glslang/Include"

link VKWindow.obj resource.res /SUBSYSTEM:WINDOWS /LIBPATH:"%VULKAN_SDK%/Lib" /OUT:FirstWindow.exe /nologo user32.lib gdi32.lib vulkan-1.lib GenericCodeGen.lib glslang-default-resource-limits.lib glslang.lib MachineIndependent.lib OSDependent.lib SPIRV-Tools-opt.lib SPIRV-Tools.lib SPIRV.lib SPVRemapper.lib
