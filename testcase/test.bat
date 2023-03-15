Set filename=%1
cd ./custom
"../../build/compiler.exe" %filename%.c
cp ir2c_%filename%.c ../ir2c_%filename%.c
del ir2c_%filename%.c
cd ..