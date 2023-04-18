# Blur Algorithms and Implementations Test in C++
![CMake workflow](https://github.com/AntonSazonov/Blur_Test/actions/workflows/cmake.yml/badge.svg?branch=main)
[![GitHub code size](https://img.shields.io/github/languages/code-size/AntonSazonov/Blur_Test?style=flat)](https://github.com/AntonSazonov/Blur_Test)

Tested algorithms and implementations:
 * **Stack Blur** <sub>(Anti-Grain Geometry 2.5 by Maxim Shemanarev)</sub>
 * **Recursive Blur** <sub>(Anti-Grain Geometry 2.5 by Maxim Shemanarev)</sub>
 * **My unoptimized implementation of Stack Blur**
 * **My optimized implementations of Stack Blur using SSE2, SSSE3, SSE4.1**

*Note: AGG versions was slightly modified to be able to use them with multiple threads.*

The Stack Blur algorithm was invented by Mario Klingemann.  
mario@quasimondo.com  
https://medium.com/@quasimondo  

AGG - Anti-Grain Geometry - a library that was written by Maxim Shemanarev.

Do not use SIMD versions with disabled compiler optimizations. They'll be too slow.  
Use at least '-O1' optimization level (GCC and Clang option).  

All tested versions use 32-bit (8 bits per component, order of components is not impotant) pixel format.

Clang do much better optimizations with same flags than GCC. Both tested are from MSYS2/MinGW64 toolchain.  

The fastest implementation I could write is about 1.6ms for a 1280x720 32bpp frame on an AMD Ryzen 7 2700 with 16 threads.  
Note, that part of time is spended to SDL's event handling, blitting etc. So actual speed of some implementations (especially multithreaded) is significantly higher, and, thats why your CPU load shows you 50-60% load with using all cores - SDL event loop runs in one thread.
<br/><br/>
## Parallel 'for' loop range distribution

 Suppose, we have loop:
```C++
for ( int i = 0; i < 8; i++ ) ...
```
And we want to break him into 3 threads.  
Thus, 8 iterations / 3 threads = 3 threads with 2 iterations + 2 remained iterations.  

### Old algorithm (remainder is added to last thread):
 * Thread #1: [0;2)  - size 2 
 * Thread #2: [2;4)  - size 2
 * Thread #3: [4;8)  - size 4

 This approach has two disadvantages:
  1. Non-uniform range distribution
  2. The last thread have biggest block size and begins its execution after all previous threads already has started. This is sad.

### New algorithm (remainder is distributed over first threads):
 * Thread #1: [0;3)  - size 3
 * Thread #2: [3;6)  - size 3
 * Thread #3: [6;8)  - size 2

<br/><br/>
TODO:
 * Recursive Blur SIMD version
 * Gaussian Blur
 * Get rid of SDL2?
 * Run tests in separate thread
<br/><br/>

Example on Youtube (need to be updated):
[![Watch the video](https://github.com/AntonSazonov/Blur_Test/blob/main/screenshot.png)](https://youtu.be/xsU6lKb5LRA)

