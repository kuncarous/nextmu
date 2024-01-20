# NextMU
NextMU is a rework of the famous MMORPG Mu Online.

# Donations
If you want to donate, press the button below

<a href="https://www.buymeacoffee.com/nextmu" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

# Discord
If you want to join our discord, press the button below

<a href="https://discord.gg/ARafEy92hp"><img src="https://discord.com/api/guilds/1025209137430265996/widget.png?style=banner2" alt="Discord server"></a>

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

## Before Setup
### Vulkan SDK
Download and install the latest Vulkan SDK from https://vulkan.lunarg.com/sdk/home.

### CMake
Download and install the latest CMake from https://cmake.org/download/.

### FMOD
Instructions soon

### NoesisGUI
Instructions soon

## Setup
### Windows
Open CMake, choose the repository folder, set builds/windows-(x64|x86|arm|arm64|arm64ec) as output directory, press configure, choose Visual Studio 2022, select the platform (x64|Win32|ARM|ARM64|ARM64EC), wait until it finish configure everything, change NEXTMU_COMPANY_NAME and NEXTMU_GAME_NAME to match your Company (if you don't have one, just use a fantasy name) and your Game name, press generate, go to the builds/windows folder, open the solution and compile.

### MacOS
Open CMake, choose the repository folder, set builds/macos as output directory, press configure, choose Xcode, wait until it finish configure everything, change NEXTMU_COMPANY_NAME and NEXTMU_GAME_NAME to match your Company (if you don't have one, just use a fantasy name) and your Game name, press generate, open Xcode, open Xcode project from builds/macos folder, configure the scheme to use NextMu executable and build.

### Android
Open the repository folder with Android Studio, let it start configuring everything, build and that is all. Remember to always use the latest NDK, also install the latest CMake and configure it inside local.properties file inside the root repository directory (if you don't have one just create it), configure it using cmake.dir, you can search in deep instructions in Google, you will find an official instruction from Android Studio.

# Licenses
## Requirements
Each license listed below are required to be listed in a user manual as part of your distribution, some licenses might have special requirements which can make this optional or permit you to provide it in a differet way.

## FreeImage
### How to include the license
List it inside the user manual of your application distribution, if there is no way to access to it (like in Android and iOS) I recommend to include it in a credits screen.

### License
This software uses the FreeImage open source image library. See http://freeimage.sourceforge.net for details.
FreeImage is used under the FIPL, version 1.0.
