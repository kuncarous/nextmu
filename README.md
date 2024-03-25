# NextMU
NextMU is a rework of the famous MMORPG Mu Online.

# Sponsors
Special thanks to our donators and sponsors, as a way to express our gratitude we list each donator in discord and also we will list our sponsors here so everyone can be aware about who donated and sponsored the project, thank you! we really appreciate it!

<a href="https://www.mudevs.com"><img src="https://i.ibb.co/FbPKs0n/logo.png" height="140px" alt="logo" border="0"></a>

# Donations
If you want to donate, you can donate through any platform listed below

<a href="https://www.buymeacoffee.com/nextmu" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

<a href='https://ko-fi.com/X8X2TY8T1' target='_blank'><img height='36' style='border:0px;height:36px;' src='https://storage.ko-fi.com/cdn/kofi2.png?v=3' border='0' alt='Buy Me a Coffee at ko-fi.com' /></a>

# Discord
If you want to join our discord, press the button below

<a href="https://discord.gg/ARafEy92hp"><img src="https://discord.com/api/guilds/1025209137430265996/widget.png?style=banner2" alt="Discord server"></a>

# Dependencies
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

If you are using linux be sure to install at least CMake 3.22 from the package manager.

### FMOD
Instructions soon

### NoesisGUI
Extract it into ```./dependencies/installed/noesisgui``` (you will need to create installed and noesisgui folder, remember to make it lowercase).

## Setup
### Windows
Open CMake, choose the repository folder, set ```builds/windows-(x64|x86|arm|arm64|arm64ec)``` as output directory, press configure, choose Visual Studio 2022, select the platform (x64|Win32|ARM|ARM64|ARM64EC), wait until it finish configure everything, change ```NEXTMU_COMPANY_NAME``` and ```NEXTMU_GAME_NAME``` to match your Company (if you don't have one, just use a fantasy name) and your Game name, press generate, go to the ```builds/windows-(x64|x86|arm|arm64|arm64ec)``` folder, open the solution and compile.

After you finish building everything you will be able to find the executable and the DLLs inside of the ```builds/windows-(x64|x86|arm|arm64|arm64ec)/client/windows/(Debug|Release)``` folder.

### Linux
Ubuntu is the current distro tested, first you need to install CMake, cURL and a few others things, you can follow the next instructions:

```
sudo apt install -y cmake curl python3 pip libstdc++-12-dev libx11-dev libxext-dev libgl-dev libglx-dev libtinfo5
bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
```

Install CLion following this page instructions https://www.jetbrains.com/help/clion/installation-guide.html#toolbox, if you want you can use another IDE.

After that you can configure the CMake and compile it, CLion can do it for you.

### MacOS
Open CMake, choose the repository folder, set ```builds/macos``` as output directory, press configure, choose Xcode, wait until it finish configure everything, change ```NEXTMU_COMPANY_NAME``` and ```NEXTMU_GAME_NAME``` to match your Company (if you don't have one, just use a fantasy name) and your Game name, press generate, open Xcode, open Xcode project from ```builds/macos``` folder, configure the scheme to use NextMu executable and build.

### Android
Open the repository folder with Android Studio, let it start configuring everything, build and that is all. Remember to always use the latest NDK, also install the latest CMake and configure it inside local.properties file inside the root repository directory (if you don't have one just create it), configure it using cmake.dir, you can search in deep instructions in Google, you will find an official instruction from Android Studio.

# Licenses
## Requirements
Each license listed below are required to be listed in a user manual as part of your distribution, some licenses might have special requirements which can make this optional or permit you to provide it in a differet way.

## NoesisGUI
Commercial, please read their pricing page (https://www.noesisengine.com/licensing.php) and follow their instructions, it is a really good commercial UI library and provides lot of features, if you don't want to use it you can modify the code and change to another library but I recommend this library due performance and flexibility beside stability and good price.

## FreeImage
### How to include the license
List it inside the user manual of your application distribution, if there is no way to access to it (like in Android and iOS) I recommend to include it in a credits screen.

### License
This software uses the FreeImage open source image library. See http://freeimage.sourceforge.net for details.
FreeImage is used under the FIPL, version 1.0.
