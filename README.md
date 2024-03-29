# Blur Algorithms and Implementations Test in C++
[![GitHub code size](https://img.shields.io/github/languages/code-size/AntonSazonov/Blur_Test?style=flat)](https://github.com/AntonSazonov/Blur_Test)

Tested algorithms and implementations:
 * **Stack Blur** <sub>(Anti-Grain Geometry 2.5 by Maxim Shemanarev)</sub>
 * **Recursive Blur** <sub>(Anti-Grain Geometry 2.5 by Maxim Shemanarev)</sub>
 * **My unoptimized implementation of Stack Blur**
 * **My optimized implementations of Stack Blur using SSE2, SSSE3, SSE4.1**

*Note: AGG versions was slightly modified to be able to use them with multiple threads and to suppress some compile warnings.*

The Stack Blur algorithm was invented by Mario Klingemann.  
mario@quasimondo.com  
https://medium.com/@quasimondo  

AGG - Anti-Grain Geometry - a library that was written by Maxim Shemanarev.

Do not use SIMD versions with disabled compiler optimizations. They'll be too slow.  
Use at least '-O1' optimization level (GCC and Clang).  

All tested versions use 32-bit (8 bits per component, order of components is not important) pixel format.

Clang do much better optimizations with same flags than GCC. Both tested are from MSYS2/MinGW64 toolchain.  

The fastest implementation I could write is about 0.7ms for a 1280x720 32bpp frame on an AMD Ryzen 7 2700 with SSE4.1 and 16 threads.  
<br/><br/>
## Parallel 'for' loop range distribution

 Suppose, we have loop:
```C++
for ( int i = 0; i < 8; i++ ) ...
```
And we want to break him into 3 threads.  
Thus, 8 iterations / 3 threads = 3 threads with 2 iterations + 2 remained iterations.  

### Old algorithm (remainder is added to the last thread):
 * Thread #1: [0;2)  - size 2 
 * Thread #2: [2;4)  - size 2
 * Thread #3: [4;8)  - size 4

 This approach has two disadvantages:
  1. Non-uniform range distribution
  2. The last thread have biggest block size and (oftenly, not always) begins its execution after all previous threads already has started

### New algorithm (remainder is distributed over first threads):
 * Thread #1: [0;3)  - size 3
 * Thread #2: [3;6)  - size 3
 * Thread #3: [6;8)  - size 2

<br/><br/>
TODO:
 * Recursive Blur SIMD version
 * Gaussian Blur SIMD version
<br/><br/>

Example on Youtube (need to be updated):
[![Watch the video](https://github.com/AntonSazonov/Blur_Test/blob/main/screenshot.jpg)](https://youtu.be/xsU6lKb5LRA)

