@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
g++ -D _WIN32=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -cpp -g -fuse-ld=lld ..\code\win32_handmade.cpp -o win32_handmade.exe -lgdi32
popd