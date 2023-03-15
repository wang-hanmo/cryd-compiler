@echo off
Set filename=%1

"..\build\compiler.exe" .\functional\%filename%.sy -S >nul
if exist test.s mv test.s .\asm\%filename%.s
