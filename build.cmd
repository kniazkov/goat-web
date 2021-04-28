gcc -g -c *.c
gcc *.o --shared -lpthread -lws2_32 -o goat-web.dll
