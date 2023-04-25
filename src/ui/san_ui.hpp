#pragma once

namespace san::ui {

template <typename ControlBaseT>
class ui : public BLContext {
	float						m_font_size;
	BLPoint						m_scale			= { 1, 1 };

	BLImage 					m_image;

	BLFontFace					m_face_sans;
	BLFontFace					m_face_mono;

	BLFont						m_font_sans;
	BLFont						m_font_mono;

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

public:
	ui( san::surface_view & image, const std::string & path_fonts, float font_size = 32. ) : m_font_size( font_size ) {
		m_image.createFromData( image.width(), image.height(), BL_FORMAT_PRGB32 /*BL_FORMAT_XRGB32*/, image.ptr(), image.stride() );

		load_font( m_face_sans, path_fonts + "/NotoSans-Regular.ttf" );
		load_font( m_face_mono, path_fonts + "/SourceCodePro-Medium.otf" );

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

	void on_mouse_motion( int x, int y ) {
		BLPoint xy( x, y );

		// Process controls in reverse order...
		for ( auto it = m_controls.rbegin(), end = m_controls.rend(); it != end; ++it ) {
			ControlBaseT * p_ctl = *it;
			bool inside = p_ctl->is_inside( xy );

			if ( m_hover == p_ctl && !inside ) {
				m_hover->on_mouse_leave( xy );
				m_hover = nullptr;
			}

			if ( m_hover != p_ctl && inside ) {
				p_ctl->on_mouse_enter( xy );
				m_hover = p_ctl;

			}

			p_ctl->on_mouse_motion( xy );
		}
	}

	void on_mouse_button( int x, int y, san::mouse_button_e button, bool pressed ) {
		BLPoint xy( x, y );

		// Process controls in reverse order...
		for ( auto it = m_controls.rbegin(), end = m_controls.rend(); it != end; ++it ) {
			ControlBaseT * p_ctl = *it;

			if ( m_focus != p_ctl && p_ctl->is_inside( xy ) ) {
				m_focus = p_ctl; // Set focus on control
			}

			p_ctl->on_mouse_button( xy, button, pressed );
		}
	}

	void set_scale( const BLPoint & scale ) {
		m_scale = scale;
	}

	const BLImage & image() const { return m_image; }
	      BLImage & image()       { return m_image; }

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
		//BLContext::scale( m_scale.x, m_scale.y );

		for ( ControlBaseT * p_ctl : m_controls ) {
			if ( p_ctl->is_visible() ) {
				p_ctl->draw();
			}
		}

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
