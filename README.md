# NextMU
NextMU is a rework of the famous MMORPG Mu Online.

# Dependencies
## From Repositories
Some dependencies will be cloned from their repositories as part of NextMU repository, following is the list of dependencies:
 - SDL2 (https://www.libsdl.org/)
 - Diligent Engine (https://diligentgraphics.com/)
 - GLM (https://glm.g-truc.net/)
 
## From External Links
Some dependencies are required to be downloaded and installed from their websites, follow is the list of dependencies:
 - FMOD (https://www.fmod.com/download#fmodengine)
 - FreeImage (https://freeimage.sourceforge.io/download.html)
 - NoesisGUI (https://www.noesisengine.com/licensing.php)
 
## Setup
### Tools
Install latest CMake which will be used in differents dependencies to create visual studio solutions.

### SDL2
Open CMake and select Dependencies/Repositories/SDL and Dependencies/Repositories/SDL/windows-x64 as output, press Configure button and configure it for Visual Studio 2022, leave platform empty, after it finish configure all press in Generate, this will create a 64-bits solution. To create a 32-bits solution change the output to Dependencies/Repositories/SDL/windows-x86 and configure platform for Win32.

Open the generated solutions (windows-x86 and windows-x64) and compile them.

### Diligent Engine
Open CMake and select Dependencies/Repositories/SDL and Dependencies/Repositories/SDL/windows-x64 as output, press Configure button and configure it for Visual Studio 2022, leave platform empty, after it finish configure all press in Generate, this will create a 64-bits solution. To create a 32-bits solution change the output to Dependencies/Repositories/SDL/windows-x86 and configure platform for Win32.

Open the generated solutions (windows-x86 and windows-x64) and compile them.

### FMOD
Download and install FMOD Studio API, copy all folders from the api folder that is inside the installation folder into ./Dependencies/Installed/FMOD folder (create FMOD folder if required).

Remember FMOD is a paid library, it has a free license option but you have to check if you can apply for it.

### NoesisGUI
Download and decompress latest SDK in ./Dependencies/Installed/NoesisGUI (create NoesisGUI folder if required) then compile it using Build folder solutions (NoesisGUI-win-x86_64.sln for Windows 64-Bits and NoesisGUI-win-x86.sln for Windows 32-Bits). This UI library will allow you to develop your UI using XAML and C#. It has support for Android and iOS.

Remember NoesisGUI is a paid library, it has a free license option but you have to check if you can apply for it.

# Android
Tutorial not available yet, however if you know what you do you will be able to compile it.