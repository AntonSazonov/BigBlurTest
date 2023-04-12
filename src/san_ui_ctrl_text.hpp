#pragma once

#include <string>
#include <functional>
#include <blend2d.h>
#include "san_ui_ctrl.hpp"

namespace san::ui {

//
// Text box
//

// TODO: Support for multiline text

class textbox : public control {
	std::string	m_text;

public:
	textbox( const BLPoint & pos, const std::string & text )
		: control( { pos.x, pos.y, 0, 0 } )
		, m_text( text ) {}

	void on_event( uint64_t timestamp, SDL_Event * p_event ) override {}

	void draw( ui & ctx ) /*const*/ override {

		BLFont & font_mono = ctx.font_mono();

		float prev_size = font_mono.size();
		font_mono.setSize( prev_size * .36f );

		BLSize text_size = ctx.get_string_size( font_mono, m_text.c_str() );

		// Control rect.
		double rect_expand = text_size.h * .7;
		BLRect rect(
			m_rect.x,
			m_rect.y,
			text_size.w + rect_expand * 2,
			text_size.h + rect_expand * 2 );

		// Update control's rect size according to text size...
		m_rect = rect;

		// Draw background
		ctx.setFillStyle( BLRgba32( 0, 0, 0, 191 ) );
		ctx.fillRoundRect( rect, rect_expand /*round radius*/ );

		double x = rect.w / 2 - text_size.w / 2;
		double y = rect.h / 2 - text_size.h / 2;
		ctx.setFillStyle( BLRgba32( 191, 191, 191 ) );
		ctx.fillUtf8Text( BLPoint( rect.x + x, rect.y + rect.h - y ), font_mono, m_text.c_str() );

		// Restore font size
		font_mono.setSize( prev_size );
	}
}; // class textbox

} // namespace san::ui
