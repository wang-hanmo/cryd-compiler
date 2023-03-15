#!/bin/bash
if test -d asm;then
    rm -rf asm
fi
mkdir asm
for filename in `cat func_test_filename.txt`
do
    ./gen_func_asm_single.sh $filename &>>/dev/null
    echo $filename done!
done
