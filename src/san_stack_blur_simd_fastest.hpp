#pragma once

#ifndef __SSE2__
 #error "At least SSE2 support must be enabled."
#endif

#include <emmintrin.h> // __SSE2__

#ifdef __SSE4_1__
 #include <smmintrin.h>
#endif

namespace san::stack_blur::simd::fastest {

namespace internal {

const uint16_t lut_mul[255] = {
	512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512, 454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
	482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456, 437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
	497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328, 320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
	446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335, 329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
	505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405, 399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
	324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271, 268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
	451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388, 385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
	332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292, 289,287,285,282,280,278,275,273,271,269,267,265,263,261,259 };

const uint8_t lut_shr[255] = {
	 9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24 };

} // namespace internal


class sse128_t {
	__m128i m_vec;

public:
	sse128_t() : m_vec( _mm_setzero_si128() ) {}

	sse128_t( const __m128i & v ) : m_vec( v ) {}

	sse128_t( uint32_t v ) {
#ifdef __SSE4_1__
		m_vec = _mm_cvtepu8_epi32( _mm_cvtsi32_si128( v ) ); // _mm_cvtepu8_epi32 - SSE4.1
#else
		__m128i zero = _mm_setzero_si128();
		m_vec = _mm_unpacklo_epi16( _mm_unpacklo_epi8( _mm_cvtsi32_si128( v ), zero ), zero );
#endif
	}

	operator __m128i () const { return m_vec; }

	operator uint32_t () const {
#ifdef __SSSE3__
		// Seems to be faster...
		__m128i s = _mm_setr_epi8( 0,  4,  8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
		return _mm_cvtsi128_si32( _mm_shuffle_epi8( m_vec, s ) ); // SSSE3
#else
		__m128i zero = _mm_setzero_si128();
#ifdef __SSE4_1__
#warning "Using SSE4.1"
		return _mm_cvtsi128_si32( _mm_packus_epi16( _mm_packus_epi32( m_vec, zero ), zero ) ); // unsigned saturation (_mm_packus_epi32 - SSE4.1)
#else
#warning "Using SSE2"
		return _mm_cvtsi128_si32(  _mm_packs_epi16(  _mm_packs_epi32( m_vec, zero ), zero ) ); // signed saturation
#endif
#endif
	}

	sse128_t & operator += ( const sse128_t & rhs ) {
		m_vec = _mm_add_epi32( m_vec, rhs );
		return *this;
	}

	sse128_t & operator -= ( const sse128_t & rhs ) {
		m_vec = _mm_sub_epi32( m_vec, rhs );
		return *this;
	}

	sse128_t operator * ( int value ) const {
#ifdef __SSE4_1__
		return _mm_mullo_epi32( m_vec, _mm_set1_epi32( value ) ); // SSE4.1
#else
		__m128i v = _mm_set1_epi32( value );
		__m128i x = _mm_mul_epu32( m_vec, v ); /* mul 2, 0*/
		__m128i y = _mm_mul_epu32(
			_mm_srli_si128( m_vec, 4 ),
			_mm_srli_si128( v, 4 ) ); /* mul 3, 1 */
		return _mm_unpacklo_epi32(
					_mm_shuffle_epi32( x, _MM_SHUFFLE( 0, 0, 2, 0 ) ),
					_mm_shuffle_epi32( y, _MM_SHUFFLE( 0, 0, 2, 0 ) ) ); /* shuffle results to [63..0] and pack */
#endif
	}

	sse128_t operator >> ( uint8_t shift ) const {
		return _mm_srli_epi32( m_vec, shift );	// SSE2
	}
}; // class sse128_t


class blur_impl {
	int			m_radius;
	int			m_div;
	uint16_t	m_mul;
	uint8_t		m_shr;

	//  p_line - points to begin of row or column
	// advance - also '1' for rows or 'stride' for columns
	void line_process( uint32_t * p_line, int len, int advance ) {

		uint32_t * p_stack = (uint32_t *)__builtin_alloca_with_align( sizeof( uint32_t ) * m_div, 128 );

		// Accum. left part of stack (border color)...
		uint32_t * p_stk = p_stack;
		sse128_t sum, sum_out;
		{
			uint32_t c = *p_line;
			sse128_t v( c );
			for ( int i = 0; i <= m_radius; i++ ) {
				*p_stk++ = c;
				sum     += v * (i + 1);
				sum_out += v;
			}
		}

		// Accum. right part of stack...
		sse128_t sum_in;
		{
			uint32_t * p_src = p_line;
			int j = m_radius;
			for ( int i = 1; i <= m_radius; i++, j-- ) {
				if ( __builtin_expect( i < len, 1 ) ) p_src += advance;
				uint32_t c = *p_src;
				*p_stk++ = c;
				sse128_t v( c );
				sum    += v * j;
				sum_in += v;
			}
		}


		int i_stack = m_radius;
		uint32_t * p_src = p_line + advance * (m_radius + 1);
		uint32_t * p_end = p_line + advance * (len - 1);
		uint32_t * p_dst = p_line;

		for ( ; len-- > 0; p_dst += advance ) {
			*p_dst = sum * m_mul >> m_shr;
			sum -= sum_out;

			int stack_start = i_stack + m_div - m_radius;
			if ( stack_start >= m_div ) stack_start -= m_div;

			sum_out -= p_stack[stack_start];

			uint32_t c;
			if ( __builtin_expect( p_src <= p_end, 1 ) ) {
				c = *p_src;
				p_src += advance;
			} else {
				c = *p_end;
			}

			p_stack[stack_start] = c;
			sum_in += c;
			sum    += sum_in;

			if ( ++i_stack >= m_div ) i_stack = 0;

			c = p_stack[i_stack];
			sum_out += c;
			sum_in  -= c;
		}
	}

public:
	template <typename ImageViewT, typename ParallelFor>
	void blur( ImageViewT & image, ParallelFor & parallel_for, int radius, int override_num_threads ) {
		if ( radius <= 0 ) return;
		if ( radius > 254 ) radius = 254;

		m_radius = radius;
		m_div = radius * 2 + 1;
		m_mul = internal::lut_mul[radius];
		m_shr = internal::lut_shr[radius];

		parallel_for.run_and_wait( 0, image.height(), [&]( int a, int b ) {
			for ( int y = a; y < b; y++ ) {
				line_process( (uint32_t *)image.row_ptr( y ), image.width(), 1 );
			}
		}, override_num_threads );

		parallel_for.run_and_wait( 0, image.width(), [&]( int a, int b ) {
			for ( int x = a; x < b; x++ ) {
				line_process( (uint32_t *)image.col_ptr( x ), image.height(), image.stride() / 4 );
			}
		}, override_num_threads );
	}
}; // class blur_impl

} // namespace san::stack_blur::simd::fastest
