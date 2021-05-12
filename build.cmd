gcc -g -c -DMG_ENABLE_CALLBACK_USERDATA=1 *.c
gcc *.o --shared -lpthread -lws2_32 -o goat-web.dll
