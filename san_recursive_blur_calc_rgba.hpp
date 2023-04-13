#pragma once

//#include <cmath>

namespace san {

template <class T = float>
struct recursive_blur_calc_rgba {
	typedef T value_type;
	typedef recursive_blur_calc_rgba <T> self_type;

	value_type r, g, b, a;

	template <class ColorT> 
	void from_pix( const ColorT & c ) {
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
	}

	void calc( value_type b1, value_type b2, value_type b3, value_type b4,
		const self_type & c1,
		const self_type & c2,
		const self_type & c3,
		const self_type & c4 )
	{
		r = b1 * c1.r + b2 * c2.r + b3 * c3.r + b4 * c4.r;
		g = b1 * c1.g + b2 * c2.g + b3 * c3.g + b4 * c4.g;
		b = b1 * c1.b + b2 * c2.b + b3 * c3.b + b4 * c4.b;
		a = b1 * c1.a + b2 * c2.a + b3 * c3.a + b4 * c4.a;
	}

	template <class ColorT> 
	void to_pix( ColorT & c ) const {
		typedef typename ColorT::value_type cv_type;
#if 0
		c.r = (cv_type)uround( r );
		c.g = (cv_type)uround( g );
		c.b = (cv_type)uround( b );
		c.a = (cv_type)uround( a );
#endif

#if 0
		c.r = std::lround( r );
		c.g = std::lround( g );
		c.b = std::lround( b );
		c.a = std::lround( a );
#else
		c.r = (cv_type)( r );
		c.g = (cv_type)( g );
		c.b = (cv_type)( b );
		c.a = (cv_type)( a );
#endif
	}
};

} // namespace san
