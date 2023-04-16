#include <cstdio>
#include <cstdint>
#include <memory>
#include <chrono>

#include "sdl_window_rgba.hpp"
#include "src/stb_impl.hpp"

#include "agg/agg_color_rgba.h"
#include "agg/agg_blur.h"

#include "san_image_list.hpp"
#include "san_impl_list.hpp"
#include "san_parallel_for.hpp"
#include "san_image_view.hpp"
#include "san_agg_image_adaptor.hpp"

#include "san_ui.hpp"
#include "san_ui_ctrl_button.hpp"
#include "san_ui_ctrl_checkbox.hpp"
#include "san_ui_ctrl_text.hpp"
#include "san_ui_ctrl_link.hpp"

// Lookup tables common for all
#include "san_stack_blur_luts.hpp"

// Naive impl.
#include "san_stack_blur_naive_calc.hpp"
#include "san_stack_blur_naive.hpp"

// SIMD impls.
#include "san_stack_blur_simd_calc.hpp"			// Common for SIMD calculator versions (bit scan reverse etc...).
#include "san_stack_blur_simd.hpp"				// Blur impl.

#include "san_stack_blur_simd_fastest.hpp"		// Optimization tests...
#include "san_stack_blur_simd_fastest_2.hpp"

#include "main_compile_opts.hpp"

//#include "san_recursive.hpp"
//#include "san_test_sb.hpp"
//#include "san_test_naive.hpp"

class app final : public sdl::window_rgba {
	std::shared_ptr <SDL_Surface>	m_backbuffer_copy;	// For scaled image
	san::image_view					m_backbuffer_view;	// View of window's backbuffer
	san::agg_image_adaptor			m_backbuffer_agg;	// Image adaptor for AGG

	san::parallel_for				m_parallel_for;
	san::ui::ui						m_ui;

	san::image_list					m_image_list;		// Loaded image list in its' original sizes

	// Implementation list
	// Function params.: 'radius', '# of threads' or 0 - max threads from 'parallel_for'
	san::impl_list <std::function <void(int, int)>>	m_impls;

	using agg_stack_blur_t				= agg::stack_blur		<agg::rgba8, agg::stack_blur_calc_rgba<uint32_t>>;
	using agg_recursive_blur_t			= agg::recursive_blur	<agg::rgba8, agg::recursive_blur_calc_rgba<double>>;

	agg_stack_blur_t					m_agg_stack_blur;
	agg_recursive_blur_t				m_agg_recursive_blur;

public:
	app( int width, int height )
		: sdl::window_rgba( width, height, "Blur Test" )
		, m_backbuffer_copy( copy_surface() )
		, m_backbuffer_view( surface() )
		, m_backbuffer_agg( m_backbuffer_view )
		, m_ui( m_backbuffer_view, "./fonts", 36 )
	{
		// Console
		//san::ui::console & con = m_ui.console();
		//con.add_command( "Exit", "Exit program.", [&](){ quit(); } );

		// At least one image must be loaded
		if ( !m_image_list ) {
			quit();
			return;
		}

		// Blit current image...
		blit_scaled( m_image_list.current_image().get(), m_backbuffer_copy.get() );

		// AGG
		m_impls.emplace(
			"agg::stack_blur_rgba32",
			std::bind( agg::stack_blur_rgba32<san::agg_image_adaptor, san::parallel_for>,
			std::ref( m_backbuffer_agg  ), std::ref( m_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

		m_impls.emplace(
			"agg::stack_blur::blur",
			std::bind( &agg_stack_blur_t::blur<san::agg_image_adaptor, san::parallel_for>, m_agg_stack_blur,
			std::ref( m_backbuffer_agg  ), std::ref( m_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

		m_impls.emplace(
			"agg::recursive_blur::blur",
			std::bind( &agg_recursive_blur_t::blur<san::agg_image_adaptor, san::parallel_for>, m_agg_recursive_blur,
			std::ref( m_backbuffer_agg  ), std::ref( m_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

		// My
		m_impls.emplace(
			"san::stack_blur::naive",
			std::bind( san::stack_blur::naive<san::stack_blur::naive_calc, san::parallel_for>,
			std::ref( m_backbuffer_view ), std::ref( m_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

#ifdef __SSE2__
		m_impls.emplace(
			"san::stack_blur::simd::blur (SSE2)",
			std::bind( san::stack_blur::simd::blur<san::stack_blur::simd::calculator::sse128_u32_t<2>, san::parallel_for>,
			std::ref( m_backbuffer_view ), std::ref( m_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );
#endif // __SSE2__

#ifdef __SSE4_1__
		m_impls.emplace(
			"san::stack_blur::simd::blur (SSE4.1)",
			std::bind( san::stack_blur::simd::blur<san::stack_blur::simd::calculator::sse128_u32_t<41>, san::parallel_for>,
			std::ref( m_backbuffer_view ), std::ref( m_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );
#endif // __SSE4_1__



		namespace sss = san::stack_blur::simd;

#ifdef __SSE4_1__
		using fast_calc = sss::calculator::sse128_u32_t<41>;
#else
		using fast_calc = sss::calculator::sse128_u32_t<2>;
#endif
		using sss_fastest_t		= sss::fastest::blur_impl<fast_calc>;
		using sss_fastest_2_t	= sss::fastest_2::blur_impl<fast_calc>;

		sss_fastest_t			m_san_sb_fastest;
		sss_fastest_2_t			m_san_sb_fastest_2;

		m_impls.emplace(
			"san::stack_blur::simd::fastest::blur_impl",
			std::bind( &sss_fastest_t::blur<san::image_view, san::parallel_for>, m_san_sb_fastest,
			std::ref( m_backbuffer_view ), std::ref( m_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );

		m_impls.emplace(
			"san::stack_blur::simd::fastest::blur_impl_2",
			std::bind( &sss_fastest_2_t::blur<san::image_view, san::parallel_for>, m_san_sb_fastest_2,
			std::ref( m_backbuffer_view ), std::ref( m_parallel_for ), std::placeholders::_1, std::placeholders::_2 ) );



		const int th = 24;
		int h = m_backbuffer_view.height() - 5;

		m_ui.add<san::ui::link>   ( BLPoint{ 10, double(h-=th) }, "https://github.com/AntonSazonov/Blur_Test" );
		m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "Compile options: " + std::string( g_compile_options ) );
		m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "       Compiler: " + std::string( g_compiler ) );
		m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "     Build type: " + std::string( g_build_type ) );
		m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "        Threads: " + std::to_string( m_parallel_for.num_threads() ) );
		m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "Use arrays <- and -> to change image." );

		// Add UI algorithms buttons...
		int y = 10;
		for ( const auto & p : m_impls ) {
			m_ui.add<san::ui::button>( BLPoint{ 10, double(y) }, p.first, [&]{ std::printf( "Benchmark start...\n" ); start_benchmark( p ); } );
			y += 40;
		}

		// Test checkbox...
		//m_ui.add<san::ui::checkbox>( BLPoint{ 350, 250 }, "Bench on original size image", [&]( bool value ){ printf( "Checkbox: %d\n", int(value) ); /*m_bench_on_original_size = value;*/ }, false );
	}


	int m_mouse_x = -1;
	int m_mouse_y = -1;

	bool on_event( const SDL_Event * const p_event ) override {

		// Forward events to UI...
		m_ui.on_event( p_event );

		switch ( p_event->type ) {
			case SDL_MOUSEMOTION:
				m_mouse_x = p_event->motion.x;
				m_mouse_y = p_event->motion.y;
				break;

			case SDL_KEYDOWN:
				switch ( p_event->key.keysym.sym ) {
					case SDLK_ESCAPE:	return false; // Exit on 'Escape'

					case SDLK_LEFT:		[[fallthrough]];
					case SDLK_RIGHT:
						if ( p_event->key.keysym.sym == SDLK_LEFT ) {
							m_image_list.go_prev();
						} else {
							m_image_list.go_next();
						}
						blit_scaled( m_image_list.current_image().get(), m_backbuffer_copy.get() );
						break;
				}
				break;
		}
		return true; // true - continue event loop, false - exit
	}


	bool	m_is_benchmarking = false;

	using clock_type = std::chrono::high_resolution_clock;

	clock_type::time_point			m_bench_start;
	int								m_bench_time_ms		= 10'000;
	int								m_bench_raises;
	int								m_bench_radius;
	int								m_bench_interations;
	std::string						m_bench_name;
	std::function <void(int, int)>	m_bench_func;

	template <typename PairT>
	void start_benchmark( const PairT & pair ) {

		m_bench_name = pair.first;
		m_bench_func = pair.second;
		std::printf( "Benchmarking (%d ms.): %s \n", m_bench_time_ms, m_bench_name.c_str() );

		m_bench_raises		= 1;
		m_bench_radius		= 0;
		m_bench_interations	= 1;

		SDL_EventState( SDL_KEYDOWN        , SDL_DISABLE );
		SDL_EventState( SDL_KEYUP          , SDL_DISABLE );
		SDL_EventState( SDL_MOUSEMOTION    , SDL_DISABLE );
		SDL_EventState( SDL_MOUSEBUTTONDOWN, SDL_DISABLE );
		SDL_EventState( SDL_MOUSEBUTTONUP  , SDL_DISABLE );

		set_wait_mode( false );
		m_is_benchmarking = true;
		m_bench_start = clock_type::now();
	}

	void on_frame() override {

		// Copy image to window's surface
		blit( m_backbuffer_copy.get() );

#if 1
		if ( !m_is_benchmarking ) {
			//san::stack_blur::naive<san::stack_blur::naive_calc>( m_backbuffer_view, m_mouse_x );

			namespace	sss			= san::stack_blur::simd;
			using		sse_calc	= sss::calculator::sse128_u32_t<2>;
			sss::blur<sse_calc>( m_backbuffer_view, m_parallel_for, m_mouse_x, 1/*threads*/ );

			//agg::recursive_blur <agg::rgba8, agg::recursive_blur_calc_rgba<float>> rbf;
			//agg::recursive_blur <agg::rgba8, san::recursive_blur_calc_rgba<double>> rbf;
			//rbf.blur( m_backbuffer_agg, m_mouse_x );

			//san::stack_blur::simd::fastest::blur_impl fb;
			//fb.blur( m_backbuffer_view, m_mouse_x );


			// Test impl.
			//san::test::blur_impl fb;
			//san::test::naive::blur_impl fb;
			//fb.blur( m_backbuffer_view, m_mouse_x );

#if 0
			double r = m_mouse_x * .1;
			fprintf( stderr, "\r%5.2f", r );
			san::recursive_blur::blur rb;
			//rb.blur_x<san::agg_image_adaptor>( m_backbuffer_agg, r );
			rb.blur_x( m_backbuffer_view, r );
#endif
		}
#endif

		if ( m_is_benchmarking ) {

			// Blur window's surface
			m_bench_func( m_bench_radius, 0 );

			if ( m_bench_raises ) {
				if ( m_bench_radius++ >= 254 ) {
					m_bench_raises ^= 1;
				}
			} else {
				if ( --m_bench_radius <=   0 ) {
					m_bench_raises ^= 1;
#if 0
					m_image_list.go_next(); // Go next image...
					blit_scaled( m_image_list.current_image().get(), m_backbuffer_copy.get() );
#endif
				}
			}

			// Get time elapsed...
			double ms = std::chrono::duration_cast<std::chrono::milliseconds>( clock_type::now() - m_bench_start ).count();
			if ( ms >= m_bench_time_ms ) {

				//m_mouse_x = m_bench_radius;
				set_wait_mode( true );
				m_is_benchmarking = false;
				SDL_EventState( SDL_KEYDOWN        , SDL_ENABLE );
				SDL_EventState( SDL_KEYUP          , SDL_ENABLE );
				SDL_EventState( SDL_MOUSEMOTION    , SDL_ENABLE );
				SDL_EventState( SDL_MOUSEBUTTONDOWN, SDL_ENABLE );
				SDL_EventState( SDL_MOUSEBUTTONUP  , SDL_ENABLE );

				double sec = ms / 1e3;
				double fps = m_bench_interations / sec;

				std::printf( "%s --- done. %4d iterations in %5.2f sec., %6.2f FPS, ~%.1f ms/frame, %u MPixels/s.\n",
					m_bench_name.c_str(), m_bench_interations, sec, fps, ms / m_bench_interations, uint32_t(width() * height() * fps / 1e6) );

				// To update window...
				SDL_Event ev = {};
				ev.type = SDL_DISPLAYEVENT;
				SDL_PushEvent( &ev );
			}
			m_bench_interations++;
		}

		if ( !m_is_benchmarking ) m_ui.draw();
	}
}; // class app

int main() {
	app a( 1280, 720 );
	if ( a ) {
		a.show();
		a.run();
	}
	return 0;
}
