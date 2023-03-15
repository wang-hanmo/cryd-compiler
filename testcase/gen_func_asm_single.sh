#!/bin/bash
../build/compiler ./functional/$1.sy -S
if test -f test.s;then
    mv test.s ./asm/$1.s
fi
