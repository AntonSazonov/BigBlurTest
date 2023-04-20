@echo off
cls
echo.
cmake -G Ninja -B _build_gcc_dbg . -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug && ^
cmake --build _build_gcc_dbg
if %errorlevel% NEQ 0 ( del /f BigBlurTest-GNU-Debug.exe 2> nul )
