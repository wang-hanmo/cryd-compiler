@ if exist perf_test_result.txt @del perf_test_result.txt
@ if exist perf_test_debug\ @rmdir perf_test_debug 2>nul
@ g++ -o add_newline.exe .\tools_src\add_newline.cpp
@ for /f "delims=" %%i in (perf_test_filename.txt) do @(
    .\test_perf_single.bat %%i 2>nul
)
