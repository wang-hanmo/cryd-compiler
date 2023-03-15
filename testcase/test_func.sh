#!/bin/bash
if test -f func_test_result.txt;then
    rm func_test_result.txt
fi
if test -d func_test_debug;then
    rm -rf func_test_debug
fi
g++ -o add_newline ./tools_src/add_newline.cpp
for filename in   `cat func_test_filename.txt`
do
    ./test_func_single.sh $filename
done