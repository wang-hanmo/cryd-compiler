@echo off
Set filename=%1

"..\build\compiler.exe" .\performance\%filename%.sy -S -O2 >>out.txt
if exist test.s mv test.s .\asm\%filename%.s
