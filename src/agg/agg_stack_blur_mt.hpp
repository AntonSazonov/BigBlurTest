#pragma once

// Multithreaded version
// TODO: SIMD version of ColorT and CalculatorT types

namespace agg {

    //==============================================================stack_blur
    template<class ColorT, class CalculatorT> class stack_blur_mt
    {
    public:
        typedef ColorT      color_type;
        typedef CalculatorT calculator_type;

        //--------------------------------------------------------------------
        template<class Img, typename ParallelFor>
		void blur_x( Img & img, int radius, ParallelFor & parallel_for )
        {
            if(radius < 1) return;

            int w   = img.width();
            int h   = img.height();
            int wm  = w - 1;
            int div = radius * 2 + 1;

            int div_sum = (radius + 1) * (radius + 1);
            int mul_sum = 0;
            int shr_sum = 0;
            int max_val = color_type::base_mask;

            if(max_val <= 255 && radius < 255) {
                mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[radius];
                shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[radius];
            }

			parallel_for.run_and_wait( 0, h,
				[&]( const int head, const int tail ) {

			pod_vector <color_type> m_buf;
			pod_vector <color_type> m_stack;

            m_buf.allocate(w, 128);
            m_stack.allocate(div, 32);

            int x, y, xp, i;
            int stack_ptr;
            int stack_start;

            color_type      pix;
            color_type*     stack_pix;
            calculator_type sum;
            calculator_type sum_in;
            calculator_type sum_out;

            for(y = head; y < tail; y++)
            {
                sum.clear();
                sum_in.clear();
                sum_out.clear();

                pix = img.pixel(0, y);
                for(i = 0; i <= radius; i++)
                {
                    m_stack[i] = pix;
                    sum.add(pix, i + 1);
                    sum_out.add(pix);
                }
                for(i = 1; i <= radius; i++)
                {
                    pix = img.pixel((i > wm) ? wm : i, y);
                    m_stack[i + radius] = pix;
                    sum.add(pix, radius + 1 - i);
                    sum_in.add(pix);
                }

                stack_ptr = radius;
                for(x = 0; x < w; x++)
                {
                    if(mul_sum) sum.calc_pix(m_buf[x], mul_sum, shr_sum);
                    else        sum.calc_pix(m_buf[x], div_sum);

                    sum.sub(sum_out);
           
                    stack_start = stack_ptr + div - radius;
                    if(stack_start >= div) stack_start -= div;
                    stack_pix = &m_stack[stack_start];

                    sum_out.sub(*stack_pix);

                    xp = x + radius + 1;
                    if(xp > wm) xp = wm;
                    pix = img.pixel(xp, y);
            
                    *stack_pix = pix;
            
                    sum_in.add(pix);
                    sum.add(sum_in);
            
                    ++stack_ptr;
                    if(stack_ptr >= div) stack_ptr = 0;
                    stack_pix = &m_stack[stack_ptr];

                    sum_out.add(*stack_pix);
                    sum_in.sub(*stack_pix);
                }
                img.copy_color_hspan(0, y, w, &m_buf[0]);
            }
			} ); // parallel_for
        }

        template <class Img, typename ParallelFor>
		void blur_y( Img & img, int radius, ParallelFor & parallel_for ) {
            pixfmt_transposer <Img> img2( img );
            blur_x( img2, radius, parallel_for );
        }

        template <class Img, typename ParallelFor>
		void blur( Img & img, int radius, ParallelFor & parallel_for ) {
            blur_x( img, radius, parallel_for );
            pixfmt_transposer <Img> img2( img );
            blur_x( img2, radius, parallel_for );
        }
    }; // class stack_blur_mt


	template <class Img, typename ParallelFor>
    void stack_blur_rgba32_mt( Img & img, unsigned rx, unsigned ry, ParallelFor & parallel_for ) {
        typedef typename Img::color_type color_type;
        typedef typename Img::order_type order_type;
        enum order_e 
        { 
            R = order_type::R, 
            G = order_type::G, 
            B = order_type::B,
            A = order_type::A 
        };

        unsigned w   = img.width();
        unsigned h   = img.height();
        unsigned wm  = w - 1;
        unsigned hm  = h - 1;

        unsigned div;
        unsigned mul_sum;
        unsigned shr_sum;


        if(rx > 0)
        {
            if(rx > 254) rx = 254;
            div = rx * 2 + 1;

            mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[rx];
            shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[rx];

			parallel_for.run_and_wait( 0, h,
				[&]( const int head, const int tail ) {

	        unsigned stack_ptr;
	        unsigned stack_start;

	        const int8u* src_pix_ptr;
	              int8u* dst_pix_ptr;
	        color_type*  stack_pix_ptr;

	        pod_vector<color_type> stack;
            stack.allocate(div);

	        unsigned sum_r;
	        unsigned sum_g;
	        unsigned sum_b;
	        unsigned sum_a;
	        unsigned sum_in_r;
	        unsigned sum_in_g;
	        unsigned sum_in_b;
	        unsigned sum_in_a;
	        unsigned sum_out_r;
	        unsigned sum_out_g;
	        unsigned sum_out_b;
	        unsigned sum_out_a;

            for( int y = head; y < tail; y++) {
                sum_r = 
                sum_g = 
                sum_b = 
                sum_a = 
                sum_in_r = 
                sum_in_g = 
                sum_in_b = 
                sum_in_a = 
                sum_out_r = 
                sum_out_g = 
                sum_out_b = 
                sum_out_a = 0;

                src_pix_ptr = img.pix_ptr(0, y);
                for( int i = 0; i <= rx; i++) {
                    stack_pix_ptr    = &stack[i];
                    stack_pix_ptr->r = src_pix_ptr[R];
                    stack_pix_ptr->g = src_pix_ptr[G];
                    stack_pix_ptr->b = src_pix_ptr[B];
                    stack_pix_ptr->a = src_pix_ptr[A];
                    sum_r           += src_pix_ptr[R] * (i + 1);
                    sum_g           += src_pix_ptr[G] * (i + 1);
                    sum_b           += src_pix_ptr[B] * (i + 1);
                    sum_a           += src_pix_ptr[A] * (i + 1);
                    sum_out_r       += src_pix_ptr[R];
                    sum_out_g       += src_pix_ptr[G];
                    sum_out_b       += src_pix_ptr[B];
                    sum_out_a       += src_pix_ptr[A];
                }
                for( int i = 1; i <= rx; i++) {
                    if(i <= wm) src_pix_ptr += Img::pix_width; 
                    stack_pix_ptr = &stack[i + rx];
                    stack_pix_ptr->r = src_pix_ptr[R];
                    stack_pix_ptr->g = src_pix_ptr[G];
                    stack_pix_ptr->b = src_pix_ptr[B];
                    stack_pix_ptr->a = src_pix_ptr[A];
                    sum_r           += src_pix_ptr[R] * (rx + 1 - i);
                    sum_g           += src_pix_ptr[G] * (rx + 1 - i);
                    sum_b           += src_pix_ptr[B] * (rx + 1 - i);
                    sum_a           += src_pix_ptr[A] * (rx + 1 - i);
                    sum_in_r        += src_pix_ptr[R];
                    sum_in_g        += src_pix_ptr[G];
                    sum_in_b        += src_pix_ptr[B];
                    sum_in_a        += src_pix_ptr[A];
                }

                stack_ptr = rx;
                int xp = rx;
                if(xp > wm) xp = wm;
                src_pix_ptr = img.pix_ptr(xp, y);
                dst_pix_ptr = img.pix_ptr(0, y);
                for( int x = 0; x < w; x++) {
                    dst_pix_ptr[R] = (sum_r * mul_sum) >> shr_sum;
                    dst_pix_ptr[G] = (sum_g * mul_sum) >> shr_sum;
                    dst_pix_ptr[B] = (sum_b * mul_sum) >> shr_sum;
                    dst_pix_ptr[A] = (sum_a * mul_sum) >> shr_sum;
                    dst_pix_ptr += Img::pix_width;

                    sum_r -= sum_out_r;
                    sum_g -= sum_out_g;
                    sum_b -= sum_out_b;
                    sum_a -= sum_out_a;
       
                    stack_start = stack_ptr + div - rx;
                    if(stack_start >= div) stack_start -= div;
                    stack_pix_ptr = &stack[stack_start];

                    sum_out_r -= stack_pix_ptr->r;
                    sum_out_g -= stack_pix_ptr->g;
                    sum_out_b -= stack_pix_ptr->b;
                    sum_out_a -= stack_pix_ptr->a;

                    if(xp < wm) {
                        src_pix_ptr += Img::pix_width;
                        ++xp;
                    }
        
                    stack_pix_ptr->r = src_pix_ptr[R];
                    stack_pix_ptr->g = src_pix_ptr[G];
                    stack_pix_ptr->b = src_pix_ptr[B];
                    stack_pix_ptr->a = src_pix_ptr[A];
        
                    sum_in_r += src_pix_ptr[R];
                    sum_in_g += src_pix_ptr[G];
                    sum_in_b += src_pix_ptr[B];
                    sum_in_a += src_pix_ptr[A];
                    sum_r    += sum_in_r;
                    sum_g    += sum_in_g;
                    sum_b    += sum_in_b;
                    sum_a    += sum_in_a;
        
                    ++stack_ptr;
                    if(stack_ptr >= div) stack_ptr = 0;
                    stack_pix_ptr = &stack[stack_ptr];

                    sum_out_r += stack_pix_ptr->r;
                    sum_out_g += stack_pix_ptr->g;
                    sum_out_b += stack_pix_ptr->b;
                    sum_out_a += stack_pix_ptr->a;
                    sum_in_r  -= stack_pix_ptr->r;
                    sum_in_g  -= stack_pix_ptr->g;
                    sum_in_b  -= stack_pix_ptr->b;
                    sum_in_a  -= stack_pix_ptr->a;
                }
            }
			} );
        }

        if(ry > 0) {
            if(ry > 254) ry = 254;
            div = ry * 2 + 1;
            mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[ry];
            shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[ry];

            int stride = img.stride();

			parallel_for.run_and_wait( 0, w,
				[&]( const int head, const int tail ) {

	        unsigned stack_ptr;
	        unsigned stack_start;

	        const int8u* src_pix_ptr;
	              int8u* dst_pix_ptr;
	        color_type*  stack_pix_ptr;

	        pod_vector<color_type> stack;
            stack.allocate(div);

	        unsigned sum_r;
	        unsigned sum_g;
	        unsigned sum_b;
	        unsigned sum_a;
	        unsigned sum_in_r;
	        unsigned sum_in_g;
	        unsigned sum_in_b;
	        unsigned sum_in_a;
	        unsigned sum_out_r;
	        unsigned sum_out_g;
	        unsigned sum_out_b;
	        unsigned sum_out_a;

            for( int x = head; x < tail; x++) {
                sum_r = 
                sum_g = 
                sum_b = 
                sum_a = 
                sum_in_r = 
                sum_in_g = 
                sum_in_b = 
                sum_in_a = 
                sum_out_r = 
                sum_out_g = 
                sum_out_b = 
                sum_out_a = 0;

                src_pix_ptr = img.pix_ptr(x, 0);
                for( int i = 0; i <= ry; i++) {
                    stack_pix_ptr    = &stack[i];
                    stack_pix_ptr->r = src_pix_ptr[R];
                    stack_pix_ptr->g = src_pix_ptr[G];
                    stack_pix_ptr->b = src_pix_ptr[B];
                    stack_pix_ptr->a = src_pix_ptr[A];
                    sum_r           += src_pix_ptr[R] * (i + 1);
                    sum_g           += src_pix_ptr[G] * (i + 1);
                    sum_b           += src_pix_ptr[B] * (i + 1);
                    sum_a           += src_pix_ptr[A] * (i + 1);
                    sum_out_r       += src_pix_ptr[R];
                    sum_out_g       += src_pix_ptr[G];
                    sum_out_b       += src_pix_ptr[B];
                    sum_out_a       += src_pix_ptr[A];
                }
                for( int i = 1; i <= ry; i++) {
                    if(i <= hm) src_pix_ptr += stride; 
                    stack_pix_ptr = &stack[i + ry];
                    stack_pix_ptr->r = src_pix_ptr[R];
                    stack_pix_ptr->g = src_pix_ptr[G];
                    stack_pix_ptr->b = src_pix_ptr[B];
                    stack_pix_ptr->a = src_pix_ptr[A];
                    sum_r           += src_pix_ptr[R] * (ry + 1 - i);
                    sum_g           += src_pix_ptr[G] * (ry + 1 - i);
                    sum_b           += src_pix_ptr[B] * (ry + 1 - i);
                    sum_a           += src_pix_ptr[A] * (ry + 1 - i);
                    sum_in_r        += src_pix_ptr[R];
                    sum_in_g        += src_pix_ptr[G];
                    sum_in_b        += src_pix_ptr[B];
                    sum_in_a        += src_pix_ptr[A];
                }

                stack_ptr = ry;
                int yp = ry;
                if(yp > hm) yp = hm;
                src_pix_ptr = img.pix_ptr(x, yp);
                dst_pix_ptr = img.pix_ptr(x, 0);
                for( int y = 0; y < h; y++) {
                    dst_pix_ptr[R] = (sum_r * mul_sum) >> shr_sum;
                    dst_pix_ptr[G] = (sum_g * mul_sum) >> shr_sum;
                    dst_pix_ptr[B] = (sum_b * mul_sum) >> shr_sum;
                    dst_pix_ptr[A] = (sum_a * mul_sum) >> shr_sum;
                    dst_pix_ptr += stride;

                    sum_r -= sum_out_r;
                    sum_g -= sum_out_g;
                    sum_b -= sum_out_b;
                    sum_a -= sum_out_a;
       
                    stack_start = stack_ptr + div - ry;
                    if(stack_start >= div) stack_start -= div;

                    stack_pix_ptr = &stack[stack_start];
                    sum_out_r -= stack_pix_ptr->r;
                    sum_out_g -= stack_pix_ptr->g;
                    sum_out_b -= stack_pix_ptr->b;
                    sum_out_a -= stack_pix_ptr->a;

                    if(yp < hm) {
                        src_pix_ptr += stride;
                        ++yp;
                    }
        
                    stack_pix_ptr->r = src_pix_ptr[R];
                    stack_pix_ptr->g = src_pix_ptr[G];
                    stack_pix_ptr->b = src_pix_ptr[B];
                    stack_pix_ptr->a = src_pix_ptr[A];
        
                    sum_in_r += src_pix_ptr[R];
                    sum_in_g += src_pix_ptr[G];
                    sum_in_b += src_pix_ptr[B];
                    sum_in_a += src_pix_ptr[A];
                    sum_r    += sum_in_r;
                    sum_g    += sum_in_g;
                    sum_b    += sum_in_b;
                    sum_a    += sum_in_a;
        
                    ++stack_ptr;
                    if(stack_ptr >= div) stack_ptr = 0;
                    stack_pix_ptr = &stack[stack_ptr];

                    sum_out_r += stack_pix_ptr->r;
                    sum_out_g += stack_pix_ptr->g;
                    sum_out_b += stack_pix_ptr->b;
                    sum_out_a += stack_pix_ptr->a;
                    sum_in_r  -= stack_pix_ptr->r;
                    sum_in_g  -= stack_pix_ptr->g;
                    sum_in_b  -= stack_pix_ptr->b;
                    sum_in_a  -= stack_pix_ptr->a;
                }
            }
			} );
        }
    }

} // namespace agg
