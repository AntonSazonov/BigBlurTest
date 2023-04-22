#pragma once

#include <forward_list>
#include <functional>

#include "agg/agg_color_rgba.h"
#include "agg/agg_blur.h"

// Common line adaptor for naive implementations
#include "san_blur_line_adaptor.hpp"

// Gaussian blur naive impl.
#include "san_blur_gaussian_naive.hpp"

// Lookup tables common for all stack blur impls.
#include "san_blur_stack_luts.hpp"

// Naive stack blur impl.
#include "san_blur_stack_naive_calc.hpp"
#include "san_blur_stack_naive.hpp"

// SIMD stack blur impls.
#include "san_blur_stack_simd_calc.hpp"
#include "san_blur_stack_simd_naive.hpp"

// Optimized versions with LUTs
#include "san_blur_stack_simd_optimized_1.hpp"
#include "san_blur_stack_simd_optimized_2.hpp"


namespace san {

#define EMPLACE_IMPL_FUNCT( name, image, func )																		\
		m_impls.emplace_front( name,																				\
			std::bind( func, std::ref( image ), std::ref( a_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

// Class must have function 'blur( ImageViewT & image, ParallelForT & parallel_for, int radius, int override_num_threads )'
#define EMPLACE_IMPL_CLASS( name, image, inst )																										\
		m_impls.emplace_front( name,																												\
			std::bind(																																\
				&decltype(inst)::template blur<std::remove_reference_t<decltype(image)>, std::remove_reference_t<decltype(a_parallel_for)>>, inst,	\
				std::ref( image ), std::ref( a_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

template <typename FuncT>
class impls_list {
	std::forward_list <std::pair<std::string, FuncT>> m_impls;

	agg::stack_blur <agg::rgba8, agg::stack_blur_calc_rgba<uint32_t>>		m_agg_stack_blur;
	agg::recursive_blur	<agg::rgba8, agg::recursive_blur_calc_rgba<double>>	m_agg_recursive_blur;

	san::blur::gaussian::naive_test <254>									m_gaussian_naive;


	using simd_calc_sse2	= san::blur::stack::simd::sse128_u32_t<2>;
	using simd_calc_sse41	= san::blur::stack::simd::sse128_u32_t<41>;

	san::blur::stack::simd::optimized_1 <simd_calc_sse41>					m_san_opt_1;
	san::blur::stack::simd::optimized_2 <simd_calc_sse41>					m_san_opt_2;

public:
	impls_list(
		san::cpu::features & cpu_features,
		san::surface_view & surface_view_san,
		san::agg_image_adaptor & surface_view_agg,
		san::parallel_for & a_parallel_for )
	{

		EMPLACE_IMPL_FUNCT( "agg::stack_blur_rgba32",							surface_view_agg, (agg::stack_blur_rgba32<san::agg_image_adaptor, san::parallel_for>) )
		EMPLACE_IMPL_CLASS( "agg::stack_blur",									surface_view_agg, m_agg_stack_blur )
		EMPLACE_IMPL_CLASS( "agg::recursive_blur",								surface_view_agg, m_agg_recursive_blur )
		EMPLACE_IMPL_CLASS( "san::blur::gaussian::naive (2-pass, fails)",		surface_view_san, m_gaussian_naive )
		EMPLACE_IMPL_FUNCT( "san::blur::stack::naive",							surface_view_san, (san::blur::stack::naive<san::blur::stack::naive_calc<>, san::parallel_for>) )

		if ( cpu_features.SSE2() ) {
			EMPLACE_IMPL_FUNCT( "san::blur::stack::simd::naive (SSE2)",			surface_view_san, (san::blur::stack::simd::naive<simd_calc_sse2 , san::parallel_for>) )
		}

		if ( cpu_features.SSE41() ) {
			EMPLACE_IMPL_FUNCT( "san::blur::stack::simd::naive (SSE4.1)",		surface_view_san, (san::blur::stack::simd::naive<simd_calc_sse41, san::parallel_for>) )

			EMPLACE_IMPL_CLASS( "san::blur::stack::simd::optimized_1 (SSE4.1)",	surface_view_san, m_san_opt_1 )
			EMPLACE_IMPL_CLASS( "san::blur::stack::simd::optimized_2 (SSE4.1)",	surface_view_san, m_san_opt_2 )
		}
	}

	// For 'range-based for' loop...
	auto begin() { return m_impls.begin(); }
	auto end()   { return m_impls.end(); }
}; // class impl_list

#undef EMPLACE_IMPL_CLASS
#undef EMPLACE_IMPL_FUNCT

} // namespace san
