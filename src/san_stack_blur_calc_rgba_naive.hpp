#pragma once

namespace san::stack_blur_calc_rgba {

class naive {
	int	c[4];

public:
	naive() {
		c[0] = c[1] = c[2] = c[3] = 0;
	}

	naive( int x, int y, int z, int w ) {
		c[0] = x;
		c[1] = y;
		c[2] = z;
		c[3] = w;
	}

	naive( uint32_t value ) {
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

	naive & operator = ( uint32_t value ) {
		c[3] = (value >> 24) & 0xff;
		c[2] = (value >> 16) & 0xff;
		c[1] = (value >>  8) & 0xff;
		c[0] =  value        & 0xff;
		return *this;
	}

	naive & operator += ( const naive & rhs ) {
		c[0] += rhs.c[0];
		c[1] += rhs.c[1];
		c[2] += rhs.c[2];
		c[3] += rhs.c[3];
		return *this;
	}

	naive & operator -= ( const naive & rhs ) {
		c[0] -= rhs.c[0];
		c[1] -= rhs.c[1];
		c[2] -= rhs.c[2];
		c[3] -= rhs.c[3];
		return *this;
	}

	naive operator * ( int value ) const {
		return naive(
			c[0] * value,
			c[1] * value,
			c[2] * value,
			c[3] * value );
	}

	naive operator / ( int value ) const {
		return naive(
			c[0] / value,
			c[1] / value,
			c[2] / value,
			c[3] / value );
	}

#if 0
	// May be will be used later...
	naive operator >> ( uint8_t value ) const {
		return naive(
			c[0] >> value,
			c[1] >> value,
			c[2] >> value,
			c[3] >> value );
	}
#endif
}; // struct naive

} // namespace san::stack_blur_calc_rgba