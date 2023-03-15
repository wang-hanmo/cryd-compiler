@echo off
Set filename=%1
echo %filename%>> perf_test_result.txt

:: 编译ir2c
cp .\performance\%filename%.sy %filename%.c
"..\build\compiler.exe" %filename%.c -I >nul
:: 判断ir2c是否失败
if not exist ir2c_%filename%.c echo IR2C_ERROR>> ir2c_%filename%.out
gcc ir2c_%filename%.c sylib.c -o ir2c_%filename%.exe -O2
:: 判断gcc是否失败
if not exist ir2c_%filename%.exe ( echo GCC_ERROR>> ir2c_%filename%.out )


:: 编译成功则执行,若有输入，则用cat和管道输入，输出重定向保存
if exist ir2c_%filename%.exe (
     if exist .\performance\%filename%.in (
        cat .\performance\%filename%.in | .\ir2c_%filename%.exe >> ir2c_%filename%.out
    ) else (
        .\ir2c_%filename%.exe >> ir2c_%filename%.out
    )
)

:: 用errorlevel获取main函数返回值
@echo off
set /a "RV=%ERRORLEVEL% & 255"
".\add_newline.exe" ir2c_%filename%.out
(echo %RV%) >> ir2c_%filename%.out
sed -i -e 's/\r\n/\n/g'  ir2c_%filename%.out

:: 使用fc命令比较，结果存放在func_test_result.txt中，命令行中只输出fail或pass，func_test_result.txt中输出全部内容
cp .\performance\%filename%.out .\%filename%.out
fc ir2c_%filename%.out %filename%.out >> func_test_result.txt
fc ir2c_%filename%.out %filename%.out >nul
if ERRORLEVEL 1 (
    echo %filename% failed!>> func_test_result.txt
    echo %filename% failed!
    :: 保存文件至debug文件夹
    if not exist .\func_test_debug\ mkdir func_test_debug
    cd func_test_debug
    if not exist .\%filename%\ mkdir %filename%
    cp ..\%filename%.c .\%filename%\%filename%.c
    if exist ..\ir2c_%filename%.c cp ..\ir2c_%filename%.c .\%filename%\ir2c_%filename%.c
    if exist ..\ir2c_%filename%.exe cp ..\ir2c_%filename%.exe .\%filename%\ir2c_%filename%.exe
    cp ..\ir2c_%filename%.out .\%filename%\ir2c_%filename%.out
    cp ..\%filename%.out .\%filename%\%filename%.out
    cd ..
) else (
    echo %filename% passed!>> func_test_result.txt
    echo %filename% passed!
)

:: 删除文件
del %filename%.c
if exist ir2c_%filename%.c del ir2c_%filename%.c
if exist ir2c_%filename%.exe del ir2c_%filename%.exe
del ir2c_%filename%.out
del %filename%.out
