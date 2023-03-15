Set filename=%1
cd ./custom
"../../build/compiler.exe" %filename%.c -I 
gcc %filename%.c ../sylib.c -o %filename%.exe -O2
gcc ir2c_%filename%.c ../sylib.c -o ir2c_%filename%.exe -O2
cp %filename%.exe ../%filename%.exe
cp ir2c_%filename%.exe ../ir2c_%filename%.exe
del ir2c_%filename%.c
del %filename%.exe
del ir2c_%filename%.exe
cd ..