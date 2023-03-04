@echo off

mkdir ..\build
pushd ..\build
g++ -g -municode -fuse-ld=lld ..\code\win32_handmade.cpp -o win32_handmade.exe
popd