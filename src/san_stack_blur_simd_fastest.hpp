#pragma once

namespace san::stack_blur::simd::fastest {

template <typename CalcT>
class blur_impl {
	int			m_radius;
	int			m_div;
	uint16_t	m_mul;
	uint8_t		m_shr;

	//  p_line - points to begin of row or column
	// advance - also '1' for rows or 'stride' for columns
	void line_process( uint32_t * p_line, int len, int advance ) {

		uint32_t * p_stack = (uint32_t *)SAN_STACK_ALLOC( sizeof( uint32_t ) * m_div );

		// Accum. left part of stack (border color)...
		uint32_t * p_stk = p_stack;
		CalcT sum, sum_out;
		{
			uint32_t c = *p_line;
			CalcT v( c );
			for ( int i = 0; i <= m_radius; i++ ) {
				*p_stk++ = c;
				sum     += v * (i + 1);
				sum_out += v;
			}
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
		}


		int i_stack = m_radius;
		uint32_t * p_src = p_line + advance * (m_radius + 1);
		uint32_t * p_end = p_line + advance * (len - 1);
		uint32_t * p_dst = p_line;

		for ( ; len-- > 0; p_dst += advance ) {
			*p_dst = sum * int(m_mul) >> m_shr;
			sum -= sum_out;

			int stack_start = i_stack + m_div - m_radius;
			if ( stack_start >= m_div ) stack_start -= m_div;

			sum_out -= p_stack[stack_start];

			uint32_t c;
			if ( SAN_LIKELY( p_src <= p_end ) ) {
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
		m_mul = ::san::stack_blur::lut_mul[radius];
		m_shr = ::san::stack_blur::lut_shr[radius];

		// Horizontal pass...
		parallel_for.run_and_wait( 0, image.height(), [&]( int a, int b ) {
			for ( int y = a; y < b; y++ ) {
				line_process( (uint32_t *)image.row_ptr( y ), image.width(), 1 );
			}
		}, override_num_threads );

		// Vertical pass...
		parallel_for.run_and_wait( 0, image.width(), [&]( int a, int b ) {
			for ( int x = a; x < b; x++ ) {
				line_process( (uint32_t *)image.col_ptr( x ), image.height(), image.stride() / 4 );
			}
		}, override_num_threads );
	}
}; // class blur_impl

} // namespace san::stack_blur::simd::fastest
