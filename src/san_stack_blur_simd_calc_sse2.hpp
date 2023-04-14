#pragma once

#ifndef __SSE2__
 #warning "SSE2 not supported or not enabled by compiler."
#else

#include <emmintrin.h>

namespace san::stack_blur::simd::calculator {

class sse2 {
	__m128i m;

	static_assert( alignof( __m128i ) >= 16 );

public:
	sse2() : m( _mm_setzero_si128() ) {}

	sse2( const __m128i & v ) : m( v ) {}

	sse2( uint32_t v ) {
		__m128i zero = _mm_setzero_si128();
		m = _mm_unpacklo_epi16( _mm_unpacklo_epi8( _mm_cvtsi32_si128( v ), zero ), zero );
	}

	operator __m128i () const { return m; }

	operator uint32_t () const {
		__m128i zero = _mm_setzero_si128();
		return _mm_cvtsi128_si32(  _mm_packs_epi16(  _mm_packs_epi32( m, zero ), zero ) );	// signed saturation
	}

	sse2 & operator += ( const sse2 & rhs ) {
		m = _mm_add_epi32( m, rhs );
		return *this;
	}

	sse2 & operator -= ( const sse2 & rhs ) {
		m = _mm_sub_epi32( m, rhs );
		return *this;
	}

	sse2 operator * ( int value ) const {
		__m128i v = _mm_set1_epi32( value );
		__m128i x = _mm_mul_epu32( m, v ); /* mul 2, 0*/
		__m128i y = _mm_mul_epu32(
			_mm_srli_si128( m, 4 ),
			_mm_srli_si128( v, 4 ) ); /* mul 3, 1 */
		return _mm_unpacklo_epi32(
					_mm_shuffle_epi32( x, _MM_SHUFFLE( 0, 0, 2, 0 ) ),
					_mm_shuffle_epi32( y, _MM_SHUFFLE( 0, 0, 2, 0 ) ) ); /* shuffle results to [63..0] and pack */
	}

	sse2 operator / ( const divisor & d ) const {
		__m128i t1 = _mm_mul_epu32( m, d.multiplier() );	// 32x32->64 bit unsigned multiplication of a[0] and a[2]
		__m128i t2 = _mm_srli_epi64( t1, 32 );				// high dword of result 0 and 2
		__m128i t3 = _mm_srli_epi64( m, 32 );				// get a[1] and a[3] into position for multiplication
		__m128i t4 = _mm_mul_epu32( t3, d.multiplier() );	// 32x32->64 bit unsigned multiplication of a[1] and a[3]

		__m128i t5 = _mm_set_epi32( -1, 0, -1, 0 );			// mask of dword 1 and 3
		__m128i t6 = _mm_and_si128( t4, t5 );				// high dword of result 1 and 3
		__m128i t7 = _mm_or_si128( t2, t6 );				// combine all four results into one vector

		__m128i t8 = _mm_sub_epi32( m, t7 );				// subtract
		__m128i t9 = _mm_srl_epi32( t8, d.shift1() );		// shift right logical
		__m128i t10 = _mm_add_epi32( t7, t9 );				// add
		return _mm_srl_epi32( t10, d.shift2() );			// shift right logical
	}
}; // class sse2

} // namespace san::stack_blur::simd::calculator

#endif // __SSE2__
