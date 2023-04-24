#pragma once

// Two-pass gaussian implementation

namespace san::blur::gaussian {

template <int MaxRadius, typename ValueT = float>
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


template <typename KernelT>
class naive {
	KernelT &	m_kernel;
	using		value_type = typename KernelT::value_type;

	void do_line( ::san::blur::line_adaptor & line, int beg, int end, int radius ) {

		int			len = end - beg;

		// That's not stack from Stack Blur, that's memory allocated in stack.
		uint32_t *	p_stack = (uint32_t *)SAN_STACK_ALLOC( sizeof( uint32_t ) * len );
		uint32_t *	p_dst = p_stack;

		for ( int coord = beg; coord < end; coord++ ) {

			value_type sum_r = 0;
			value_type sum_g = 0;
			value_type sum_b = 0;
			value_type sum_a = 0;

			for ( int i = -radius; i <= radius; i++ ) {
				uint32_t c = line.get_pix( coord + i );

				value_type weight = m_kernel[i + radius];
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
		p_dst = line.ptr() + beg;
		while ( len-- > 0 ) {
			*p_dst = *p_stack++;
			p_dst += line.advance();
		}
	}

public:
	naive( KernelT & kernel ) : m_kernel( kernel ) {}

	template <typename ImageViewT, typename ParallelForT>
	void blur( ImageViewT & image, ParallelForT & parallel_for, int override_num_threads ) {
		assert( image.components() == 4 );

		// Horizontal pass...
		parallel_for.run_and_wait( 0, image.height(), [&]( int beg, int end ) {
			for ( int i = beg; i < end; i++ ) {
				::san::blur::line_adaptor line( (uint32_t *)image.row_ptr( i ), image.width(), 1 );
				do_line( line, 0, image.width(), m_kernel.radius() );
			}
		}, override_num_threads );

		// Vertical pass...
		parallel_for.run_and_wait( 0, image.width(), [&]( int beg, int end ) {
			for ( int i = beg; i < end; i++ ) {
				::san::blur::line_adaptor line( (uint32_t *)image.col_ptr( i ), image.height(), image.stride() / image.components() );
				do_line( line, 0, image.height(), m_kernel.radius() );
			}
		}, override_num_threads );
	}
}; // class naive


// Very slow implementation. Will be used as reference.
template <size_t MaxRadius>
class naive_test {
	kernel <MaxRadius>			m_kernel;
	naive <decltype(m_kernel)>	m_filter;

public:
	naive_test() : m_filter( m_kernel ) {}

	template <typename ImageViewT, typename ParallelForT>
	void blur( ImageViewT & image, ParallelForT & parallel_for, int radius, int override_num_threads ) {
		if ( radius < 1 ) return;
		m_kernel.set_radius( radius );
		m_kernel.normalize();
		m_filter.blur( image, parallel_for, override_num_threads );
	}
}; // class naive_test


} // namespace san::blur::gaussian
