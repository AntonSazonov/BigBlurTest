#pragma once

namespace san::stack_blur::simd {

// Horizontal or vertical line adaptor
class line_adaptor {
	uint32_t *	m_ptr;
	int			m_len;
	int			m_advance;

public:
	line_adaptor( uint32_t * p, int len, int advance ) :
		m_ptr( p ), m_len( len ), m_advance( advance ) {}

	uint32_t get_pix( int i ) const {
		if ( i < 0 ) i = 0; else if ( i >= m_len ) i = m_len - 1;
		return m_ptr[i * m_advance];
	}

	void set_pix( int i, uint32_t value ) {
		m_ptr[i * m_advance] = value;
	}
}; // struct line_adaptor


template <typename SIMDCalcT>
void line_process( line_adaptor & line, int head, int tail/*exclusive*/, int radius ) {
	int div = radius * 2 + 1;
	uint32_t * p_stack = (uint32_t *)__builtin_alloca_with_align( sizeof( uint32_t ) * div, 128 );

	// Precalculated divisor. Uses multiplication and right shift under the hood.
	san::stack_blur::simd::calculator::divisor divisor( radius * (radius + 2) + 1 );

	// Fill initial stack...
	SIMDCalcT sum, sum_in, sum_out;
	for ( int i = -radius; i <= radius; i++ ) {
		uint32_t c = line.get_pix( head + i );
		p_stack[i + radius] = c;
		sum += SIMDCalcT( c ) * (radius - std::abs( i ) + 1);
		if ( i <= 0 ) {
			sum_out += c;
		} else {
			sum_in  += c;
		}
	}

	int i_stack = radius;
	for ( int i = head; i < tail; i++ ) {
		line.set_pix( i, uint32_t( sum / divisor ) );

		sum -= sum_out;

		int stack_start = i_stack + div - radius;
		if ( stack_start >= div ) stack_start -= div;

		sum_out -= p_stack[stack_start];

		uint32_t c = line.get_pix( i + radius + 1 );

		p_stack[stack_start] = c;
		sum_in += c;
		sum    += sum_in;

		i_stack++;
		if ( i_stack >= div ) i_stack = 0;

		c = p_stack[i_stack];
		sum_out += c;
		sum_in  -= c;
	}
}

template <typename SIMDCalcT>
void blur( san::image_view & image, int radius ) {
	if ( radius <= 0 ) return;

	// Horizontal pass...
	for ( int y = 0; y < image.height(); y++ ) {
		line_adaptor line( (uint32_t *)image.row_ptr( y ), image.width(), 1/*advance*/ );
		line_process<SIMDCalcT>( line, 0, image.width(), radius );
	}

	// Vertical pass...
	for ( int x = 0; x < image.width(); x++ ) {
		line_adaptor line( (uint32_t *)image.col_ptr( x ), image.height(), image.stride() / 4/*sizeof uint32_t*/ );
		line_process<SIMDCalcT>( line, 0, image.height(), radius );
	}
}

template <typename SIMDCalcT, typename ParallelFor>
void blur( san::image_view & image, int radius, ParallelFor & parallel_for, int override_num_threads = 0 ) {
	if ( radius <= 0 ) return;

	// Horizontal pass...
	parallel_for.run_and_wait( 0, image.height(), [&]( int a, int b ) {
		for ( int y = a; y < b; y++ ) {
			line_adaptor line( (uint32_t *)image.row_ptr( y ), image.width(), 1/*advance*/ );
			line_process<SIMDCalcT>( line, 0, image.width(), radius );
		}
	}, override_num_threads );

	// Vertical pass...
	parallel_for.run_and_wait( 0, image.width(), [&]( int a, int b ) {
		for ( int x = a; x < b; x++ ) {
			line_adaptor line( (uint32_t *)image.col_ptr( x ), image.height(), image.stride() / 4/*sizeof uint32_t*/ );
			line_process<SIMDCalcT>( line, 0, image.height(), radius );
		}
	}, override_num_threads );
}

} // namespace san::stack_blur::simd
