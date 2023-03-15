#!/bin/bash
if test -f result.txt;then
    rm result.txt
fi

if test -d bin;then
    rm -rf bin
    mkdir bin
fi
g++ -o ../tools/add_newline ../tools/add_newline.cpp
for filename in   `cat filename.txt`
do
    ./test_single.sh $filename
done