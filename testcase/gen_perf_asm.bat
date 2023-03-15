@ if exist asm\ @rmdir asm 2>nul
@ mkdir asm 2>nul
@ for /f "delims=" %%i in (perf_test_filename.txt) do @(
    .\gen_perf_asm_single.bat %%i
    echo %%i done!
)