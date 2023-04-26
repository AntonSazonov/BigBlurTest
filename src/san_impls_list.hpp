#pragma once

namespace san {

#define EMPLACE_IMPL_FUNCT( name, image, func )																					\
		m_impls.emplace_front( name,																							\
			std::bind( func, std::ref( image ), std::ref( a_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

// Class must have function 'blur( ImageViewT & image, ParallelForT & parallel_for, float radius, int override_num_threads )'
#define EMPLACE_IMPL_CLASS( name, image, inst )																												\
		static_assert( std::is_trivially_copyable_v<decltype(inst)> );																						\
		m_impls.emplace_front( name,																														\
			std::bind(																																		\
				&decltype(inst)::template operator () <std::remove_reference_t<decltype(image)>, std::remove_reference_t<decltype(a_parallel_for)>>, inst,	\
				std::ref( image ), std::ref( a_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

template <typename FuncT>
class impls_list {
	std::forward_list <std::pair<std::string, FuncT>> m_impls;

	using simd_calc_sse2	= san::blur::stack::simd::sse128_u32_t<2>;
	using simd_calc_sse41	= san::blur::stack::simd::sse128_u32_t<41>;

	san::blur::stack::simd::optimized_1 <simd_calc_sse41>					m_san_opt_1;
	san::blur::stack::simd::optimized_2 <simd_calc_sse41>					m_san_opt_2;

	agg::stack_blur <agg::rgba8, agg::stack_blur_calc_rgba<uint32_t>>		m_agg_stack_blur;
	agg::recursive_blur	<agg::rgba8, agg::recursive_blur_calc_rgba<double>>	m_agg_recursive_blur;

	san::blur::gaussian::naive_test <256, float>							m_gaussian_naive;
	san::blur::recursive::naive <>											m_recursive_naive;

public:
	impls_list(
		const san::cpu_info & cpu_info,
		san::surface_view & surface_view_san,
		san::adaptor::agg_image & surface_view_agg,
		san::parallel_for & a_parallel_for )
	{

		EMPLACE_IMPL_FUNCT( "agg::stack_blur_rgba32",							surface_view_agg, (agg::stack_blur_rgba32<san::adaptor::agg_image, san::parallel_for>) )
		EMPLACE_IMPL_CLASS( "agg::stack_blur",									surface_view_agg, m_agg_stack_blur )
		EMPLACE_IMPL_CLASS( "agg::recursive_blur",								surface_view_agg, m_agg_recursive_blur )
		EMPLACE_IMPL_CLASS( "san::blur::gaussian::naive",						surface_view_san, m_gaussian_naive )
		EMPLACE_IMPL_CLASS( "san::blur::recursive::naive",						surface_view_san, m_recursive_naive )
		EMPLACE_IMPL_FUNCT( "san::blur::stack::naive",							surface_view_san, (san::blur::stack::naive<san::blur::stack::naive_calc<>, san::parallel_for>) )

		if ( cpu_info.sse2() ) {
			EMPLACE_IMPL_FUNCT( "san::blur::stack::simd::naive (SSE2)",			surface_view_san, (san::blur::stack::simd::naive<simd_calc_sse2 , san::parallel_for>) )
		}

		if ( cpu_info.sse41() ) {
			EMPLACE_IMPL_FUNCT( "san::blur::stack::simd::naive (SSE4.1)",		surface_view_san, (san::blur::stack::simd::naive<simd_calc_sse41, san::parallel_for>) )
			EMPLACE_IMPL_CLASS( "san::blur::stack::simd::optimized_1 (SSE4.1)",	surface_view_san, m_san_opt_1 )
			EMPLACE_IMPL_CLASS( "san::blur::stack::simd::optimized_2 (SSE4.1)",	surface_view_san, m_san_opt_2 )
		}
	}

	// For 'range-based for' loop...
	auto begin() { return m_impls.begin(); }
	auto end()   { return m_impls.end(); }

#if 0
//std::optional<int> opt;
//std::cout << opt.has_value() << '\n';

	std::optional <int> opt find_by_name() {
		for ( const auto & impl : m_impls ) {
			impl.first;
		}
		//std::function <void(int, int)>	m_bench_func;
		return;
	}
#endif
}; // class impl_list

#undef EMPLACE_IMPL_CLASS
#undef EMPLACE_IMPL_FUNCT

} // namespace san
