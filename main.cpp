#include <cstdio>
#include <cstdint>
#include <memory>
#include <chrono>
#include <filesystem>
#include <forward_list>
#include <functional>

#include "sdl_window_rgba.hpp"
#include "sdl_image_loader.hpp"

#include "agg/agg_color_rgba.h"
#include "agg/agg_blur.h"
#include "agg/agg_stack_blur_mt.hpp"
#include "agg/agg_recursive_blur_mt.hpp"

#include "san_parallel_for.hpp"
#include "san_image_view.hpp"
#include "san_agg_image_adaptor.hpp"

#include "san_ui.hpp"
#include "san_ui_ctrl_button.hpp"
#include "san_ui_ctrl_checkbox.hpp"
#include "san_ui_ctrl_text.hpp"

#include "san_stack_blur_naive.hpp"
#include "main_compile_opts.hpp"

#include "san_image_list.hpp"


class app final : public sdl::window_rgba {
	std::shared_ptr <SDL_Surface>	m_backbuffer_copy;	// For scaled image
	san::image_view					m_backbuffer_view;	// View of window's backbuffer
	san::agg_image_adaptor			m_backbuffer_agg;	// Image adaptor for AGG

	san::parallel_for				m_parallel_for;
	san::ui::ui						m_ui;
	san::image_list					m_image_list;

	using agg_stack_blur_t			= agg::stack_blur			<agg::rgba8, agg::stack_blur_calc_rgba<uint32_t>>;
	using agg_stack_blur_mt_t		= agg::stack_blur_mt		<agg::rgba8, agg::stack_blur_calc_rgba<uint32_t>>;

	using agg_recursive_blur_t		= agg::recursive_blur		<agg::rgba8, agg::recursive_blur_calc_rgba<>>;
	using agg_recursive_blur_mt_t	= agg::recursive_blur_mt	<agg::rgba8, agg::recursive_blur_calc_rgba<>>;

	agg_stack_blur_t				m_agg_stack_blur;
	agg_stack_blur_mt_t				m_agg_stack_blur_mt;
	agg_recursive_blur_t			m_agg_recursive_blur;
	agg_recursive_blur_mt_t			m_agg_recursive_blur_mt;

	std::map <std::string, std::function <void(int)>> m_algorithms{
		{ "1. san::stack_blur_naive"       , std::bind( san::stack_blur_naive                                                                             , std::ref( m_backbuffer_view ), std::placeholders::_1 ) },
		{ "2. san::stack_blur_naive_mt"    , std::bind( san::stack_blur_naive_mt                              <san::parallel_for>                         , std::ref( m_backbuffer_view ), std::placeholders::_1, std::ref( m_parallel_for ), 0 ) },
		{ "3. agg::stack_blur_rgba32"      , std::bind( agg::stack_blur_rgba32        <san::agg_image_adaptor>                                            , std::ref( m_backbuffer_agg  ), std::placeholders::_1, std::placeholders::_1 ) },
		{ "4. agg::stack_blur_rgba32_mt"   , std::bind( agg::stack_blur_rgba32_mt     <san::agg_image_adaptor, san::parallel_for>                         , std::ref( m_backbuffer_agg  ), std::placeholders::_1, std::placeholders::_1, std::ref( m_parallel_for ) ) },
		{ "5. agg::stack_blur::blur"       , std::bind( &agg_stack_blur_t       ::blur<san::agg_image_adaptor>,                    m_agg_stack_blur       , std::ref( m_backbuffer_agg  ), std::placeholders::_1 ) },
		{ "6. agg::stack_blur_mt::blur"    , std::bind( &agg_stack_blur_mt_t    ::blur<san::agg_image_adaptor, san::parallel_for>, m_agg_stack_blur_mt    , std::ref( m_backbuffer_agg  ), std::placeholders::_1, std::ref( m_parallel_for ) ) },
		{ "7. agg::recursive_blur::blur"   , std::bind( &agg_recursive_blur_t   ::blur<san::agg_image_adaptor>,                    m_agg_recursive_blur   , std::ref( m_backbuffer_agg  ), std::placeholders::_1 ) },
		{ "8. agg::recursive_blur_mt::blur", std::bind( &agg_recursive_blur_mt_t::blur<san::agg_image_adaptor, san::parallel_for>, m_agg_recursive_blur_mt, std::ref( m_backbuffer_agg  ), std::placeholders::_1, std::ref( m_parallel_for ) ) },
	};

public:
	app( int width, int height )
		: sdl::window_rgba( width, height, "Blur Test" )
		, m_backbuffer_copy( copy_surface() )
		, m_backbuffer_view( surface() )
		, m_backbuffer_agg( m_backbuffer_view )
		, m_ui( m_backbuffer_view, 36 )
	{
		set_title( (std::to_string( m_parallel_for.num_threads() ) + " threads available").c_str() );

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


		const int th = 24;
		int h = m_backbuffer_view.height() - 5;
		m_ui.add_control( new san::ui::textbox ( { 10, double(h-=th) }, "Compile options: " + std::string( g_compile_options ) ) );
		m_ui.add_control( new san::ui::textbox ( { 10, double(h-=th) }, "       Compiler: " + std::string( g_compiler ) ) );
		m_ui.add_control( new san::ui::textbox ( { 10, double(h-=th) }, "     Build type: " + std::string( g_build_type ) ) );
		m_ui.add_control( new san::ui::textbox ( { 10, double(h-=th) }, "Use arrays <- and -> to change image." ) );

		// Add UI algorithms buttons...
		int y = 10;
		for ( const auto & p : m_algorithms ) {
			m_ui.add_control( new san::ui::button( { 10, double(y) }, p.first, [&]{ printf( "Benchmark start...\n" ); start_benchmark( p ); } ) );
			y += 40;
		}

		// TODO: checkbox
		//m_ui.add_control( new san::ui::checkbox( { 20, 200 }, "Bench on original size image", [&]( bool value ){ m_bench_on_original_size = value; }, false ) );
	}


	//int m_mouse_x = -1;
	//int m_mouse_y = -1;

	bool on_event( uint64_t timestamp, SDL_Event * p_event ) override {

		// Forward events to UI...
		m_ui.on_event( timestamp, p_event );

		switch ( p_event->type ) {
			case SDL_MOUSEMOTION:
				//m_mouse_x = p_event->motion.x;
				//m_mouse_y = p_event->motion.y;
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

	clock_type::time_point	m_bench_start;

	int		m_bench_time_ms		= 10'000;
	int		m_bench_raises;
	int		m_bench_radius;
	int		m_bench_interations;
	std::function <void(int)>	m_bench_func;

	template <typename PairT>
	void start_benchmark( PairT pair ) {

		m_bench_func = pair.second;
		printf( "Benchmarking (%d ms.): %s \n", m_bench_time_ms, pair.first.c_str() );

		m_bench_raises		= 1;
		m_bench_radius		= 0;
		m_bench_interations	= 1;

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

		if ( m_is_benchmarking ) {

			// Blur window's surface
			m_bench_func( m_bench_radius );

			if ( m_bench_raises ) {
				if ( m_bench_radius++ >= 254 ) m_bench_raises ^= 1;
			} else {
				if ( --m_bench_radius <=   0 ) m_bench_raises ^= 1;
			}

			// Get time elapsed...
			double ms = std::chrono::duration_cast<std::chrono::milliseconds>( clock_type::now() - m_bench_start ).count();
			if ( ms >= m_bench_time_ms ) {

				//m_mouse_x = m_bench_radius;
				set_wait_mode( true );
				m_is_benchmarking = false;
				SDL_EventState( SDL_MOUSEMOTION    , SDL_ENABLE );
				SDL_EventState( SDL_MOUSEBUTTONDOWN, SDL_ENABLE );
				SDL_EventState( SDL_MOUSEBUTTONUP  , SDL_ENABLE );

				// 100 frames
				// 1000 ms
				double sec = ms / 1000;
				double fps = m_bench_interations / sec;
				printf( "Benchmark done. %4d iterations in %5.2f ms. %5.2f FPS. %" PRIu64 " pixels/sec.\n",
					m_bench_interations, ms, fps, uint64_t(width() * height() * fps) );

				// To update display...
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
