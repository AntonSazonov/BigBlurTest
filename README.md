# Blur algorithms and implementations test in C++

Tested algorithms and implementations:
 * Stack Blur (from Anti-Grain Geometry 2.5 by Maxim Shemanarev).
 * Recursive Blur (from Anti-Grain Geometry 2.5 by Maxim Shemanarev).
 * My unoptimized implementation of Stack Blur.
 * My optimized implementations of Stack Blur using SSE2, SSE3, SSE4.1 and multithreading.

The Stack Blur algorithm was invented by Mario Klingemann.  
mario@quasimondo.com  
https://medium.com/@quasimondo  

Do not use SIMD versions with disabled compiler optimizations. They'll be too slow. Use at least -O1 optimization level.  
All tested versions use 32-bit pixel format.  
Clang do much better optimizations with same flags than GCC. Both from MSYS2/MinGW64 toolchain.  

The fastest implementation I could write is about 1.6ms for a 1280x720 32bpp frame on an AMD Ryzen 7 2700 with 16 threads.  
Note, that some part of time is spended to SDL's event handling, blitting etc. So actual speed of some implementations (especially multithreaded) is significantly higher.  

TODO:
 * Recursive Blur SIMD version 

Example on Youtube (need to be updated):
[![Watch the video](https://github.com/AntonSazonov/Blur_Test/blob/main/screenshot.png)](https://youtu.be/xsU6lKb5LRA)
