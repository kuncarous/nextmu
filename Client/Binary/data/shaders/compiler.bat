@echo off
setlocal EnableDelayedExpansion

FOR %%A IN (%*) DO (
   FOR /f "tokens=1,2 delims=:" %%G IN ("%%A") DO set %%G=%%H
)

if "%OUTDIR%" == "" (
	set OUTDIR=.\
)

if "%OUTPUT%" == "" (
	set OUTPUT=%INPUT%
)

set platform[1]=windows
set platform[2]=android

set Uplatform[1]=Windows
set Uplatform[2]=Android

set idx=0
for /L %%G in (1,1,2) do (
	echo "Compiling for !Uplatform[%%G]!"
	mkdir %CD%\%OUTDIR%\!platform[%%G]!
	
	if "!platform[%%G]!" == "windows" (
		echo "OpenGL"
		"%BGFX_PATH%\.build\win64_vs2022\bin\shadercRelease.exe" -f %CD%\%INPUT% -o %CD%\%OUTDIR%\!platform[%%G]!\%OUTPUT%.gl --platform !platform[%%G]! --type %TYPE% -i "%BGFX_PATH%\src" --profile 120 --define "%DEFINES%"
	) else (
		echo "OpenGLES"
		"%BGFX_PATH%\.build\win64_vs2022\bin\shadercRelease.exe" -f %CD%\%INPUT% -o %CD%\%OUTDIR%\!platform[%%G]!\%OUTPUT%.gles --platform !platform[%%G]! --type %TYPE% -i "%BGFX_PATH%\src" --profile 300_es --define "%DEFINES%"
	)

	echo "Vulkan"
	"%BGFX_PATH%\.build\win64_vs2022\bin\shadercRelease.exe" -f %CD%\%INPUT% -o %CD%\%OUTDIR%\!platform[%%G]!\%OUTPUT%.vk --platform !platform[%%G]! --type %TYPE% -i "%BGFX_PATH%\src" --profile spirv13-11 --define "%DEFINES%"

	if "!platform[%%G]!" == "windows" (
		echo "Direct3D 11/12"
		if "%TYPE%" == "vertex" (
			"%BGFX_PATH%\.build\win64_vs2022\bin\shadercRelease.exe" -f %CD%\%INPUT% -o %CD%\%OUTDIR%\!platform[%%G]!\%OUTPUT%.d3d --platform !platform[%%G]! --type vertex -i "%BGFX_PATH%\src" --profile vs_4_0 --define "%DEFINES%"
		) else (
			"%BGFX_PATH%\.build\win64_vs2022\bin\shadercRelease.exe" -f %CD%\%INPUT% -o %CD%\%OUTDIR%\!platform[%%G]!\%OUTPUT%.d3d --platform !platform[%%G]! --type fragment -i "%BGFX_PATH%\src" --profile ps_4_0 --define "%DEFINES%"
		)
	)
	
	set /a idx+=1
)

endlocal