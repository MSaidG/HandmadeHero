@echo off

mkdir ..\build
pushd ..\build
g++ -g -fuse-ld=lld ..\code\win32_handmade.cpp -o win32_handmade.exe -lgdi32
popd