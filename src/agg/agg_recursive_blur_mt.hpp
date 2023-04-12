#pragma once

// Multithreaded version
// TODO: SIMD version of ColorT and CalculatorT types

namespace agg {

    template <class ColorT, class CalculatorT> class recursive_blur_mt {
    public:
        typedef ColorT color_type;
        typedef CalculatorT calculator_type;
        typedef typename color_type::value_type value_type;
        typedef typename calculator_type::value_type calc_type;

        //--------------------------------------------------------------------
        template <class Img, typename ParallelFor>
		void blur_x( Img & img, double radius, ParallelFor & parallel_for ) {
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

			parallel_for.run_and_wait( 0, h,
				[&]( const int head, const int tail ) {

			agg::pod_vector <calculator_type> m_sum1;
			agg::pod_vector <calculator_type> m_sum2;
			agg::pod_vector <color_type>      m_buf;

            m_sum1.allocate(w);
            m_sum2.allocate(w);
            m_buf.allocate(w);


            for(int y = head; y < tail; y++) {
            //for(int y = 0; y < h; y++) {
                calculator_type c;
                c.from_pix(img.pixel(0, y));
                m_sum1[0].calc(b, b1, b2, b3, c, c, c, c);

                c.from_pix(img.pixel(1, y));
                m_sum1[1].calc(b, b1, b2, b3, c, m_sum1[0], m_sum1[0], m_sum1[0]);

                c.from_pix(img.pixel(2, y));
                m_sum1[2].calc(b, b1, b2, b3, c, m_sum1[1], m_sum1[0], m_sum1[0]);

                for(int x = 3; x < w; ++x) {
                    c.from_pix(img.pixel(x, y));
                    m_sum1[x].calc(b, b1, b2, b3, c, m_sum1[x-1], m_sum1[x-2], m_sum1[x-3]);
                }
    
                m_sum2[wm  ].calc(b, b1, b2, b3, m_sum1[wm  ], m_sum1[wm  ], m_sum1[wm], m_sum1[wm]);
                m_sum2[wm-1].calc(b, b1, b2, b3, m_sum1[wm-1], m_sum2[wm  ], m_sum2[wm], m_sum2[wm]);
                m_sum2[wm-2].calc(b, b1, b2, b3, m_sum1[wm-2], m_sum2[wm-1], m_sum2[wm], m_sum2[wm]);
                m_sum2[wm  ].to_pix(m_buf[wm  ]);
                m_sum2[wm-1].to_pix(m_buf[wm-1]);
                m_sum2[wm-2].to_pix(m_buf[wm-2]);

                for(int x = wm-3; x >= 0; --x) {
                    m_sum2[x].calc(b, b1, b2, b3, m_sum1[x], m_sum2[x+1], m_sum2[x+2], m_sum2[x+3]);
                    m_sum2[x].to_pix(m_buf[x]);
                }
                img.copy_color_hspan(0, y, w, &m_buf[0]);
            }
			} ); // parallel_for
        }

        //--------------------------------------------------------------------
        template <class Img, typename ParallelFor>
		void blur_y( Img & img, double radius, ParallelFor & parallel_for ) {
            pixfmt_transposer <Img> img2( img );
            blur_x( img2, radius, parallel_for );
        }

        //--------------------------------------------------------------------
        template <class Img, typename ParallelFor>
		void blur( Img & img, double radius, ParallelFor & parallel_for ) {
            blur_x( img, radius, parallel_for );
            pixfmt_transposer <Img> img2( img );
            blur_x( img2, radius, parallel_for );
        }
#if 0
    private:
        agg::pod_vector<calculator_type> m_sum1;
        agg::pod_vector<calculator_type> m_sum2;
        agg::pod_vector<color_type>      m_buf;
#endif
    };

#if 0
    //=================================================recursive_blur_calc_rgba
    template<class T=double> struct recursive_blur_calc_rgba {
        typedef T value_type;
        typedef recursive_blur_calc_rgba<T> self_type;

        value_type r,g,b,a;

        template<class ColorT> 
        AGG_INLINE void from_pix(const ColorT& c) {
            r = c.r;
            g = c.g;
            b = c.b;
            a = c.a;
        }

        AGG_INLINE void calc(value_type b1, 
                             value_type b2, 
                             value_type b3, 
                             value_type b4,
                             const self_type& c1, 
                             const self_type& c2, 
                             const self_type& c3, 
                             const self_type& c4)
        {
            r = b1*c1.r + b2*c2.r + b3*c3.r + b4*c4.r;
            g = b1*c1.g + b2*c2.g + b3*c3.g + b4*c4.g;
            b = b1*c1.b + b2*c2.b + b3*c3.b + b4*c4.b;
            a = b1*c1.a + b2*c2.a + b3*c3.a + b4*c4.a;
        }

        template<class ColorT> 
        AGG_INLINE void to_pix(ColorT& c) const {
            typedef typename ColorT::value_type cv_type;
            c.r = (cv_type)uround(r);
            c.g = (cv_type)uround(g);
            c.b = (cv_type)uround(b);
            c.a = (cv_type)uround(a);
        }
    };
#endif

} // namespace agg
