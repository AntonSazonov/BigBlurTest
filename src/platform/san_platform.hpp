#pragma once

// Arch.
#if !defined( _M_X64 ) && !defined( __x86_64__ )
 #error "Only x86_64 architecture is supported."
#endif

// Compiler
#if defined( __INTEL_COMPILER )
 #define SAN_COMPILER_INTEL

#elif defined( _MSC_VER ) && defined( _MSC_FULL_VER )
 #define SAN_COMPILER_MSC

#elif defined( __clang_major__ ) && defined( __clang_minor__ ) && defined( __clang_patchlevel__ )
 #define SAN_COMPILER_CLANG

#elif defined( __GNUC__ ) && defined( __GNUC_MINOR__ ) && defined( __GNUC_PATCHLEVEL__ )
 #define SAN_COMPILER_GNU

#else
 #error "Unknown or unsupported compiler."
#endif

// Platform
#if defined( _WIN32 ) || defined( WIN32 )
 #define SAN_PLATFORM_WINDOWS

 #ifndef WIN32_LEAN_AND_MEAN
   #define WIN32_LEAN_AND_MEAN
 #endif
 #ifndef NOMINMAX
   #define NOMINMAX
 #endif
 #include <windows.h>

#elif defined( __linux__ )
 #define SAN_PLATFORM_LINUX

#else
 #error "Unknown or unsupported platform."
#endif

// ------------------------------------------

#if defined( SAN_COMPILER_MSC )
 #ifndef _CRT_SECURE_NO_DEPRECATE
   #define _CRT_SECURE_NO_DEPRECATE
 #endif
 #ifndef _CRT_SECURE_NO_WARNINGS
   #define _CRT_SECURE_NO_WARNINGS
 #endif
#endif

#if defined( SAN_COMPILER_GNU )
 #define SAN_LIKELY( ... )		__builtin_expect( !!(__VA_ARGS__), 1 )
 #define SAN_UNLIKELY( ... )	__builtin_expect( !!(__VA_ARGS__), 0 )
#else
 #define SAN_LIKELY( ... )		( __VA_ARGS__ )
 #define SAN_UNLIKELY( ... )	( __VA_ARGS__ )
#endif


#if defined( SAN_COMPILER_MSC )

 // The allocated space is automatically freed when the calling function exits (not when the allocation merely passes out of scope).
 #define SAN_STACK_ALLOC( size )	_alloca( size )


#elif defined( SAN_COMPILER_GNU ) || defined( SAN_COMPILER_CLANG )

 // The lifetime of the allocated object ends just before the calling function returns to its caller. This is so even when __builtin_alloca is called within a nested block.
 #define SAN_STACK_ALLOC( size )	__builtin_alloca( size )

#else
 #error "TODO"
#endif
