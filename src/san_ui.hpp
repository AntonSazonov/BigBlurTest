#pragma once

//#define SDL_MAIN_HANDLED
//#include <SDL.h>

#include <list>
#include <blend2d.h>
#include "san_ui_console.hpp"
#include "san_ui_ctrl.hpp"
//#include "san_parallel_for.hpp"
//#include "san_stack_blur_simd.hpp"

namespace san::ui {

class ui : public BLContext {
	double					m_font_size;

	BLImage 				m_image;
	BLFontFace				m_face_sans;
	BLFontFace				m_face_mono;
	BLFont					m_font_sans;
	BLFont					m_font_mono;

	//console					m_console;
	std::list <control *>	m_controls;

	bool load_font( BLFontFace & face, const char * name ) {
		if ( face.createFromFile( name ) ) {
			std::fprintf( stderr, "Could not create font '%s'.\n", name );
			return false;
		}
		return true;
	}

public:

	//ui( san::image_view & surface, double font_size = 32. ) : m_font_size( font_size ) {

	template <typename WindowType>
	ui( WindowType & window, double font_size = 32. ) : m_font_size( font_size ) {
		m_image.createFromData( window.width(), window.height(), BL_FORMAT_XRGB32, window.ptr(), window.stride() );

		load_font( m_face_sans, "./fonts/NotoSans-Regular.ttf" );
		load_font( m_face_mono, "./fonts/SourceCodePro-Medium.otf" );

		m_font_sans.createFromFace( m_face_sans, m_font_size );
		m_font_mono.createFromFace( m_face_mono, m_font_size );
	}

	virtual ~ui() {
		for ( control * p_ctl : m_controls ) delete p_ctl;
	}

//	const class console & console() const { return m_console; }
//	      class console & console()       { return m_console; }

	template <typename CtlT, typename ... Args>
	control * add( Args && ... args ) {
		control * p_ctl = new (std::nothrow) CtlT( std::forward<Args>(args)... );
		if ( p_ctl ) m_controls.push_back( p_ctl );
		return p_ctl;
	}

	void on_event( uint64_t timestamp, const SDL_Event * const p_event ) {

		// Process controls in reverse order...
		for ( auto it = m_controls.rbegin(), end = m_controls.rend(); it != end; ++it ) {
			control * p_ctl = *it;

			// Don't send mouse events to all controls. Only to first.
			if ( p_event->type == SDL_MOUSEMOTION ) {
				if ( p_ctl->is_point_inside( { double(p_event->motion.x), double(p_event->motion.y) } ) ) {
					p_ctl->on_event( timestamp, p_event );
					break;
				}
			}

			if ( p_event->type == SDL_MOUSEBUTTONDOWN ||
				 p_event->type == SDL_MOUSEBUTTONUP )
			{
				if ( p_ctl->is_point_inside( { double(p_event->button.x), double(p_event->button.y) } ) ) {

					// Move control to top (end of list)...
					m_controls.erase( std::next( it ).base() );
					m_controls.push_back( p_ctl );

					p_ctl->on_event( timestamp, p_event );
					break;
				}
			}

			p_ctl->on_event( timestamp, p_event );
		}
	}

	const BLImage & image() const { return m_image; }
	      BLImage & image()       { return m_image; }

	BLFont & font_sans() { return m_font_sans; }
	BLFont & font_mono() { return m_font_mono; }

	BLSize get_string_size( const BLFont & font, const char * str ) {
		BLGlyphBuffer gb;
		gb.setUtf8Text( str );

		BLTextMetrics tm;
		font.getTextMetrics( gb, tm );

		BLFontMetrics fm = font.metrics();
		//std::printf( "capHeight = %f, size = %f, xHeight = %f\n", fm.capHeight, fm.size, fm.xHeight );
		return { tm.boundingBox.x1 - tm.boundingBox.x0 + 1, fm.capHeight };
	}

#if 0
	BLPoint m_pen_origin;
	BLPoint m_pen;

	void set_pen( const BLPoint & pen ) {
		m_pen_origin = m_pen = pen;
	}

	const char * m_test_text = R"(
This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
)";

	void test_text() {

		//BLSize size = get_string_size( m_font_sans, m_test_text );

		BLGlyphBuffer gb;
		gb.setUtf8Text( m_test_text );

		BLTextMetrics tm;
		m_font_sans.getTextMetrics( gb, tm );

		//BLFontMetrics fm = m_font_sans.metrics();
		//std::printf( "capHeight = %f, size = %f, xHeight = %f\n", fm.capHeight, fm.size, fm.xHeight );
		//return { tm.boundingBox.x1 - tm.boundingBox.x0 + 1, fm.capHeight };

		std::printf( "test_text: w = %f, h = %f\n",
			tm.boundingBox.x1 - tm.boundingBox.x0 + 1,
			tm.boundingBox.y1 - tm.boundingBox.y0 + 1 );
		return;

		//const BLFontMetrics& metrics() const;
		//LOG( "lineGap = %f\n", m_font_sans.metrics().lineGap );
		//LOG( "size = %f\n", m_font_sans.size() );

		//const char * text = "Привет 123 Blend2D!\nasdpoj123-8";
		const char * p1 = m_test_text;

		while ( *p1 ) {
			const char * p2 = p1;
			while ( *p2 && *p2 != '\n' ) ++p2;
			std::ptrdiff_t len = p2 - p1;

#if 0
			m_ctx.setFillStyle( BLRgba32( 255, 255, 255 ) );
			m_ctx.fillUtf8Text( m_pen, m_font_sans, p1, len );

			m_ctx.setStrokeStyle( BLRgba32( 0, 0, 0 ) );
			m_ctx.setStrokeWidth( m_font_sans.size() / 64.f );
			m_ctx.strokeUtf8Text( m_pen, m_font_sans, p1, len );
#endif

			if ( !*p2 ) break; // check for '\0'
			p1 += len + 1;

			m_pen.x = m_pen_origin.x;
			m_pen.y += m_font_sans.size() * 1.2f;
		}
	}
#endif

	void draw() {
		BLContext::begin( m_image );

		for ( control * p_ctl : m_controls ) {

			//BLBox bbox = c->get_bbox();
			//BLRect rect = c->get_rect();

			//sdl::surface_view surface_view( m_surface, int(rect.x), int(rect.y), int(rect.w), int(rect.h) );
			//m_san_stack_blur_simd.blur( surface_view, 8, 8, m_parallel_for );

			if ( p_ctl->is_visible() ) {
				p_ctl->draw( *this );
			}
		}

		BLContext::flush( BL_CONTEXT_FLUSH_SYNC ); // This actually synchronizes.
		BLContext::end();

		//m_console.set_cursor( { 10, 10 } );
		//m_console.draw( this );
	}
}; // class ui

} // namespace san::ui
