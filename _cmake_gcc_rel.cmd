@echo off
cls
echo.
cmake -G Ninja -B _build_gcc_rel . -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release && ^
cmake --build _build_gcc_rel
if %errorlevel% NEQ 0 ( del /f BigBlurTest-GNU-Release.exe 2> nul )
