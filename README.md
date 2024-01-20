# NextMU
NextMU is a rework of the famous MMORPG Mu Online.

# Donations
If you want to donate, you can do it pressing the image below

<a href="https://www.buymeacoffee.com/nextmu" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

# Dependencies
## From Repositories
Some dependencies will be cloned from their repositories as part of NextMU repository, following is the list of dependencies:
 - SDL2 (https://www.libsdl.org/)
 - Diligent Engine (https://diligentgraphics.com/)
 - GLM (https://glm.g-truc.net/)
 
## From External Links
Some dependencies are required to be downloaded and installed from their websites, follow is the list of dependencies:
 - FMOD (https://www.fmod.com/download#fmodengine)
 - NoesisGUI (https://www.noesisengine.com/licensing.php)

## Already included in this repository
### FreeImage (https://freeimage.sourceforge.io/download.html)
This library is already distributed as part of this project with its own license which can be found below, since it is a dual license library we selected the license that is compatible with the project.
 
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

# Licenses
## Requirements
Each license listed below are required to be listed in a user manual as part of your distribution, some licenses might have special requirements which can make this optional or permit you to provide it in a differet way.

## Free Image
### How to include the license
List it inside the user manual of your application distribution, if there is no way to access to it (like in Android and iOS) I recommend to include it in a credits screen.

### License
This software uses the FreeImage open source image library. See http://freeimage.sourceforge.net for details.
FreeImage is used under the FIPL, version 1.0.
