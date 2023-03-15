@ if exist func_test_result.txt @del func_test_result.txt
@ if exist func_test_debug\ @rmdir func_test_debug 2>nul
@ g++ -o add_newline.exe .\tools_src\add_newline.cpp
@ for /f "delims=" %%i in (func_test_filename.txt) do @(
    .\test_func_single.bat %%i 2>nul
)
