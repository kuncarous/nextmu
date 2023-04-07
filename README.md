# NextMU
NextMU is a rework of the famous MMORPG Mu Online.

## Dependencies (vcpkg)
### Windows (64 bits)
```
vcpkg install cryptopp:x64-windows entt:x64-windows fmt:x64-windows glm:x64-windows nlohmann-json:x64-windows freeimage:x64-windows
```

### Windows (32 bits)
```
vcpkg install cryptopp:x86-windows entt:x86-windows fmt:x86-windows glm:x86-windows nlohmann-json:x86-windows freeimage:x86-windows
```

# Dependencies
## From Repositories
Some dependencies will be cloned from their repositories as part of NextMU repository, following is the list of dependencies:
 - SDL2 (https://www.libsdl.org/)
 - bgfx (https://bkaradzic.github.io/bgfx/)
 - GLM (https://glm.g-truc.net/)
 
## From External Links
Some dependencies are required to be downloaded and installed from their websites, follow is the list of dependencies:
 - CGLM (https://github.com/recp/cglm/tags)
 - FMOD (https://www.fmod.com/download#fmodengine)
 - FreeImage (https://freeimage.sourceforge.io/download.html)
 - NoesisGUI (https://www.noesisengine.com/licensing.php)
 
## Setup
### Tools
Install latest CMake which will be used in differents dependencies to create visual studio solutions.

### SDL2
Open CMake and select Dependencies/Repositories/SDL and Dependencies/Repositories/SDL/build as output, press Configure button and configure it for Visual Studio 2022, leave platform empty, after it finish configure all press in Generate, this will create a 64-bits solution. To create a 32-bits solution change the output to Dependencies/Repositories/SDL/build-x86 and configure platform for Win32.

Open the generated solutions (build and build-x64) and compile them.

### bgfx
Open a command prompt (cmd), go to ./Dependencies/Repositories folder and execute prepare_bgfx.bat file, this will create .build/projects/vs2022/bgfx.sln solution, compile it for 32 and 64 bits.

### CGLM
Download the latest version and decompress it in ./Dependencies/Installed, rename the folder to cglm.

### FMOD
Download and install FMOD Studio API, copy all folders from the api folder that is inside the installation folder into ./Dependencies/Installed/FMOD folder (create FMOD folder if required).

Remember FMOD is a paid library, it has a free license option but you have to check if you can apply for it.

### NoesisGUI
Download and decompress latest SDK in ./Dependencies/Installed/NoesisGUI (create NoesisGUI folder if required) then compile it using Build folder solutions (NoesisGUI-win-x86_64.sln for Windows 64-Bits and NoesisGUI-win-x86.sln for Windows 32-Bits). This UI library will allow you to develop your UI using XAML and C#. It has support for Android and iOS.

Remember NoesisGUI is a paid library, it has a free license option but you have to check if you can apply for it.

## Shaders
### Setup
After followed dependencies instructions you are ready to configure your environment to compile the shaders, add BGFX_PATH and make it point to {...}\Dependencies\Repositories\bgfx (use absolute path), now you can execute compile.bat files and will generate the shaders for the compatible APIs and Operating Systems.

# Android
Tutorial not available yet, however if you know what you do you will be able to compile it.