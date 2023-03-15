#!/bin/bash

echo $1 >> result.txt

# 编译ir2c
# 判断gcc是否失败
gcc ./asm/$1.s ../lib/libsysy.a -o ./bin/$1 1>>/dev/null
if ! test -f ./bin/$1;then
    echo ERROR >> $1.out
fi

# 编译成功则执行
# 若有输入,则用cat和管道输入
# 输出重定向保存
if test -f ./bin/$1;then
    if test -f ./input/$1.in;then
        cat ./input/$1.in | ./bin/$1 >> $1.out 2>>/dev/null
    else
        ./bin/$1 >> $1.out 2>>/dev/null
    fi
fi
ret=$?
# 用$?获取返回值
"../tools/add_newline" $1.out &>>/dev/null
echo $ret >> $1.out

# 使用diff命令比较，结果存放在result.txt中
# 命令行中只输出fail或pass，result.txt中输出全部内容
echo -----------output----------- >> result.txt
cat $1.out  >> result.txt
echo -----------trace----------- >> result.txt
cat ./output/$1.out >> result.txt
echo ------------------------------ >> result.txt
diff -u --strip-trailing-cr $1.out ./output/$1.out>> /dev/null
if test $? == 0;then
    echo $1 pass! >> result.txt
    echo $1 pass!
else
    echo $1 fail! >> result.txt
    echo $1 fail!
fi
# 删除文件
rm ./$1.out


