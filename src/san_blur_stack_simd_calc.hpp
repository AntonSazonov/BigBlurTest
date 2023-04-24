#pragma once

namespace san::blur::stack::simd {

// https://github.com/vectorclass/version2/blob/master/instrset.h
#if (defined( __GNUC__ ) || defined( __clang__ )) && !defined( _MSC_VER )
static inline uint32_t bit_scan_reverse( uint32_t a ) {
	uint32_t r;
	__asm( "bsrl %1, %0" : "=r"(r) : "r"(a) : );
	return r;
}
#else
static inline uint32_t bit_scan_reverse( unsigned long a ) {
	unsigned long r;
	_BitScanReverse( (unsigned long *)&r, a );
	return r;
}
#endif // (defined( __GNUC__ ) || defined( __clang__ )) && !defined( _MSC_VER )

// https://github.com/vectorclass/version2/blob/master/vectori128.h
class divisor {
	__m128i m_multiplier;
	__m128i m_shift1;
	__m128i m_shift2;

public:
	divisor( uint32_t d ) {
		uint32_t L = bit_scan_reverse( d - 1 ) + 1;		// ceil(log2(d))
		uint32_t L2 = L < 32 ? 1 << L : 0;				// 2^L, overflow to 0 if L = 32
		m_multiplier = _mm_set1_epi32( (uint64_t(L2 - d) << 32) / d + 1 );
		m_shift1 = _mm_setr_epi32(     1, 0, 0, 0 );
		m_shift2 = _mm_setr_epi32( L - 1, 0, 0, 0 );
	}

	const __m128i & multiplier() const { return m_multiplier; }
	const __m128i & shift1() const { return m_shift1; }
	const __m128i & shift2() const { return m_shift2; }
}; // class divisor


#ifdef _MSC_VER

 #ifndef _M_X64
  #error "At least SSE2 support must be enabled by compiler."
 #endif

 #define __SSE2__

#else

 #if !defined( __SSE4_1__ ) && !defined( __SSE2__ )
  #error "At least SSE2 support must be enabled by compiler."
 #endif

#endif

// Common for all SSE variants
template <int SSE = 2>
class sse128_u32_t {
	__m128i m_vec;

public:
	sse128_u32_t() : m_vec( _mm_setzero_si128() ) {}

	sse128_u32_t( const __m128i & v ) : m_vec( v ) {}

	sse128_u32_t( uint32_t v ) {
		if constexpr ( SSE == 41 ) {
			m_vec = _mm_cvtepu8_epi32( _mm_cvtsi32_si128( v ) ); // _mm_cvtepu8_epi32 - SSE4.1
		} else { // SSE2
			__m128i zero = _mm_setzero_si128();
			m_vec = _mm_unpacklo_epi16( _mm_unpacklo_epi8( _mm_cvtsi32_si128( v ), zero ), zero );
		}
	}

	operator __m128i () const { return m_vec; }

	operator uint32_t () const {
		if constexpr ( SSE == 41 ) {

// Prefer SSSE3 over SSE41 here
#if defined( __SSSE3__ )
			// Seems to be faster...
			__m128i s = _mm_setr_epi8( 0,  4,  8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
			return _mm_cvtsi128_si32( _mm_shuffle_epi8( m_vec, s ) ); // SSSE3
#else // SSE41
			__m128i zero = _mm_setzero_si128();
			return _mm_cvtsi128_si32( _mm_packus_epi16( _mm_packus_epi32( m_vec, zero ), zero ) ); // unsigned saturation (_mm_packus_epi32 - SSE4.1)
#endif
		} else {
			// SSE2
			__m128i zero = _mm_setzero_si128();
			return _mm_cvtsi128_si32(  _mm_packus_epi16(  _mm_packs_epi32( m_vec, zero ), zero ) ); // signed saturation
		}
	}

	sse128_u32_t & operator += ( const sse128_u32_t & rhs ) {
		m_vec = _mm_add_epi32( m_vec, rhs );
		return *this;
	}

	sse128_u32_t & operator -= ( const sse128_u32_t & rhs ) {
		m_vec = _mm_sub_epi32( m_vec, rhs );
		return *this;
	}

	sse128_u32_t operator * ( int value ) const {
		if constexpr ( SSE == 41 ) {
			return _mm_mullo_epi32( m_vec, _mm_set1_epi32( value ) ); // SSE4.1
		} else { // SSE2
			__m128i v = _mm_set1_epi32( value );
			__m128i x = _mm_mul_epu32( m_vec, v ); /* mul 2, 0*/
			__m128i y = _mm_mul_epu32(
				_mm_srli_si128( m_vec, 4 ),
				_mm_srli_si128( v, 4 ) ); /* mul 3, 1 */
			return _mm_unpacklo_epi32(
				_mm_shuffle_epi32( x, _MM_SHUFFLE( 0, 0, 2, 0 ) ),
				_mm_shuffle_epi32( y, _MM_SHUFFLE( 0, 0, 2, 0 ) ) ); /* shuffle results to [63..0] and pack */
		}
	}

	sse128_u32_t operator >> ( uint8_t shift ) const {
		return _mm_srli_epi32( m_vec, shift );	// SSE2
	}

	sse128_u32_t operator / ( const divisor & d ) const {
		__m128i t1 = _mm_mul_epu32( m_vec, d.multiplier() );// 32x32->64 bit unsigned multiplication of a[0] and a[2]
		__m128i t2 = _mm_srli_epi64( t1, 32 );				// high dword of result 0 and 2
		__m128i t3 = _mm_srli_epi64( m_vec, 32 );			// get a[1] and a[3] into position for multiplication
		__m128i t4 = _mm_mul_epu32( t3, d.multiplier() );	// 32x32->64 bit unsigned multiplication of a[1] and a[3]

		__m128i t7;
		if constexpr ( SSE == 41 ) {
			t7 = _mm_blend_epi16( t2, t4, 0xcc );			// blend two results (SSE4.1)
		} else { // SSE2
			__m128i t5 = _mm_set_epi32( -1, 0, -1, 0 );		// mask of dword 1 and 3
			__m128i t6 = _mm_and_si128( t4, t5 );			// high dword of result 1 and 3
			t7 = _mm_or_si128( t2, t6 );					// combine all four results into one vector
		}

		__m128i t8 = _mm_sub_epi32( m_vec, t7 );			// subtract
		__m128i t9 = _mm_srl_epi32( t8, d.shift1() );		// shift right logical
		__m128i t10 = _mm_add_epi32( t7, t9 );				// add
		return _mm_srl_epi32( t10, d.shift2() );			// shift right logical
	}
}; // class sse128_u32_t

} // namespace san::blur::stack::simd
