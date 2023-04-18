#pragma once

#include <string>
#include <functional>
#include <blend2d.h>
#include "san_ui_ctrl.hpp"

namespace san::ui {

//
// URL Link control
//
class link : public control {
	std::string				m_url;

	static constexpr float	m_font_scale		= .36f;
	BLFont &				m_font;
	BLSize					m_text_size;

	bool					m_is_lbutton_down	= false;
	bool					m_is_hovered		= false;

public:
	link( ui <control> * p_ctx, const BLPoint & pos, const std::string & url )
		: control( p_ctx, { pos.x, pos.y, 0, 0 } )
		, m_url( url )
		, m_font( p_ctx->font_mono() )
		, m_text_size( fit_to_string( m_font, m_url.c_str(), m_font_scale ) )
	{
		// Expand size...
		const double expand = m_text_size.h * .7;
		m_rect.w += expand * 2;
		m_rect.h += expand;
	}

	void on_mouse_button( const BLPoint & xy, mouse_button_e button, bool is_pressed ) override {

		// If: button is left && current state is up && previous state is down...
		if ( button == mouse_button_e::left && !is_pressed && m_is_lbutton_down ) {
#if defined( WIN32 )
			//std::system( ("start " + m_url).c_str() ); // "start" not works with MSYS2 in my PATH env. var.
			std::system( ("explorer " + m_url).c_str() );
#elif defined( __unix__ )
			std::system( ("xdg-open " + m_url).c_str() );
#endif // Do nothing on other systems
		}

		m_is_lbutton_down = is_pressed && button == mouse_button_e::left;
	}

	void on_mouse_enter( const BLPoint & xy ) override { m_is_hovered = true; }
	void on_mouse_leave( const BLPoint & xy ) override { m_is_hovered = false; }

	void draw() override {

		// Rect. (background)
		m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 191 ) );
		m_ctx->fillRoundRect( m_rect, m_text_size.h * .7 );

		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 200, 255, 200 ) );
		} else {
			m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		double x = m_rect.x +            (m_rect.w / 2 - m_text_size.w / 2);
		double y = m_rect.y + m_rect.h - (m_rect.h / 2 - m_text_size.h / 2);
		m_ctx->fill_string( BLPoint( x, y ), m_font, m_url.c_str(), .36f );
	}
}; // class link

} // namespace san::ui
