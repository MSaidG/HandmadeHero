@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
g++ -Og -mwindows -mwin32 -march=native -Wall -D _WIN32=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -cpp -g -fuse-ld=lld ..\code\win32_handmade.cpp -o win32_handmade.exe -Wl,-static,-subsystem,windows -lgdi32
popd

:: -flto
:: g++ -mwindows -mwin32 -march=native -Wall -D _WIN32=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -cpp -g -fuse-ld=lld -flto ..\code\win32_handmade.cpp -o win32_handmade.exe -Wl,-static,-subsystem,windows -lgdi32
 