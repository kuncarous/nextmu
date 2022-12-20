@echo off
for /f %%f in ('dir /s /b *.bmd') do (
	call ".\x64\GenerateBoundingBox" "%%f"
)