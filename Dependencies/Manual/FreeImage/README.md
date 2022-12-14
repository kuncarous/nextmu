# FreeImage-Android
FreeImage compiled for Android (based on https://github.com/jamcar23/FreeImage-Android)

# Before compile
Download FreeImage source distribution from https://freeimage.sourceforge.io/download.html then decompress it in Dependencies/Installed/FreeImage folder (if Installed is missing create it) and copy jni folder to FreeImage folder.

# How to compile
Execute next command:
```ndk-build APP_PLATFORM:=android-31 -j8 V=1```

If you are using command prompt in Windows configure ANDROID_NDK_HOME, ANDROID_NDK_ROOT and PATH to point to your NDK path before execute the command.
For example you can use next commands to only configure these environment variables for the current command prompt session:
```
set ANDROID_NDK_HOME="YOUR_NDK_PATH"
set ANDROID_NDK_ROOT="YOUR_NDK_PATH"
set PATH=%PATH%;"YOUR_NDK_PATH"
```