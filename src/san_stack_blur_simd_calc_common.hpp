#pragma once

namespace san::stack_blur::calculator {

// https://github.com/vectorclass/version2/blob/master/instrset.h
#if (defined (__GNUC__) || defined(__clang__)) && !defined (_MSC_VER)
static inline uint32_t bit_scan_reverse( uint32_t a ) {
	uint32_t r;
	__asm( "bsrl %1, %0" : "=r"(r) : "r"(a) : );
	return r;
}
#else
static inline uint32_t bit_scan_reverse( uint32_t a ) {
	uint32_t r;
	_BitScanReverse( &r, a );
	return r;
}
#endif // (defined (__GNUC__) || defined(__clang__)) && !defined (_MSC_VER)

// https://github.com/vectorclass/version2/blob/master/vectori128.h
class divisor {
	__m128i m_multiplier;
	__m128i m_shift1;
	__m128i m_shift2;

public:
	divisor( uint32_t d ) {
		uint32_t L = bit_scan_reverse( d - 1 ) + 1;				// ceil(log2(d))
		uint32_t L2 = uint32_t(L < 32 ? 1 << L : 0);			// 2^L, overflow to 0 if L = 32
		uint32_t m = 1 + uint32_t((uint64_t(L2 - d) << 32) / d);// multiplier
		uint32_t sh1 = 1;
		uint32_t sh2 = L - 1;									// shift counts

		m_multiplier = _mm_set1_epi32( m );
		m_shift1 = _mm_setr_epi32( sh1, 0, 0, 0 );
		m_shift2 = _mm_setr_epi32( sh2, 0, 0, 0 );
	}

	const __m128i & multiplier() const { return m_multiplier; }
	const __m128i & shift1() const { return m_shift1; }
	const __m128i & shift2() const { return m_shift2; }
}; // class divisor

} // namespace san::stack_blur::calculator
