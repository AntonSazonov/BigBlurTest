
#include "san_pch.hpp"

#include "san_cmake_config.hpp"
#include "san_cpu_features.hpp"

#include "stb_impl.hpp"
#include "san_surface.hpp"


#include "platform/san_platform.hpp"

#ifdef SAN_PLATFORM_WINDOWS
 #include "platform/san_window_win32.hpp"
#endif

#include "san_blur_line_adaptor.hpp"			// Common line adaptor for naive implementations
#include "san_blur_gaussian_naive.hpp"			// Gaussian blur naive impl.
#include "san_blur_stack_luts.hpp"				// Lookup tables common for all stack blur impls.

#include "san_blur_stack_naive_calc.hpp"		// Naive stack blur impl.
#include "san_blur_stack_naive.hpp"

#include "san_blur_stack_simd_calc.hpp"			// SIMD stack blur impls.
#include "san_blur_stack_simd_naive.hpp"

#include "san_blur_stack_simd_optimized_1.hpp"	// Optimized versions with LUTs
#include "san_blur_stack_simd_optimized_2.hpp"

#include "san_agg_image_adaptor.hpp"
#include "san_parallel_for.hpp"

#include "san_image_list.hpp"
#include "san_impls_list.hpp"

#include <blend2d.h>
#include "ui/san_ui.hpp"
#include "ui/san_ui_ctrl.hpp"
#include "ui/san_ui_ctrl_button.hpp"
#include "ui/san_ui_ctrl_checkbox.hpp"
#include "ui/san_ui_ctrl_text.hpp"
#include "ui/san_ui_ctrl_link.hpp"
#include "ui/san_ui_ctrl_slider.hpp"


class app final : public san::window {
	san::cpu::features				m_cpu_features;

	std::shared_ptr <san::surface>	m_backbuffer_copy;	// For scaled image

	san::surface_view				m_surface_view_san;	// View of window's surface
	san::agg_image_adaptor			m_surface_view_agg;	// Image view adaptor for AGG implementations

	san::parallel_for				m_parallel_for;
	san::ui::ui <san::ui::control>	m_ui;

	san::image_list					m_image_list;		// Loaded image list in its' original sizes

	// Implementation list.
	// Function params.: 'radius', '# of threads' or 0 - max. available threads from 'parallel_for'.
	san::impls_list <std::function <void(int, int)>>	m_impls;

public:
	app( int width, int height )
		: san::window( width, height, "Big Blur Test" )
		, m_backbuffer_copy ( san::window::get_surface_copy() )
		, m_surface_view_san( san::window::get_surface_view() )
		, m_surface_view_agg( m_surface_view_san )
		, m_ui( m_surface_view_san, "./fonts", 36 )
		, m_impls( m_cpu_features, m_surface_view_san, m_surface_view_agg, m_parallel_for )
	{
		// Console
		//san::ui::console & con = m_ui.console();
		//con.add_command( "Exit", "Exit program.", [&](){ san::window::quit(); } );

		// At least one image must be loaded
		if ( !m_image_list ) {
			san::window::quit();
			return;
		}

		// Blit current image...
		m_image_list.current_image()->blit_to( m_backbuffer_copy );

		{ // Add some text...
			const int th = 20;
			int h = m_surface_view_san.height() - 5;
			m_ui.add<san::ui::link>   ( BLPoint{ 10, double(h-=th) }, "https://github.com/AntonSazonov/BigBlurTest" );
			m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "Compile options: " + san::cmake::compile_opts() );
			m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, " SIMD supported: " + m_cpu_features.SIMD() );
			m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "            CPU: " + m_cpu_features.CPU() );
			m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "       Compiler: " + san::cmake::compiler_id() );
			m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "     Build type: " + san::cmake::build_type() );	
			m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "        Threads: " + std::to_string( m_parallel_for.num_threads() ) );
			m_ui.add<san::ui::textbox>( BLPoint{ 10, double(h-=th) }, "Use arrays <- and -> to change image." );
		}

		// Add UI algorithms buttons...
		int y = 10;
		for ( const auto & p : m_impls ) {
			m_ui.add<san::ui::button>( BLPoint{ 10, double(y) }, p.first, [&]{ std::printf( "Benchmark start...\n" ); start_benchmark( p ); } );
			y += 40;
		}
		//m_ui.add<san::ui::checkbox>( BLPoint{ 10, double(y) }, "Bench on original size image", [&]( bool value ){ printf( "Checkbox: %d\n", int(value) ); /*m_bench_on_original_size = value;*/ }, false );

		m_ui.add<san::ui::slider>( BLRect{ 500, 10, 400, 30 }, []( float value ){ std::printf( "Slider: %.1f\n", value ); }, 16, 0, 64 );
	}


	int m_mouse_x = -1;
	int m_mouse_y = -1;

	void on_mouse_motion( int x, int y ) override {
		m_ui.on_mouse_motion( x, y );
		m_mouse_x = x;
	}

	void on_mouse_button( int x, int y, san::mouse_button_e button, bool pressed ) override {
		m_ui.on_mouse_button( x, y, button, pressed );
	}

	// At the moment, 'vk' is a Windows virtual key code.
	// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	void on_key( int vk, bool pressed ) override {
		switch ( vk ) {
			case VK_ESCAPE:	san::window::quit(); break;

			case VK_LEFT:	[[fallthrough]];
			case VK_RIGHT:
				if ( !pressed ) break;

				if ( vk == VK_LEFT ) {
					m_image_list.go_prev();
				} else {
					m_image_list.go_next();
				}
				m_image_list.current_image()->blit_to( m_backbuffer_copy );
				break;
		}
	}

	bool							m_is_benchmarking	= false;

	double							m_bench_start;
	int								m_bench_time_ms		= 10'000;
	double							m_bench_time_func;

	int								m_bench_radius_raises;
	int								m_bench_radius;
	int								m_bench_interations;
	std::string						m_bench_name;
	std::function <void(int, int)>	m_bench_func;

	template <typename PairT>
	void start_benchmark( const PairT & pair ) {

		m_bench_name = pair.first;
		m_bench_func = pair.second;

		return; // DEBUG!!!

		std::printf( "\nBenchmarking (%d ms.): %s \n", m_bench_time_ms, m_bench_name.c_str() );

		// Disable input while bench.
		san::window::enable_input_events( false );
		san::window::set_wait_events( false );

		m_is_benchmarking		= true;
		m_bench_radius_raises	= 1;
		m_bench_radius			= 0;
		m_bench_interations		= 0;
		m_bench_time_func		= 0;
		m_bench_start			= san::window::time_ms();
	}

	void on_frame() override {

		// Copy image to window's surface
		m_backbuffer_copy->blit_to( m_surface_view_san );

		if ( !m_is_benchmarking ) {
#if 1
#if 1
			//san::blur::gaussian::naive_test <32> gaussian;
			//gaussian.blur( m_surface_view_san, m_parallel_for, m_mouse_x, 0/*max. threads*/ );
			if ( m_bench_func ) m_bench_func( m_mouse_x, 0 );
#else
			agg::recursive_blur	<agg::rgba8, agg::recursive_blur_calc_rgba<double>>	agg_recursive_blur;
			agg_recursive_blur.blur( m_surface_view_agg, m_parallel_for, m_mouse_x, 0/*max. threads*/ );
#endif
#endif
		}

		if ( m_is_benchmarking ) {

			// Blur window's surface
			{
				double time_func_start = san::window::time_us();
				if ( m_bench_func ) {
					m_bench_func( m_bench_radius, 0/* thread count; 0 means max. available */ );
				} else {
					fprintf( stderr, "m_bench_func( %s ) is empty. Terminating...\n", m_bench_name.c_str() );
					san::window::quit();
				}
				m_bench_time_func += san::window::time_us() - time_func_start;
			}

			if ( m_bench_radius_raises ) {
				if ( m_bench_radius++ >= 254 ) {
					m_bench_radius_raises ^= 1;
				}
			} else {
				if ( --m_bench_radius <=   0 ) {
					m_bench_radius_raises ^= 1;

					//m_image_list.go_next(); // Go next image...
					//m_image_list.current_image()->blit_to( m_backbuffer_copy );
				}
			}

			// Get time elapsed...
			if ( san::window::time_ms() - m_bench_start >= m_bench_time_ms ) {

				//m_mouse_x = m_bench_radius;
				m_is_benchmarking = false;

				san::window::set_wait_events( true );
				san::window::enable_input_events( true );

				double sec = m_bench_time_func / 1e6;
				double fps = m_bench_interations / sec;

				std::printf( "%s --- done. %4d iterations in %5.2f sec., %6.2f FPS, ~%.2f ms./frame, %u MPixels/s.\n",
					m_bench_name.c_str(), m_bench_interations, sec, fps, double(m_bench_time_func / 1e3) / m_bench_interations, uint32_t(m_surface_view_san.width() * m_surface_view_san.height() * fps / 1e6) );

				// Update window...
				m_backbuffer_copy->blit_to( m_surface_view_san );
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
