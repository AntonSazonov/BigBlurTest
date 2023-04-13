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
	bool			m_is_rbutton_down	= false;
	bool			m_is_hovered		= false;

public:
	link( const BLPoint & pos, const std::string & url )
		: control( { pos.x, pos.y, 0, 0 } )
		, m_url( url ) {}

	void on_event( uint64_t timestamp, SDL_Event * p_event ) override {
		switch ( p_event->type ) {

			case SDL_MOUSEBUTTONDOWN: [[fallthrough]];
			case SDL_MOUSEBUTTONUP: {
				SDL_MouseButtonEvent * p = &p_event->button;

				bool is_inside = is_point_inside( { double(p->x), double(p->y) } );
				bool is_down   = p_event->type == SDL_MOUSEBUTTONDOWN;

				// If left button is unpressed by the event and was pressed before...
				if ( p->button == SDL_BUTTON_LEFT && is_inside && !is_down && m_is_lbutton_down ) {
					//std::system( "start https://github.com/AntonSazonov/Blur_Test" );

#if defined( _WIN32 )
					//std::system( ("start " + m_url).c_str() ); // "start" not works with MSYS2 in my PATH env. var.
					std::system( ("explorer " + m_url).c_str() );
#elif defined( __unix__ )
					// Non tested in Unix-like systems...
					std::system( ("xdg-open " + m_url).c_str() );
#endif // Do nothing on other systems
				}

				if ( p->button == SDL_BUTTON_LEFT  ) m_is_lbutton_down = is_down && is_inside;
				if ( p->button == SDL_BUTTON_RIGHT ) m_is_rbutton_down = is_down && is_inside;

			} break;

			case SDL_MOUSEMOTION: {
				SDL_MouseMotionEvent * p = &p_event->motion;
				m_is_hovered = is_point_inside( { double(p->x), double(p->y) } );
			} break;
		}
	}

	void draw( ui & ctx ) override {
		BLFont & font_mono = ctx.font_mono();

		float prev_size = font_mono.size();
		font_mono.setSize( prev_size * .36f );

		BLSize text_size = ctx.get_string_size( font_mono, m_url.c_str() );

		// Control rect.
		double rect_expand = text_size.h * .7;
		BLRect rect(
			m_rect.x,
			m_rect.y,
			text_size.w + rect_expand * 2,
			text_size.h + rect_expand * 2 );

		// Fit control bbox to text size
		m_rect = rect;

		// Draw background
		ctx.setFillStyle( BLRgba32( 0, 0, 0, 191 ) );
		ctx.fillRoundRect( rect, rect_expand /*round radius*/ );

		double x = rect.w / 2 - text_size.w / 2;
		double y = rect.h / 2 - text_size.h / 2;

		if ( m_is_hovered ) {
			ctx.setFillStyle( BLRgba32( 200, 255, 200 ) );
		} else {
			ctx.setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		ctx.fillUtf8Text( BLPoint( rect.x + x, rect.y + rect.h - y ), font_mono, m_url.c_str() );

		// Restore font size
		font_mono.setSize( prev_size );
	}
}; // class link

} // namespace san::ui
