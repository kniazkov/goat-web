gcc -g -c -fPIC *.c
gcc *.o --shared -lpthread -o goat-web.dll
