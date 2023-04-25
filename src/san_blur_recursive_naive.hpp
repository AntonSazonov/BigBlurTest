#pragma once

namespace san::blur::recursive {

// std::common_type<T, U>::type
// std::common_type<int, double>::type == double

template <typename ValueT = float>
struct naive_calc_t {
	using	value_type	= ValueT;

	ValueT r, g, b, a;

	void from_pix( uint32_t c ) {
		r = (c >> 24) & 0xff;
		g = (c >> 16) & 0xff;
		b = (c >>  8) & 0xff;
		a =  c        & 0xff;
	}

	void to_pix( uint32_t & c ) const {
		c =  (uint8_t(r) << 24) |
			 (uint8_t(g) << 16) |
			 (uint8_t(b) <<  8) |
			  uint8_t(a);
	}

	void calc( ValueT b1, ValueT b2, ValueT b3, ValueT b4, const naive_calc_t & c1, const naive_calc_t & c2, const naive_calc_t & c3, const naive_calc_t & c4 ) {
		r = b1 * c1.r + b2 * c2.r + b3 * c3.r + b4 * c4.r;
		g = b1 * c1.g + b2 * c2.g + b3 * c3.g + b4 * c4.g;
		b = b1 * c1.b + b2 * c2.b + b3 * c3.b + b4 * c4.b;
		a = b1 * c1.a + b2 * c2.a + b3 * c3.a + b4 * c4.a;
	}
}; // struct naive_calc_t

template <typename CalcT = naive_calc_t<>>
class naive {
	using	value_type	= typename CalcT::value_type;

	void calc_coefficients( value_type radius, value_type & c, value_type & c1, value_type & c2, value_type & c3 ) {
		value_type s = radius * 0.5f;

		value_type q1  = s < 2.5f
			? 3.97156f - 4.14554f * std::sqrt( 1 - 0.26891f * s )
			: 0.98711f * s - 0.96330f;

		value_type q2 = q1 * q1;
		value_type q3 = q2 * q1;

		value_type c0;
		c0 = 1.57825f + 2.44413f * q1 +  1.42810f * q2 +  0.422205f * q3;
		c1 =            2.44413f * q1 +  2.85619f * q2 +  1.266610f * q3;
		c2 =                            -1.42810f * q2 + -1.266610f * q3;
		c3 =                                              0.422205f * q3;

		c   = 1 - (c1 + c2 + c3) / c0;
		c1 /= c0;
		c2 /= c0;
		c3 /= c0;
	}

public:
	template <typename ImageViewT, typename ParallelForT>
	void operator () ( ImageViewT & image, ParallelForT & parallel_for, value_type radius, int override_num_threads ) {

		if ( image.width() < 3 ) return;
		if ( radius < 0.62f ) return;
		//if ( radius > 120 ) radius = 120;

		value_type b, b1, b2, b3;
		calc_coefficients( radius, b, b1, b2, b3 );

		int w = image.width();
		int h = image.height();
		int wm = w - 1;

		CalcT *		m_sum1 = (CalcT *)SAN_STACK_ALLOC( sizeof( CalcT ) * w );
		CalcT *		m_sum2 = (CalcT *)SAN_STACK_ALLOC( sizeof( CalcT ) * w );
		uint32_t *	m_buf  = (uint32_t *)SAN_STACK_ALLOC( sizeof( uint32_t ) * w );

		for ( int y = 0; y < h; y++ ) {
			CalcT c;


			c.from_pix( *((uint32_t *)image.pix_ptr( 0, y )) );
			m_sum1[0].calc( b, b1, b2, b3, c, c, c, c );

			c.from_pix( *((uint32_t *)image.pix_ptr( 1, y )) );
			m_sum1[1].calc( b, b1, b2, b3, c, m_sum1[0], m_sum1[0], m_sum1[0] );

			c.from_pix( *((uint32_t *)image.pix_ptr( 2, y )) );
			m_sum1[2].calc( b, b1, b2, b3, c, m_sum1[1], m_sum1[0], m_sum1[0] );

			for ( int x = 3; x < w; ++x ) {
				c.from_pix( *((uint32_t *)image.pix_ptr( x, y )) );
				m_sum1[x].calc( b, b1, b2, b3, c, m_sum1[x - 1], m_sum1[x - 2], m_sum1[x - 3] );
			}


			m_sum2[wm    ].calc( b, b1, b2, b3, m_sum1[wm    ], m_sum1[wm    ], m_sum1[wm], m_sum1[wm] );
			m_sum2[wm - 1].calc( b, b1, b2, b3, m_sum1[wm - 1], m_sum2[wm    ], m_sum2[wm], m_sum2[wm] );
			m_sum2[wm - 2].calc( b, b1, b2, b3, m_sum1[wm - 2], m_sum2[wm - 1], m_sum2[wm], m_sum2[wm] );

			m_sum2[wm    ].to_pix( m_buf[wm    ] );
			m_sum2[wm - 1].to_pix( m_buf[wm - 1] );
			m_sum2[wm - 2].to_pix( m_buf[wm - 2] );

			for ( int x = wm - 3; x >= 0; --x ) {
				m_sum2[x].calc( b, b1, b2, b3, m_sum1[x], m_sum2[x + 1], m_sum2[x + 2], m_sum2[x + 3] );
				m_sum2[x].to_pix( m_buf[x] );
			}

			//image.copy_color_hspan(0, y, w, &m_buf[0]);

			//void copy_color_hspan( int x, int y, unsigned len, const color_type * colors ) const {
				int len = w;
				uint8_t * p = image.row_ptr( y );// + (x << 2);
				uint32_t * colors = &m_buf[0];
				do {
					*((uint32_t *)p) = *colors;
					++colors;
					p += 4;
				} while ( --len );
			//}
		}
	}
}; // class naive

#if 0
    //===========================================================recursive_blur
    template<class ColorT, class CalculatorT> class recursive_blur
    {
    public:
        typedef ColorT color_type;
        typedef CalculatorT calculator_type;
        typedef typename color_type::value_type value_type;
        typedef typename calculator_type::value_type calc_type;

        //--------------------------------------------------------------------
		template <typename Img, typename ParallelForT>
		void blur_x( Img & img, ParallelForT & parallel_for, double radius, int override_num_threads ) {
            if(radius < 0.62) return;
            if(img.width() < 3) return;

            calc_type s = calc_type(radius * 0.5);
            calc_type q = calc_type((s < 2.5) ?
                                    3.97156 - 4.14554 * sqrt(1 - 0.26891 * s) :
                                    0.98711 * s - 0.96330);

            calc_type q2 = calc_type(q * q);
            calc_type q3 = calc_type(q2 * q);

            calc_type b0 = calc_type(1.0 / (1.578250 + 
                                            2.444130 * q + 
                                            1.428100 * q2 + 
                                            0.422205 * q3));

            calc_type b1 = calc_type( 2.44413 * q + 
                                      2.85619 * q2 + 
                                      1.26661 * q3);

            calc_type b2 = calc_type(-1.42810 * q2 + 
                                     -1.26661 * q3);

            calc_type b3 = calc_type(0.422205 * q3);

            calc_type b  = calc_type(1 - (b1 + b2 + b3) * b0);

            b1 *= b0;
            b2 *= b0;
            b3 *= b0;

            int w = img.width();
            int h = img.height();
            int wm = w-1;

			// parallelize...
			parallel_for.run_and_wait( 0, h, [&]( int pf_begin, int pf_end ) {

            agg::pod_vector<calculator_type> m_sum1;
            agg::pod_vector<calculator_type> m_sum2;
            agg::pod_vector<color_type>      m_buf;

            m_sum1.allocate(w);
            m_sum2.allocate(w);
            m_buf.allocate(w);

            for( int y = pf_begin; y < pf_end; y++)
            {
                calculator_type c;
                c.from_pix(img.pixel(0, y));
                m_sum1[0].calc(b, b1, b2, b3, c, c, c, c);
                c.from_pix(img.pixel(1, y));
                m_sum1[1].calc(b, b1, b2, b3, c, m_sum1[0], m_sum1[0], m_sum1[0]);
                c.from_pix(img.pixel(2, y));
                m_sum1[2].calc(b, b1, b2, b3, c, m_sum1[1], m_sum1[0], m_sum1[0]);

                for( int x = 3; x < w; ++x)
                {
                    c.from_pix(img.pixel(x, y));
                    m_sum1[x].calc(b, b1, b2, b3, c, m_sum1[x-1], m_sum1[x-2], m_sum1[x-3]);
                }
    
                m_sum2[wm  ].calc(b, b1, b2, b3, m_sum1[wm  ], m_sum1[wm  ], m_sum1[wm], m_sum1[wm]);
                m_sum2[wm-1].calc(b, b1, b2, b3, m_sum1[wm-1], m_sum2[wm  ], m_sum2[wm], m_sum2[wm]);
                m_sum2[wm-2].calc(b, b1, b2, b3, m_sum1[wm-2], m_sum2[wm-1], m_sum2[wm], m_sum2[wm]);
                m_sum2[wm  ].to_pix(m_buf[wm  ]);
                m_sum2[wm-1].to_pix(m_buf[wm-1]);
                m_sum2[wm-2].to_pix(m_buf[wm-2]);

                for( int x = wm-3; x >= 0; --x)
                {
                    m_sum2[x].calc(b, b1, b2, b3, m_sum1[x], m_sum2[x+1], m_sum2[x+2], m_sum2[x+3]);
                    m_sum2[x].to_pix(m_buf[x]);
                }
                img.copy_color_hspan(0, y, w, &m_buf[0]);
            }

			}, override_num_threads );
        }

        //--------------------------------------------------------------------
		template <typename Img, typename ParallelForT>
		void blur( Img & img, ParallelForT & parallel_for, double radius, int override_num_threads ) {
            blur_x( img, parallel_for, radius, override_num_threads );
            pixfmt_transposer <Img> img2( img );
            blur_x( img2, parallel_for, radius, override_num_threads );
        }
    };
}
#endif

} // namespace san::blur::recursive
