# Blur algorithms and implementations test in C++

 The Stack Blur algorithm was invented by Mario Klingemann.  
 mario@quasimondo.com  
 https://medium.com/@quasimondo  

 SSE2 versions are far from ideal (especially division operation), but shows nice results even in naive implementation.
 Do not use SIMD versions with disabled compiler optimizations. They'll be too slow. Use at least -O1 optimization level.

 All tested versions use 32-bit pixel format.

 Clang do much better optimizations with same flags than GCC. Both from MSYS2/MinGW64 toolchain.

Example on Youtube:
[![Watch the video](https://github.com/AntonSazonov/Blur_Test/blob/main/screenshot.png)](https://youtu.be/xsU6lKb5LRA)
