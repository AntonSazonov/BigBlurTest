#pragma once

namespace san::blur::stack {

template <typename NaiveCalcT>
void naive_line_process( ::san::blur::line_adaptor & line, int beg, int end/*exclusive*/, int radius ) {
	int den = radius * (radius + 2) + 1;
	int div = radius * 2 + 1;
	uint32_t * p_stack = (uint32_t *)SAN_STACK_ALLOC( sizeof( uint32_t ) * div );

	// Fill initial stack...
	NaiveCalcT sum, sum_in, sum_out;
	for ( int i = -radius; i <= radius; i++ ) {
		uint32_t c = line.get_pix( beg + i );
		p_stack[i + radius] = c;
		sum += NaiveCalcT( c ) * (radius - std::abs( i ) + 1);
		if ( i <= 0 ) {
			sum_out += c;
		} else {
			sum_in  += c;
		}
	}

	int i_stack = radius;
	for ( int i = beg; i < end; i++ ) {
		line.set_pix( i, uint32_t( sum / den ) );

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

template <typename NaiveCalcT, typename ParallelFor>
void naive( san::surface_view & image, ParallelFor & parallel_for, int radius, int override_num_threads ) {
	if ( radius <= 0 ) return;

	// Horizontal pass...
	parallel_for.run_and_wait( 0, image.height(), [&]( int a, int b ) {
		for ( int y = a; y < b; y++ ) {
			::san::blur::line_adaptor line( (uint32_t *)image.row_ptr( y ), image.width(), 1/*advance*/ );
			naive_line_process<NaiveCalcT>( line, 0, image.width(), radius );
		}
	}, override_num_threads );

	// Vertical pass...
	parallel_for.run_and_wait( 0, image.width(), [&]( int a, int b ) {
		for ( int x = a; x < b; x++ ) {
			::san::blur::line_adaptor line( (uint32_t *)image.col_ptr( x ), image.height(), image.stride() / 4/*sizeof uint32_t*/ );
			naive_line_process<NaiveCalcT>( line, 0, image.height(), radius );
		}
	}, override_num_threads );
}

} // namespace san::blur::stack
