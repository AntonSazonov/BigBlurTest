#pragma once

#include <vector>

// Two-pass gaussian implementation

namespace san::blur::gaussian {

template <int MaxRadius, typename ValueT>
class kernel {
	static_assert( std::is_floating_point_v<ValueT> );

	std::array <ValueT, MaxRadius * 2 + 1>	m_kernel;

	int		m_radius;
	int		m_radius2;

	ValueT	m_sigma;
	ValueT	m_sigma_coefficient;
	ValueT	m_sum;					// Sum of all kernel's values. Used for normalization.

	void calculate() {
		m_sum = 0;
		for ( int i = -m_radius; i <= m_radius; ++i ) {
			ValueT value = std::exp( -(i * i) / (m_sigma * m_sigma * 2) );
			m_kernel[i + m_radius] = value;
			m_sum += value;
		}
	}

public:
	using	value_type	= ValueT;

	kernel( ValueT sigma_coefficient = 2.5 ) : m_sigma_coefficient( sigma_coefficient ) {}

	void set_radius( int radius ) {
		if ( radius < 1 ) radius = 1; else
		if ( radius > MaxRadius ) radius = MaxRadius;
		m_radius  = radius;
		m_radius2 = radius * 2;
		m_sigma   = radius / m_sigma_coefficient;
		calculate();
	}

	void normalize() {
		for ( int i = 0; i <= m_radius2; ++i ) {
			m_kernel[i] /= m_sum;
		}
	}

	const ValueT & operator [] ( int i ) const { return m_kernel[i]; }
	      ValueT & operator [] ( int i )       { return m_kernel[i]; }

	int radius() const { return m_radius; }
}; // class kernel


class naive {

	template <typename KernelT>
	void do_line( const KernelT & kernel, adaptor::straight_line & line, int beg, int end ) {

		int		radius = kernel.radius();
		int		length = end - beg;

		// That's not stack from Stack Blur, that's memory allocated in stack.
		uint32_t *	p_stack = (uint32_t *)SAN_STACK_ALLOC( sizeof( uint32_t ) * length );
		uint32_t *	p_dst = p_stack;

		for ( int coord = beg; coord < end; coord++ ) {

			typename KernelT::value_type sum_r = 0;
			typename KernelT::value_type sum_g = 0;
			typename KernelT::value_type sum_b = 0;
			typename KernelT::value_type sum_a = 0;

			for ( int i = -radius; i <= radius; i++ ) {
				uint32_t c = line.get_pix( coord + i );

				typename KernelT::value_type weight = kernel[i + radius];
				sum_r += ( c        & 0xff) * weight;
				sum_g += ((c >> 8)  & 0xff) * weight;
				sum_b += ((c >> 16) & 0xff) * weight;
				sum_a += ((c >> 24) & 0xff) * weight;
			}

			uint32_t c = ((uint32_t(sum_a) & 0xff) << 24) |
						 ((uint32_t(sum_b) & 0xff) << 16) |
						 ((uint32_t(sum_g) & 0xff) <<  8) |
						  (uint32_t(sum_r) & 0xff);

			*p_dst++ = c;
		}

		// Blit blurred line back...
		line.set_pix_start( beg );
		while ( length-- > 0 ) {
			line.set_pix_next( *p_stack++ );
		}
	}

public:
	template <typename KernelT, typename ImageViewT, typename ParallelForT>
	void operator () ( const KernelT & kernel, ImageViewT & image, ParallelForT & parallel_for, int override_num_threads ) {
		assert( image.components() == 4 );

		// Horizontal pass...
		parallel_for.run_and_wait( 0, image.height(), [&]( int beg, int end ) {
			for ( int i = beg; i < end; i++ ) {
				adaptor::straight_line line( (uint32_t *)image.row_ptr( i ), image.width(), 1 );
				do_line( kernel, line, 0, image.width() );
			}
		}, override_num_threads );

		// Vertical pass...
		parallel_for.run_and_wait( 0, image.width(), [&]( int beg, int end ) {
			for ( int i = beg; i < end; i++ ) {
				adaptor::straight_line line( (uint32_t *)image.col_ptr( i ), image.height(), image.stride() / image.components() );
				do_line( kernel, line, 0, image.height() );
			}
		}, override_num_threads );
	}
}; // class naive


template <int MaxRadius, typename ValueT>
class naive_test {
	kernel <MaxRadius, ValueT>	m_kernel;
	naive						m_filter;

public:
	template <typename ImageViewT, typename ParallelForT>
	void operator () ( ImageViewT & image, ParallelForT & parallel_for, int radius, int override_num_threads ) {
		if ( radius < 1 ) return;
		m_kernel.set_radius( radius );
		m_kernel.normalize();
		m_filter( m_kernel, image, parallel_for, override_num_threads );
	}
}; // class naive_test

} // namespace san::blur::gaussian
