#pragma once

#include <cmath>

namespace san::recursive_blur {

struct calc_t {
	float r, g, b, a;

	void from_pix( uint32_t c ) {
		r = (c >> 24) & 0xff;
		g = (c >> 16) & 0xff;
		b = (c >>  8) & 0xff;
		a =  c        & 0xff;
	}

	void calc( float b1, float b2, float b3, float b4, const calc_t & c1, const calc_t & c2, const calc_t & c3, const calc_t & c4 ) {
		r = b1 * c1.r + b2 * c2.r + b3 * c3.r + b4 * c4.r;
		g = b1 * c1.g + b2 * c2.g + b3 * c3.g + b4 * c4.g;
		b = b1 * c1.b + b2 * c2.b + b3 * c3.b + b4 * c4.b;
		a = b1 * c1.a + b2 * c2.a + b3 * c3.a + b4 * c4.a;
	}

	void to_pix( uint32_t & c ) const {
		c =  (uint8_t(r) << 24) |
			 (uint8_t(g) << 16) |
			 (uint8_t(b) <<  8) |
			  uint8_t(a);
	}
};

class blur {
	agg::pod_vector<calc_t>		m_sum1;
	agg::pod_vector<calc_t>		m_sum2;
	agg::pod_vector<uint32_t>	m_buf;

public:
	template <class Img>
	void blur_x( Img & img, float radius ) {
		if ( img.width() < 3 ) return;
		if ( radius < 0.62f ) return;
		if ( radius > 100 ) radius = 100;

		float s = radius * 0.5f;
		float q  = (s < 2.5) ? 3.97156f - 4.14554f * sqrt(1 - 0.26891f * s) : 0.98711f * s - 0.96330f;
		float q2 = q * q;
		float q3 = q2 * q;
		float b0 = 1.0f / (1.578250f + 2.444130f * q + 1.428100f * q2 + 0.422205f * q3);
		float b1 = 2.44413f * q + 2.85619f * q2 + 1.26661f * q3;
		float b2 = -1.42810f * q2 + -1.26661f * q3;
		float b3 = 0.422205f * q3;
		float b  = 1 - (b1 + b2 + b3) * b0;

		b1 *= b0;
		b2 *= b0;
		b3 *= b0;

		int w = img.width();
		int h = img.height();
		int wm = w-1;
		int x, y;

		m_sum1.allocate(w);
		m_sum2.allocate(w);
		m_buf.allocate(w);

		for(y = 0; y < h; y++) {
			calc_t c;

			c.from_pix( *((uint32_t *)img.pix_ptr(0, y)) );
			m_sum1[0].calc(b, b1, b2, b3, c, c, c, c);

			c.from_pix( *((uint32_t *)img.pix_ptr(1, y)) );
			m_sum1[1].calc(b, b1, b2, b3, c, m_sum1[0], m_sum1[0], m_sum1[0]);

			c.from_pix( *((uint32_t *)img.pix_ptr(2, y)) );
			m_sum1[2].calc(b, b1, b2, b3, c, m_sum1[1], m_sum1[0], m_sum1[0]);

			for(x = 3; x < w; ++x) {
				c.from_pix( *((uint32_t *)img.pix_ptr(x, y)) );
				m_sum1[x].calc(b, b1, b2, b3, c, m_sum1[x-1], m_sum1[x-2], m_sum1[x-3]);
			}

			m_sum2[wm  ].calc(b, b1, b2, b3, m_sum1[wm  ], m_sum1[wm  ], m_sum1[wm], m_sum1[wm]);
			m_sum2[wm-1].calc(b, b1, b2, b3, m_sum1[wm-1], m_sum2[wm  ], m_sum2[wm], m_sum2[wm]);
			m_sum2[wm-2].calc(b, b1, b2, b3, m_sum1[wm-2], m_sum2[wm-1], m_sum2[wm], m_sum2[wm]);

			m_sum2[wm  ].to_pix(m_buf[wm  ]);
			m_sum2[wm-1].to_pix(m_buf[wm-1]);
			m_sum2[wm-2].to_pix(m_buf[wm-2]);

			for(x = wm-3; x >= 0; --x) {
				m_sum2[x].calc(b, b1, b2, b3, m_sum1[x], m_sum2[x+1], m_sum2[x+2], m_sum2[x+3]);
				m_sum2[x].to_pix(m_buf[x]);
			}

			//img.copy_color_hspan(0, y, w, &m_buf[0]);
			//void copy_color_hspan( int x, int y, unsigned len, const color_type * colors ) const {
				int len = w;
				uint8_t * p = img.row_ptr( y );// + (x << 2);
				uint32_t * colors = &m_buf[0];
				do {
					*((uint32_t *)p) = *colors;
					++colors;
					p += 4;
				} while ( --len );
			//}


		}
	}
}; // blur

} // namespace san::recursive_blur
