#pragma once

#include <list>
#include <blend2d.h>
//#include "san_ui_console.hpp"

namespace san::ui {

enum class mouse_button_e : uint8_t {
	left	= SDL_BUTTON_LEFT,
	middle	= SDL_BUTTON_MIDDLE,
	right	= SDL_BUTTON_RIGHT,
	x1		= SDL_BUTTON_X1,
	x2		= SDL_BUTTON_X2
}; // enum class mouse_button_e


template <typename ControlBaseT>
class ui : public BLContext {
	double					m_font_size;
	BLPoint					m_scale			= { 1, 1 };

	BLImage 				m_image;

	BLFontFace				m_face_fawf;	// FontAwesome-WebFont
	BLFontFace				m_face_sans;
	BLFontFace				m_face_mono;

	BLFont					m_font_fawf;	// FontAwesome-WebFont
	BLFont					m_font_sans;
	BLFont					m_font_mono;

	//console					m_console;
	std::list <ControlBaseT *>	m_controls;
	ControlBaseT *				m_hover			= nullptr;	// control under mouse pointer
	ControlBaseT *				m_focus			= nullptr;	// active control

	bool load_font( BLFontFace & face, const std::string & name ) {
		if ( face.createFromFile( name.c_str() ) ) {
			std::fprintf( stderr, "Could not create font '%s'.\n", name.c_str() );
			return false;
		}
		return true;
	}

	ControlBaseT * find_first_by_point( const BLPoint & xy ) {
		// Process controls in reverse order...
		for ( auto it = m_controls.rbegin(), end = m_controls.rend(); it != end; ++it ) {
			ControlBaseT * p_ctl = *it;
			if ( p_ctl->is_point_inside( xy ) ) {
				return p_ctl;
			}
		}
		return nullptr;
	}

public:
	ui( san::image_view & image, const std::string & path_fonts, double font_size = 32. ) : m_font_size( font_size ) {
		m_image.createFromData( image.width(), image.height(), BL_FORMAT_PRGB32 /*BL_FORMAT_XRGB32*/, image.ptr(), image.stride() );

		load_font( m_face_fawf, path_fonts + "/fontawesome-webfont.ttf" );
		load_font( m_face_sans, path_fonts + "/NotoSans-Regular.ttf" );
		load_font( m_face_mono, path_fonts + "/SourceCodePro-Medium.otf" );

		m_font_fawf.createFromFace( m_face_fawf, m_font_size );
		m_font_sans.createFromFace( m_face_sans, m_font_size );
		m_font_mono.createFromFace( m_face_mono, m_font_size );
	}

	virtual ~ui() {
		for ( ControlBaseT * p_ctl : m_controls ) delete p_ctl;
	}

	operator BLImage & () { return m_image; }

//	const class console & console() const { return m_console; }
//	      class console & console()       { return m_console; }

	template <typename CtlT, typename ... Args>
	ControlBaseT * add( Args && ... args ) {
		ControlBaseT * p_ctl = new (std::nothrow) CtlT( this, std::forward<Args>( args )... );
		if ( p_ctl ) m_controls.push_back( p_ctl );
		return p_ctl;
	}

	void on_event( const SDL_Event * const p_event ) {

		switch ( p_event->type ) {

			case SDL_MOUSEBUTTONDOWN: [[fallthrough]];
			case SDL_MOUSEBUTTONUP: {

				// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent
				// button - SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT, SDL_BUTTON_X1, SDL_BUTTON_X2
				//  state - SDL_PRESSED or SDL_RELEASED
				//   x, y
				const SDL_MouseButtonEvent * const p_mbe = &p_event->button;

				BLPoint xy( p_mbe->x, p_mbe->y );
				ControlBaseT * p_ctl = find_first_by_point( xy );
				if ( p_ctl ) {
					m_focus = p_ctl; // Set focus on control

					// Move control to top (end of list)...
					//m_controls.erase( std::next( it ).base() );
					//m_controls.push_back( p_ctl );

					p_ctl->on_mouse_button( xy, mouse_button_e(p_mbe->button), p_mbe->state == SDL_PRESSED );
					break;
				}
			} break;

			case SDL_MOUSEMOTION: {

				// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent
				const SDL_MouseMotionEvent * const p_mme = &p_event->motion;

				BLPoint xy( p_mme->x, p_mme->y );
				ControlBaseT * p_ctl = find_first_by_point( xy );
				if ( p_ctl ) {
					if ( p_ctl != m_hover ) {

						// In case of overlapped controls...
						if ( m_hover ) {
							m_hover->on_mouse_leave( xy );
						}

						// Enter another control...
						p_ctl->on_mouse_enter( xy );
						m_hover = p_ctl;
					}

					// p_mbe->state - button state mask
					// SDL_BUTTON_LMASK
					// SDL_BUTTON_MMASK
					// SDL_BUTTON_RMASK
					// SDL_BUTTON_X1MASK
					// SDL_BUTTON_X2MASK
					p_ctl->on_mouse_motion( xy );
					break;
				}

				// ...
				// Pointer doesn't hits any control...
				// ...

				// Leave last hovered...
				if ( m_hover ) {
					m_hover->on_mouse_leave( xy );
					m_hover = nullptr;
				}
			} break;
		}
	}

	void set_scale( const BLPoint & scale ) {
		m_scale = scale;
	}

	const BLImage & image() const { return m_image; }
	      BLImage & image()       { return m_image; }

	BLFont & font_fawf() { return m_font_fawf; }
	BLFont & font_sans() { return m_font_sans; }
	BLFont & font_mono() { return m_font_mono; }

	BLSize string_size( BLFont & font, const char * str, float size_scale ) {

		float prev_size = font.size();
		font.setSize( prev_size * size_scale );

		BLGlyphBuffer gb;
		gb.setUtf8Text( str );

		BLTextMetrics tm;
		font.getTextMetrics( gb, tm );

		BLFontMetrics fm = font.metrics();

		font.setSize( prev_size ); // Restore font size

		return { tm.boundingBox.x1 - tm.boundingBox.x0 + 1, fm.capHeight };
	}

	void fill_string( const BLPoint & xy, BLFont & font, const char * str, float size_scale ) {
		float prev_size = font.size();
		font.setSize( prev_size * size_scale );
		fillUtf8Text( xy, font, str );
		font.setSize( prev_size ); // Restore font size
	}

	void fill_text( const BLPoint & xy, BLFont & font, const char * text, float size_scale ) {
		float prev_size = font.size();
		font.setSize( prev_size * size_scale );

		BLPoint pen = xy;
		for ( const char * p_line_begin = text; *p_line_begin; ) {

			// Search EOL...
			const char * p_line_end = p_line_begin;
			while ( *p_line_end && *p_line_end != '\n' ) ++p_line_end;
			std::ptrdiff_t line_len = p_line_end - p_line_begin;

			fillUtf8Text( pen, font, p_line_begin, line_len );
#if 1
			strokeUtf8Text( pen, font, p_line_begin, line_len );
#endif

			if ( !*p_line_end ) break; // check for '\0'
			p_line_begin += line_len + 1;

			pen.x = xy.x;
			pen.y += font.size() * 1.2f;
		}

		font.setSize( prev_size ); // Restore font size
	}

	void draw() {
		BLContext::begin( m_image );

		// BLResult BLContext::save()
		// BLResult BLContext::restore()
		BLContext::scale( m_scale.x, m_scale.y );

		for ( ControlBaseT * p_ctl : m_controls ) {
			if ( p_ctl->is_visible() ) {
				p_ctl->draw();
			}
		}

		// Test
		//BLFont & fawf = font_fawf();
		//fill_string( { 350, 350 }, fawf, "\xef\x81\xae", 2 );
		//fill_string( { 450, 350 }, fawf, "\xef\x81\xb0", 2 );

#if 0
		// Test
	const char * test_text = R"(
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

		setFillStyle( BLRgba32( 255, 255, 255 ) );
		setStrokeStyle( BLRgba32( 0, 0, 0 ) );
		setStrokeWidth( .5f );
		fill_text( { 350, 100 }, font_sans(), test_text, .75f );
#endif

		BLContext::flush( BL_CONTEXT_FLUSH_SYNC ); // This actually synchronizes.
		BLContext::end();

		//m_console.set_cursor( { 10, 10 } );
		//m_console.draw( this );
	}
}; // class ui

} // namespace san::ui
