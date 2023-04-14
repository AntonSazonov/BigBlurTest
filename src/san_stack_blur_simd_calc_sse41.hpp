#pragma once

#ifndef __SSE4_1__
 #warning "SSE4.1 not supported or not enabled by compiler."
#else

#include <smmintrin.h>

namespace san::stack_blur::simd::calculator {

class sse41 {
	__m128i m;

	static_assert( alignof( __m128i ) >= 16 );

public:
	sse41() : m( _mm_setzero_si128() ) {}

	sse41( const __m128i & v ) : m( v ) {}

	sse41( uint32_t v ) {
		m = _mm_cvtepu8_epi32( _mm_cvtsi32_si128( v ) ); // _mm_cvtepu8_epi32 - SSE4.1
	}

	operator __m128i () const { return m; }

	operator uint32_t () const {
		__m128i zero = _mm_setzero_si128();
		return _mm_cvtsi128_si32( _mm_packus_epi16( _mm_packus_epi32( m, zero ), zero ) ); // unsigned saturation (_mm_packus_epi32 - SSE4.1)
	}

	sse41 & operator += ( const sse41 & rhs ) {
		m = _mm_add_epi32( m, rhs );
		return *this;
	}

	sse41 & operator -= ( const sse41 & rhs ) {
		m = _mm_sub_epi32( m, rhs );
		return *this;
	}

	sse41 operator * ( int value ) const {
		return _mm_mullo_epi32( m, _mm_set1_epi32( value ) ); // SSE4.1
	}

	sse41 operator / ( const divisor & d ) const {
		__m128i t1 = _mm_mul_epu32( m, d.multiplier() );	// 32x32->64 bit unsigned multiplication of a[0] and a[2]
		__m128i t2 = _mm_srli_epi64( t1, 32 );				// high dword of result 0 and 2
		__m128i t3 = _mm_srli_epi64( m, 32 );				// get a[1] and a[3] into position for multiplication
		__m128i t4 = _mm_mul_epu32( t3, d.multiplier() );	// 32x32->64 bit unsigned multiplication of a[1] and a[3]

		__m128i t7 = _mm_blend_epi16( t2, t4, 0xcc );		// blend two results (SSE4.1)

		__m128i t8 = _mm_sub_epi32( m, t7 );				// subtract
		__m128i t9 = _mm_srl_epi32( t8, d.shift1() );		// shift right logical
		__m128i t10 = _mm_add_epi32( t7, t9 );				// add
		return _mm_srl_epi32( t10, d.shift2() );			// shift right logical
	}
}; // class sse41

} // namespace san::stack_blur::simd::calculator

#endif
