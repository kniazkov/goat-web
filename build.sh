gcc -g -c -DMG_ENABLE_CALLBACK_USERDATA=1 -fPIC *.c
gcc *.o --shared -lpthread -o goat-web.dll
