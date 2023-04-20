@echo off
cls
echo.
cmake -G "Visual Studio 17 2022" -A x64 -B _build_vs17_dbg . -DCMAKE_BUILD_TYPE=Debug && ^
cmake --build _build_vs17_dbg
