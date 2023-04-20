#pragma once

#include <forward_list>
#include <functional>

#include "agg/agg_color_rgba.h"
#include "agg/agg_blur.h"

#include "san_line_adaptor.hpp"					// Common line adaptor for naive implementations
#include "san_stack_blur_luts.hpp"				// Lookup tables common for all

// Gaussian blur naive impl.
#include "san_blur_gaussian_naive.hpp"

// Naive impl.
#include "san_stack_blur_naive_calc.hpp"
#include "san_stack_blur_naive.hpp"

// SIMD impls.
#include "san_stack_blur_simd_calc.hpp"			// Common for SIMD calculator versions (bit scan reverse etc...).
#include "san_stack_blur_simd.hpp"				// Blur impl.

#include "san_stack_blur_simd_fastest.hpp"		// Optimization tests...
#include "san_stack_blur_simd_fastest_2.hpp"


namespace san {

#define EMPLACE_IMPL_FUNCT( name, image, func )																		\
		emplace( name, std::bind( func,																				\
				std::ref( image ), std::ref( a_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

// Class must have function 'blur( ImageViewT & image, ParallelFor & parallel_for, int radius, int override_num_threads )'
#define EMPLACE_IMPL_CLASS( name, image, instance )																									\
		emplace( name, std::bind( &decltype(instance)::template blur<std::remove_reference<decltype(image)>::type, san::parallel_for>, instance,	\
				std::ref( image ), std::ref( a_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

template <typename FuncT>
class impl_list {
	std::forward_list <std::pair<std::string, FuncT>> m_impls;

	agg::stack_blur <agg::rgba8, agg::stack_blur_calc_rgba<uint32_t>>		m_agg_stack_blur;
	agg::recursive_blur	<agg::rgba8, agg::recursive_blur_calc_rgba<double>>	m_agg_recursive_blur;

	san::blur::gaussian::naive_test <256>									m_gaussian_naive;


	using fast_calc_sse41 = san::stack_blur::simd::calculator::sse128_u32_t<41>;
	//using fast_calc_sse2  = san::stack_blur::simd::calculator::sse128_u32_t<2>;

	san::stack_blur::simd::fastest  ::blur_impl <fast_calc_sse41>			m_san_sb_fastest;
	san::stack_blur::simd::fastest_2::blur_impl <fast_calc_sse41>			m_san_sb_fastest_2;


public:
	impl_list(
		san::cpu::features & cpu_features,
		san::surface_view & surface_view_san,
		san::agg_image_adaptor & surface_view_agg,
		san::parallel_for & a_parallel_for )
	{

		EMPLACE_IMPL_FUNCT( "agg::stack_blur_rgba32",						surface_view_agg, (agg::stack_blur_rgba32<san::agg_image_adaptor, san::parallel_for>) )
		EMPLACE_IMPL_CLASS( "agg::stack_blur::blur",						surface_view_agg, m_agg_stack_blur )

		EMPLACE_IMPL_CLASS( "agg::recursive_blur::blur",					surface_view_agg, m_agg_recursive_blur )

		EMPLACE_IMPL_CLASS( "san::blur::gaussian::naive",					surface_view_san, m_gaussian_naive )
		EMPLACE_IMPL_FUNCT( "san::stack_blur::naive",						surface_view_san, (san::stack_blur::naive<san::stack_blur::naive_calc<>, san::parallel_for>) )

		if ( cpu_features.SSE2() ) {
			EMPLACE_IMPL_FUNCT( "san::stack_blur::simd::blur (SSE2)",		surface_view_san, (san::stack_blur::simd::blur<san::stack_blur::simd::calculator::sse128_u32_t<2>, san::parallel_for>) )
		}

		if ( cpu_features.SSE41() ) {
			EMPLACE_IMPL_FUNCT( "san::stack_blur::simd::blur (SSE4.1)",		surface_view_san, (san::stack_blur::simd::blur<san::stack_blur::simd::calculator::sse128_u32_t<41>, san::parallel_for>) )
		}

		if ( cpu_features.SSE41() ) {
			EMPLACE_IMPL_CLASS( "san::stack_blur::simd::fastest::blur_impl"  ,	surface_view_san, m_san_sb_fastest )
			EMPLACE_IMPL_CLASS( "san::stack_blur::simd::fastest::blur_impl_2",	surface_view_san, m_san_sb_fastest_2 )
		}
	}

	template <typename ... Args>
	auto emplace( Args && ... args ) {
		return m_impls.emplace_front( std::forward<Args>(args)... );
	}

	// For 'range-based for' loop...
	auto begin() { return m_impls.begin(); }
	auto end()   { return m_impls.end(); }
}; // class impl_list

#undef EMPLACE_IMPL_CLASS
#undef EMPLACE_IMPL_FUNCT

} // namespace san
