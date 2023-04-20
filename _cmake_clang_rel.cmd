@echo off
cls
echo.
cmake -G Ninja -B _build_clang_rel . -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release && ^
cmake --build _build_clang_rel
if %errorlevel% NEQ 0 ( del /f BigBlurTest-Clang-Release.exe 2> nul )
