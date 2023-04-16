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
	std::string		m_url;

	bool			m_is_lbutton_down	= false;
	bool			m_is_hovered		= false;

public:
	link( ui * p_ctx, const BLPoint & pos, const std::string & url )
		: control( p_ctx, { pos.x, pos.y, 0, 0 } )
		, m_url( url ) {}

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
		BLFont & mono = m_ctx->font_mono();
		BLSize   size = m_ctx->string_size( mono, m_url.c_str(), .36f );

		// Control rect.
		double rect_expand = size.h * .7;
		BLRect rect(
			m_rect.x,
			m_rect.y,
			size.w + rect_expand * 2,
			size.h + rect_expand * 2 );

		// Fit control bbox to text size
		m_rect = rect;

		// Draw background
		m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 191 ) );
		m_ctx->fillRoundRect( rect, rect_expand /*round radius*/ );

		double x = rect.w / 2 - size.w / 2;
		double y = rect.h / 2 - size.h / 2;

		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 200, 255, 200 ) );
		} else {
			m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		m_ctx->fill_string( BLPoint( rect.x + x, rect.y + rect.h - y ), mono, m_url.c_str(), .36f );
	}
}; // class link

} // namespace san::ui
