#!/bin/bash

echo $1 >> func_test_result.txt

# 编译ir2c
cp ./functional/$1.sy ./$1.c
../build/compiler $1.c -I &>>/dev/null
# 判断ir2c是否失败
if ! test -f ir2c_$1.c;then
    echo IR2C_ERROR >> ir2c_$1.out
fi
# 判断gcc是否失败
gcc ir2c_$1.c sylib.c -o ir2c_$1 -O2 &>>/dev/null
if ! test -f ir2c_$1;then
    echo GCC_ERROR >> ir2c_$1.out
fi

# 编译成功则执行
# 若有输入,则用cat和管道输入
# 输出重定向保存
if test -f ir2c_$1;then
    if test -f ./functional/$1.in;then
        cat ./functional/$1.in | ./ir2c_$1 >> ir2c_$1.out 2>>/dev/null
    else
        ./ir2c_$1 >> ir2c_$1.out 2>>/dev/null
    fi
fi
ret=$?
# 用$?获取返回值
"./add_newline" ir2c_$1.out &>>/dev/null
echo $ret >> ir2c_$1.out

# 使用diff命令比较，结果存放在func_test_result.txt中
# 命令行中只输出fail或pass，func_test_result.txt中输出全部内容
cp ./functional/$1.out ./$1.out
#为了支持脚本运行在windows系统下，删除\r
# cat ./$1.out|tr -d '\r' > ./$1.out
echo -----------output----------- >> func_test_result.txt
cat ir2c_$1.out  >> func_test_result.txt
echo -----------trace----------- >> func_test_result.txt
cat ./$1.out >> func_test_result.txt
echo ------------------------------ >> func_test_result.txt
diff -u --strip-trailing-cr ir2c_$1.out ./$1.out>> /dev/null
if test $? == 0;then
    echo $1 pass! >> func_test_result.txt
    echo $1 pass!
    # 删除文件
    rm ./$1.c
    rm ./ir2c_$1.out
    rm ./$1.out
    if test -f ir2c_$1.c;then
        rm ./ir2c_$1.c
    fi
    if test -f ir2c_$1;then
        rm ./ir2c_$1
    fi
else
    echo $1 fail! >> func_test_result.txt
    echo $1 fail!
    # 移动文件至debug文件夹
    if ! test -d func_test_debug;then
        mkdir func_test_debug
    fi
    cd func_test_debug
    if ! test -d $1;then
        mkdir $1
    fi
    mv ../$1.c ./$1/
    mv ../ir2c_$1.out ./$1/
    mv ../$1.out ./$1/
    if test -f ../ir2c_$1.c;then
        mv ../ir2c_$1.c ./$1
    fi
    if test -f ../ir2c_$1;then
        mv ../ir2c_$1 ./$1
    fi
    cd ..
fi


