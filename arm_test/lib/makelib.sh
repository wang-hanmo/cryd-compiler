gcc -c sylib.c
ar -rc libsysy.a sylib.o
gcc -fPIC -shared -o libsysy.so sylib.o

