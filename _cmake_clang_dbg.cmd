@echo off
cls
echo.
cmake -G Ninja -B _build_clang_dbg . -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DDEBUG_OPT_LEVEL="-O1" && ^
cmake --build _build_clang_dbg
if %errorlevel% NEQ 0 ( del /f main-Clang-Debug.exe 2> nul )
