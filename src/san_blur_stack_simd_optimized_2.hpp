#pragma once

namespace san::blur::stack::simd {

template <typename CalcT>
class optimized_2 {
	int			m_radius;
	int			m_div;
	uint16_t	m_mul;
	uint8_t		m_shr;

	//  p_line - points to begin of row or column
	// advance - also '1' for rows or 'stride' for columns
	void do_line( uint32_t * __restrict p_line, int len, int advance ) {

		uint32_t * p_stack = (uint32_t *)SAN_STACK_ALLOC( sizeof( uint32_t ) * m_div );

		// Accum. left part of stack (border color)...
		uint32_t * p_stk = p_stack;
		CalcT sum, sum_out;
		{
			uint32_t c = *p_line;
			CalcT v( c );
			for ( int i = 0; i <= m_radius; i++ ) *p_stk++ = c;
			int n = m_radius + 1;
			sum = v * ((n * (n + 1)) >> 1); // sum = 1v + 2v + 3v + ... + Nv, where N = m_radius + 1
			sum_out = v * n;
		}

		// Accum. right part of stack...
		CalcT sum_in;
		{
			uint32_t * p_src = p_line;
			int j = m_radius;
			for ( int i = 1; i <= m_radius; i++, j-- ) {
				if ( SAN_LIKELY( i < len ) ) p_src += advance;
				uint32_t c = *p_src;
				*p_stk++ = c;
				CalcT v( c );
				sum    += v * j;
				sum_in += v;
			}

			// At now, p_src == p_line + (m_radius * advance);
			assert( p_src == p_line + (m_radius * advance) );
		}


		int i_stack = m_radius;
		uint32_t * p_src = p_line + advance * (m_radius + 1);
		uint32_t * p_dst = p_line;

		// TODO: handle that case
		assert( len >= m_radius + 1 );
		//int l = len - (m_radius + 1);
		len -= m_radius + 1;

		while ( len-- > 0 ) {
			*p_dst = sum * int(m_mul) >> m_shr;	// Stupid MSC compiler with C2666
			sum -= sum_out;

			int stack_start = i_stack + m_div - m_radius;
			if ( stack_start >= m_div ) stack_start -= m_div;

			sum_out -= p_stack[stack_start];

			uint32_t c = *p_src;
			p_stack[stack_start] = c;
			sum_in += c;
			sum    += sum_in;

			if ( ++i_stack >= m_div ) i_stack = 0;

			CalcT v = p_stack[i_stack];
			sum_out += v;
			sum_in  -= v;

			p_src += advance;
			p_dst += advance;
		}

		// At now, p_src == p_line + (len * advance);
		//assert( p_src == p_line + (len * advance) );

		p_src -= advance;
		uint32_t border_c = *p_src;
		CalcT border_v( border_c );

		for ( len = m_radius; len >= 0; len-- ) {
			*p_dst = sum * int(m_mul) >> m_shr;
			sum -= sum_out;

			int stack_start = i_stack + m_div - m_radius;
			if ( stack_start >= m_div ) stack_start -= m_div;

			sum_out -= p_stack[stack_start];

			p_stack[stack_start] = border_c;
			sum_in += border_v;
			sum    += sum_in;

			if ( ++i_stack >= m_div ) i_stack = 0;

			CalcT c = p_stack[i_stack];
			sum_out += c;
			sum_in  -= c;

			//p_src += advance;
			p_dst += advance;
		}
	}

public:
	template <typename ImageViewT, typename ParallelForT>
	void blur( ImageViewT & image, ParallelForT & parallel_for, int radius, int override_num_threads ) {
		if ( radius < 1 ) return;
		if ( radius > 254 ) radius = 254;

		m_radius = radius;
		m_div = radius * 2 + 1;
		m_mul = ::san::blur::stack::lut_mul[radius];
		m_shr = ::san::blur::stack::lut_shr[radius];

		// Horizontal pass...
		parallel_for.run_and_wait( 0, image.height(), [&]( int a, int b ) {
			for ( int y = a; y < b; y++ ) {
				do_line( (uint32_t *)image.row_ptr( y ), image.width(), 1 );
			}
		}, override_num_threads );

		// Vertical pass...
		parallel_for.run_and_wait( 0, image.width(), [&]( int a, int b ) {
			for ( int x = a; x < b; x++ ) {
				do_line( (uint32_t *)image.col_ptr( x ), image.height(), image.stride() / image.components() );
			}
		}, override_num_threads );
	}
}; // class optimized_2

} // namespace san::blur::stack::simd
