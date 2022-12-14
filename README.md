# NextMU
NextMU is a rework of the famous MMORPG Mu Online.

# Dependencies
## From Repositories
Some dependencies will be cloned from their repositories as part of NextMU repository, following is the list of dependencies:
 - SDL2 (https://www.libsdl.org/)
 - bgfx (https://bkaradzic.github.io/bgfx/)
 - GLM (https://glm.g-truc.net/)
 
## From External Links
Some dependencies are required to be downloaded and installed from their websites, follow is the list of dependencies:
 - FMOD (https://www.fmod.com/download#fmodengine)
 - FreeImage (https://freeimage.sourceforge.io/download.html)
 - Ultralight (https://ultralig.ht/#pricing)
 - NoesisGUI (https://www.noesisengine.com/licensing.php)
 
## Setup
### Tools
Install latest CMake which will be used in differents dependencies to create visual studio solutions.

### SDL2
Open CMake and select Dependencies/Repositories/SDL and Dependencies/Repositories/SDL/build as output, press Configure button and configure it for Visual Studio 2022, leave platform empty, after it finish configure all press in Generate, this will create a 64-bits solution. To create a 32-bits solution change the output to Dependencies/Repositories/SDL/build-x86 and configure platform for Win32.

Open the generated solutions (build and build-x64) and compile them.

### FreeImage
Decompress in ./Dependencies/Installed/FreeImage (create Installed and FreeImage folders if required) and copy all files from ./Dependencies/Manual/FreeImage into it, you will be able to follow ./Dependencies/Manual/FreeImage/README.md instructions to compile it for Android.

To compile it for Windows open ./Dependencies/Installed/FreeImage/FreeImage.2017.sln (if has newer, use it) and upgrade it for Visual Studio 2022 and compile FreeImageLib project.

### FMOD
Download and install FMOD Studio API, copy all folders from the api folder that is inside the installation folder into ./Dependencies/Installed/FMOD folder (create FMOD folder if required).

Remember FMOD is a paid library, it has a free license option but you have to check if you can apply for it.

### Ultralight
Download and decompress latest SDK in ./Dependencies/Installed/ultralight (create ultralight folder if required). This UI library will allow you to develop your UI using HTML, CSS and JS however it doesn't support Android and iOS yet.

Remember Ultralight is a paid libary, it has a free license option but you have to check if you can apply for it.

### NoesisGUI
Download and decompress latest SDK in ./Dependencies/Installed/NoesisGUI (create NoesisGUI folder if required) then compile it using Build folder solutions (NoesisGUI-win-x86_64.sln for Windows 64-Bits and NoesisGUI-win-x86.sln for Windows 32-Bits). This UI library will allow you to develop your UI using XAML and C#. It has support for Android and iOS.

Remember NoesisGUI is a paid library, it has a free license option but you have to check if you can apply for it.

# Android
Tutorial not available yet, however if you know what you do you will be able to compile it.