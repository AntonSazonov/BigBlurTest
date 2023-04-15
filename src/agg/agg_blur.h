//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------
//
// The Stack Blur Algorithm was invented by Mario Klingemann, 
// mario@quasimondo.com and described here:
// http://incubator.quasimondo.com/processing/fast_blur_deluxe.php
// (search phrase "Stackblur: Fast But Goodlooking"). 
// The major improvement is that there's no more division table
// that was very expensive to create for large blur radii. Insted, 
// for 8-bit per channel and radius not exceeding 254 the division is 
// replaced by multiplication and shift. 
//
//----------------------------------------------------------------------------

//
// My note:
//  * removed unused by me implementations and calculator types (gray, rgb24).
//  * added 'parallel for' loop support
//  * stack_blur_rgba32 now takes one radius parameter
//  * some variables moved inside parallel loop
//
// Anton.
//


#ifndef AGG_BLUR_INCLUDED
#define AGG_BLUR_INCLUDED

#include "agg_array.h"
#include "agg_pixfmt_transposer.h"

namespace agg
{

    template<class T> struct stack_blur_tables
    {
        static int16u const g_stack_blur8_mul[255];
        static int8u  const g_stack_blur8_shr[255];
    };

    //------------------------------------------------------------------------
    template<class T> 
    int16u const stack_blur_tables<T>::g_stack_blur8_mul[255] = 
    {
        512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
        454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
        482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
        437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
        497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
        320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
        446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
        329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
        505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
        399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
        324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
        268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
        451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
        385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
        332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
        289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
    };

    //------------------------------------------------------------------------
    template<class T> 
    int8u const stack_blur_tables<T>::g_stack_blur8_shr[255] = 
    {
          9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 
         17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
         19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
         20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
         21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
         21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
         22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
         23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 
         23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
         24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
    };



    //==============================================================stack_blur
    template<class ColorT, class CalculatorT> class stack_blur
    {
    public:
        typedef ColorT      color_type;
        typedef CalculatorT calculator_type;

        //--------------------------------------------------------------------
		template <typename Img, typename ParallelFor>
		void blur_x( Img & img, ParallelFor & parallel_for, unsigned radius, int override_num_threads )
        {
            if(radius < 1) return;

            unsigned w   = img.width();
            unsigned h   = img.height();
            unsigned wm  = w - 1;
            unsigned div = radius * 2 + 1;

            unsigned div_sum = (radius + 1) * (radius + 1);
            unsigned mul_sum = 0;
            unsigned shr_sum = 0;
            unsigned max_val = color_type::base_mask;

            if(max_val <= 255 && radius < 255)
            {
                mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[radius];
                shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[radius];
            }

			// parallelize...
			parallel_for.run_and_wait( 0, h, [&]( int pf_begin, int pf_end ) {

            unsigned xp;
            unsigned stack_ptr;
            unsigned stack_start;

            color_type      pix;
            color_type*     stack_pix;
            calculator_type sum;
            calculator_type sum_in;
            calculator_type sum_out;

            pod_vector<color_type> m_buf;
            pod_vector<color_type> m_stack;

            m_buf.allocate(w, 128);
            m_stack.allocate(div, 32);

            for( int y = pf_begin; y < pf_end; y++)
            {
                sum.clear();
                sum_in.clear();
                sum_out.clear();

                pix = img.pixel(0, y);
                for( int i = 0; i <= radius; i++)
                {
                    m_stack[i] = pix;
                    sum.add(pix, i + 1);
                    sum_out.add(pix);
                }
                for( int i = 1; i <= radius; i++)
                {
                    pix = img.pixel((i > wm) ? wm : i, y);
                    m_stack[i + radius] = pix;
                    sum.add(pix, radius + 1 - i);
                    sum_in.add(pix);
                }

                stack_ptr = radius;
                for( int x = 0; x < w; x++)
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

			}, override_num_threads );
        }

        //--------------------------------------------------------------------
		template <typename Img, typename ParallelFor>
		void blur( Img & img, ParallelFor & parallel_for, unsigned radius, int override_num_threads )
        {
            blur_x(img, parallel_for, radius, override_num_threads );
            pixfmt_transposer<Img> img2(img);
            blur_x(img2, parallel_for, radius, override_num_threads );
        }
    };

    //====================================================stack_blur_calc_rgba
    template<class T=unsigned> struct stack_blur_calc_rgba
    {
        typedef T value_type;
        value_type r,g,b,a;

        AGG_INLINE void clear() 
        { 
            r = g = b = a = 0; 
        }

        template<class ArgT> AGG_INLINE void add(const ArgT& v)
        {
            r += v.r;
            g += v.g;
            b += v.b;
            a += v.a;
        }

        template<class ArgT> AGG_INLINE void add(const ArgT& v, unsigned k)
        {
            r += v.r * k;
            g += v.g * k;
            b += v.b * k;
            a += v.a * k;
        }

        template<class ArgT> AGG_INLINE void sub(const ArgT& v)
        {
            r -= v.r;
            g -= v.g;
            b -= v.b;
            a -= v.a;
        }

        template<class ArgT> AGG_INLINE void calc_pix(ArgT& v, unsigned div)
        {
            typedef typename ArgT::value_type value_type;
            v.r = value_type(r / div);
            v.g = value_type(g / div);
            v.b = value_type(b / div);
            v.a = value_type(a / div);
        }

        template<class ArgT> 
        AGG_INLINE void calc_pix(ArgT& v, unsigned mul, unsigned shr)
        {
            typedef typename ArgT::value_type value_type;
            v.r = value_type((r * mul) >> shr);
            v.g = value_type((g * mul) >> shr);
            v.b = value_type((b * mul) >> shr);
            v.a = value_type((a * mul) >> shr);
        }
    };


    //=======================================================stack_blur_rgba32
	template <typename Img, typename ParallelFor>
	void stack_blur_rgba32( Img & img, ParallelFor & parallel_for, int radius, int override_num_threads )
    {
        if ( radius <= 0 ) return;

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

        if ( radius > 254 ) radius = 254;

        int stride = img.stride();
        unsigned div = radius * 2 + 1;

        unsigned mul_sum = stack_blur_tables<int>::g_stack_blur8_mul[radius];
        unsigned shr_sum = stack_blur_tables<int>::g_stack_blur8_shr[radius];

        {
			// parallelize...
			parallel_for.run_and_wait( 0, h, [&]( int pf_begin, int pf_end ) {

            unsigned xp, yp;
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

            for( int y = pf_begin; y < pf_end; y++)
            {
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
                for( int i = 0; i <= radius; i++)
                {
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
                for( int i = 1; i <= radius; i++)
                {
                    if(i <= wm) src_pix_ptr += Img::pix_width; 
                    stack_pix_ptr = &stack[i + radius];
                    stack_pix_ptr->r = src_pix_ptr[R];
                    stack_pix_ptr->g = src_pix_ptr[G];
                    stack_pix_ptr->b = src_pix_ptr[B];
                    stack_pix_ptr->a = src_pix_ptr[A];
                    sum_r           += src_pix_ptr[R] * (radius + 1 - i);
                    sum_g           += src_pix_ptr[G] * (radius + 1 - i);
                    sum_b           += src_pix_ptr[B] * (radius + 1 - i);
                    sum_a           += src_pix_ptr[A] * (radius + 1 - i);
                    sum_in_r        += src_pix_ptr[R];
                    sum_in_g        += src_pix_ptr[G];
                    sum_in_b        += src_pix_ptr[B];
                    sum_in_a        += src_pix_ptr[A];
                }

                stack_ptr = radius;
                xp = radius;
                if(xp > wm) xp = wm;
                src_pix_ptr = img.pix_ptr(xp, y);
                dst_pix_ptr = img.pix_ptr(0, y);
                for( int x = 0; x < w; x++)
                {
                    dst_pix_ptr[R] = (sum_r * mul_sum) >> shr_sum;
                    dst_pix_ptr[G] = (sum_g * mul_sum) >> shr_sum;
                    dst_pix_ptr[B] = (sum_b * mul_sum) >> shr_sum;
                    dst_pix_ptr[A] = (sum_a * mul_sum) >> shr_sum;
                    dst_pix_ptr += Img::pix_width;

                    sum_r -= sum_out_r;
                    sum_g -= sum_out_g;
                    sum_b -= sum_out_b;
                    sum_a -= sum_out_a;
       
                    stack_start = stack_ptr + div - radius;
                    if(stack_start >= div) stack_start -= div;
                    stack_pix_ptr = &stack[stack_start];

                    sum_out_r -= stack_pix_ptr->r;
                    sum_out_g -= stack_pix_ptr->g;
                    sum_out_b -= stack_pix_ptr->b;
                    sum_out_a -= stack_pix_ptr->a;

                    if(xp < wm) 
                    {
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

			}, override_num_threads );
        }

        {
			// parallelize...
			parallel_for.run_and_wait( 0, w, [&]( int pf_begin, int pf_end ) {

            unsigned xp, yp;
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

            for( int x = pf_begin; x < pf_end; x++)
            {
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
                for( int i = 0; i <= radius; i++)
                {
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
                for( int i = 1; i <= radius; i++)
                {
                    if(i <= hm) src_pix_ptr += stride; 
                    stack_pix_ptr = &stack[i + radius];
                    stack_pix_ptr->r = src_pix_ptr[R];
                    stack_pix_ptr->g = src_pix_ptr[G];
                    stack_pix_ptr->b = src_pix_ptr[B];
                    stack_pix_ptr->a = src_pix_ptr[A];
                    sum_r           += src_pix_ptr[R] * (radius + 1 - i);
                    sum_g           += src_pix_ptr[G] * (radius + 1 - i);
                    sum_b           += src_pix_ptr[B] * (radius + 1 - i);
                    sum_a           += src_pix_ptr[A] * (radius + 1 - i);
                    sum_in_r        += src_pix_ptr[R];
                    sum_in_g        += src_pix_ptr[G];
                    sum_in_b        += src_pix_ptr[B];
                    sum_in_a        += src_pix_ptr[A];
                }

                stack_ptr = radius;
                yp = radius;
                if(yp > hm) yp = hm;
                src_pix_ptr = img.pix_ptr(x, yp);
                dst_pix_ptr = img.pix_ptr(x, 0);
                for( int y = 0; y < h; y++)
                {
                    dst_pix_ptr[R] = (sum_r * mul_sum) >> shr_sum;
                    dst_pix_ptr[G] = (sum_g * mul_sum) >> shr_sum;
                    dst_pix_ptr[B] = (sum_b * mul_sum) >> shr_sum;
                    dst_pix_ptr[A] = (sum_a * mul_sum) >> shr_sum;
                    dst_pix_ptr += stride;

                    sum_r -= sum_out_r;
                    sum_g -= sum_out_g;
                    sum_b -= sum_out_b;
                    sum_a -= sum_out_a;
       
                    stack_start = stack_ptr + div - radius;
                    if(stack_start >= div) stack_start -= div;

                    stack_pix_ptr = &stack[stack_start];
                    sum_out_r -= stack_pix_ptr->r;
                    sum_out_g -= stack_pix_ptr->g;
                    sum_out_b -= stack_pix_ptr->b;
                    sum_out_a -= stack_pix_ptr->a;

                    if(yp < hm) 
                    {
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

			}, override_num_threads );
        }
    }



    //===========================================================recursive_blur
    template<class ColorT, class CalculatorT> class recursive_blur
    {
    public:
        typedef ColorT color_type;
        typedef CalculatorT calculator_type;
        typedef typename color_type::value_type value_type;
        typedef typename calculator_type::value_type calc_type;

        //--------------------------------------------------------------------
		template <typename Img, typename ParallelFor>
		void blur_x( Img & img, ParallelFor & parallel_for, double radius, int override_num_threads ) {
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
		template <typename Img, typename ParallelFor>
		void blur( Img & img, ParallelFor & parallel_for, double radius, int override_num_threads ) {
            blur_x( img, parallel_for, radius, override_num_threads );
            pixfmt_transposer <Img> img2( img );
            blur_x( img2, parallel_for, radius, override_num_threads );
        }
    };


    //=================================================recursive_blur_calc_rgba
    template<class T=double> struct recursive_blur_calc_rgba
    {
        typedef T value_type;
        typedef recursive_blur_calc_rgba<T> self_type;

        value_type r,g,b,a;

        template<class ColorT> 
        AGG_INLINE void from_pix(const ColorT& c)
        {
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
        AGG_INLINE void to_pix(ColorT& c) const
        {
            typedef typename ColorT::value_type cv_type;
            c.r = (cv_type)uround(r);
            c.g = (cv_type)uround(g);
            c.b = (cv_type)uround(b);
            c.a = (cv_type)uround(a);
        }
    };
}

#endif
