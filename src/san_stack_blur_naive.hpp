#pragma once

namespace san {

struct accum {
	int	c[4];

	accum() { zero(); }

	accum( int x, int y, int z, int w ) {
		c[0] = x;
		c[1] = y;
		c[2] = z;
		c[3] = w;
	}

	accum( uint32_t value ) {
		c[3] = (value >> 24) & 0xff;
		c[2] = (value >> 16) & 0xff;
		c[1] = (value >>  8) & 0xff;
		c[0] =  value        & 0xff;
	}

	void zero() { c[0] = c[1] = c[2] = c[3] = 0; }

	accum & operator = ( uint32_t value ) {
		c[3] = (value >> 24) & 0xff;
		c[2] = (value >> 16) & 0xff;
		c[1] = (value >>  8) & 0xff;
		c[0] =  value        & 0xff;
		return *this;
	}

	accum & operator += ( const accum & rhs ) {
		c[0] += rhs.c[0];
		c[1] += rhs.c[1];
		c[2] += rhs.c[2];
		c[3] += rhs.c[3];
		return *this;
	}

	accum & operator += ( uint32_t value ) {
		c[3] += (value >> 24) & 0xff;
		c[2] += (value >> 16) & 0xff;
		c[1] += (value >>  8) & 0xff;
		c[0] +=  value        & 0xff;
		return *this;
	}

	accum & operator -= ( uint32_t value ) {
		c[3] -= (value >> 24) & 0xff;
		c[2] -= (value >> 16) & 0xff;
		c[1] -= (value >>  8) & 0xff;
		c[0] -=  value        & 0xff;
		return *this;
	}

	accum & operator -= ( const accum & rhs ) {
		c[0] -= rhs.c[0];
		c[1] -= rhs.c[1];
		c[2] -= rhs.c[2];
		c[3] -= rhs.c[3];
		return *this;
	}

	accum operator * ( int value ) const {
		return accum(
			c[0] * value,
			c[1] * value,
			c[2] * value,
			c[3] * value );
	}

	accum operator / ( int value ) const {
		return accum(
			c[0] / value,
			c[1] / value,
			c[2] / value,
			c[3] / value );
	}

	accum operator >> ( uint8_t value ) const {
		return accum(
			c[0] >> value,
			c[1] >> value,
			c[2] >> value,
			c[3] >> value );
	}

	explicit operator uint32_t () const {
		return ((c[3] & 0xff) << 24) |
			   ((c[2] & 0xff) << 16) |
			   ((c[1] & 0xff) <<  8) |
			    (c[0] & 0xff);
	}
}; // struct accum


class line {
	uint32_t *	m_ptr;
	int			m_len;
	int			m_advance;

public:
	line( uint32_t * p, int len, int advance ) :
		m_ptr( p ), m_len( len ), m_advance( advance ) {}

	uint32_t get_pix( int i ) const {
		if ( i < 0 ) i = 0; else if ( i >= m_len ) i = m_len - 1;
		return m_ptr[i * m_advance];
	}

	void set_pix( int i, uint32_t value ) {
		m_ptr[i * m_advance] = value;
	}
}; // struct line


void stack_blur_line_naive( line & r, int head, int tail/*exclusive*/, int radius ) {
	int den = radius * (radius + 2) + 1;
	int div = radius * 2 + 1;
	uint32_t * p_stack = (uint32_t *)__builtin_alloca_with_align( sizeof( uint32_t ) * div, 128 );

	// Fill initial stack...
	accum sum, sum_in, sum_out;
	for ( int i = -radius; i <= radius; i++ ) {
		uint32_t c = r.get_pix( head + i );
		p_stack[i + radius] = c;
		sum += accum( c ) * (radius - std::abs( i ) + 1);
		if ( i <= 0 ) {
			sum_out += c;
		} else {
			sum_in  += c;
		}
	}

	int i_stack = radius;
	for ( int i = head; i < tail; i++ ) {
		r.set_pix( i, uint32_t( sum / den ) );

		sum -= sum_out;

		int stack_start = i_stack + div - radius;
		if ( stack_start >= div ) stack_start -= div;

		sum_out -= p_stack[stack_start];

		uint32_t c = r.get_pix( i + radius + 1 );

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

void stack_blur_naive( san::image_view & image, int radius ) {
	if ( radius <= 0 ) return;

	// Horizontal pass...
	for ( int y = 0; y < image.height(); y++ ) {
		san::line r( (uint32_t *)image.row_ptr( y ), image.width(), 1/*advance*/ );
		san::stack_blur_line_naive( r, 0, image.width(), radius );
	}

	// Vertical pass...
	for ( int x = 0; x < image.width(); x++ ) {
		san::line r( (uint32_t *)image.col_ptr( x ), image.height(), image.stride() / 4/*sizeof uint32_t*/ );
		san::stack_blur_line_naive( r, 0, image.height(), radius );
	}
}

template <typename ParallelFor>
void stack_blur_naive_mt( san::image_view & image, int radius, ParallelFor & parallel_for, int override_num_threads = 0 ) {
//void stack_blur_naive_mt( san::image_view & image, int radius, san::parallel_for & parallel_for, int override_num_threads = 0 ) {
	if ( radius <= 0 ) return;

	// Horizontal pass...
	parallel_for.run_and_wait( 0, image.height(), [&]( int a, int b ) {
		for ( int y = a; y < b; y++ ) {
			san::line r( (uint32_t *)image.row_ptr( y ), image.width(), 1/*advance*/ );
			san::stack_blur_line_naive( r, 0, image.width(), radius );
		}
	}, override_num_threads );

	// Vertical pass...
	parallel_for.run_and_wait( 0, image.width(), [&]( int a, int b ) {
		for ( int x = a; x < b; x++ ) {
			san::line r( (uint32_t *)image.col_ptr( x ), image.height(), image.stride() / 4/*sizeof uint32_t*/ );
			san::stack_blur_line_naive( r, 0, image.height(), radius );
		}
	}, override_num_threads );
}

} // namespace san
