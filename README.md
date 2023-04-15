# Blur Algorithms and Implementations Test in C++
![CMake workflow](https://github.com/AntonSazonov/Blur_Test/actions/workflows/cmake.yml/badge.svg?branch=main)
[![GitHub code size](https://img.shields.io/github/languages/code-size/AntonSazonov/Blur_Test?style=flat)](https://github.com/AntonSazonov/Blur_Test)

Tested algorithms and implementations:
 * **Stack Blur** <sub>(Anti-Grain Geometry 2.5 by Maxim Shemanarev)</sub>
 * **Recursive Blur** <sub>(Anti-Grain Geometry 2.5 by Maxim Shemanarev)</sub>
 * **My unoptimized implementation of Stack Blur**
 * **My optimized implementations of Stack Blur using SSE2, SSSE3, SSE4.1 and multithreading**

The Stack Blur algorithm was invented by Mario Klingemann.  
mario@quasimondo.com  
https://medium.com/@quasimondo  

Do not use SIMD versions with disabled compiler optimizations. They'll be too slow.  
Use at least -O1 optimization level.  

All tested versions use 32-bit pixel format.

Clang do much better optimizations with same flags than GCC. Both tested are from MSYS2/MinGW64 toolchain.  

The fastest implementation I could write is about 1.6ms for a 1280x720 32bpp frame on an AMD Ryzen 7 2700 with 16 threads.  
Note, that part of time is spended to SDL's event handling, blitting etc. So actual speed of some implementations (especially multithreaded) is significantly higher. And, thats why your CPU load shows you 50-60% load with using all cores - SDL runs in one thread.

TODO:
 * Recursive Blur SIMD version
 * Gaussian Blur
 * Get rid of SDL2?

Example on Youtube (need to be updated):
[![Watch the video](https://github.com/AntonSazonov/Blur_Test/blob/main/screenshot.png)](https://youtu.be/xsU6lKb5LRA)

