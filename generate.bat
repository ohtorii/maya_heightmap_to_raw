@echo off
setlocal

set DEVKIT_LOCATION=D:\Users\ikeuc_000\Documents\MayaDevkitBase2019
cmake -H. -Bbuild2019 -G "Visual Studio 14 2015 Win64"

rem cmake --build build --config Release
