#pragma once

namespace san::stack_blur {

class naive_calc {
	int	c[4];

public:
	naive_calc() {
		c[0] = c[1] = c[2] = c[3] = 0;
	}

	naive_calc( int x, int y, int z, int w ) {
		c[0] = x;
		c[1] = y;
		c[2] = z;
		c[3] = w;
	}

	naive_calc( uint32_t value ) {
		c[3] = (value >> 24) & 0xff;
		c[2] = (value >> 16) & 0xff;
		c[1] = (value >>  8) & 0xff;
		c[0] =  value        & 0xff;
	}

	operator uint32_t () const {
		return ((c[3] & 0xff) << 24) |
			   ((c[2] & 0xff) << 16) |
			   ((c[1] & 0xff) <<  8) |
			    (c[0] & 0xff);
	}

	naive_calc & operator = ( uint32_t value ) {
		c[3] = (value >> 24) & 0xff;
		c[2] = (value >> 16) & 0xff;
		c[1] = (value >>  8) & 0xff;
		c[0] =  value        & 0xff;
		return *this;
	}

	naive_calc & operator += ( const naive_calc & rhs ) {
		c[0] += rhs.c[0];
		c[1] += rhs.c[1];
		c[2] += rhs.c[2];
		c[3] += rhs.c[3];
		return *this;
	}

	naive_calc & operator -= ( const naive_calc & rhs ) {
		c[0] -= rhs.c[0];
		c[1] -= rhs.c[1];
		c[2] -= rhs.c[2];
		c[3] -= rhs.c[3];
		return *this;
	}

	naive_calc operator * ( int value ) const {
		return naive_calc(
			c[0] * value,
			c[1] * value,
			c[2] * value,
			c[3] * value );
	}

	naive_calc operator / ( int value ) const {
		return naive_calc(
			c[0] / value,
			c[1] / value,
			c[2] / value,
			c[3] / value );
	}

#if 0
	// May be will be used later...
	naive_calc operator >> ( uint8_t value ) const {
		return naive_calc(
			c[0] >> value,
			c[1] >> value,
			c[2] >> value,
			c[3] >> value );
	}
#endif
}; // struct naive_calc

} // namespace san::stack_blur
