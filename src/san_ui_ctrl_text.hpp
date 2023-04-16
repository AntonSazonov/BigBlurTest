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
	textbox( ui * p_ctx, const BLPoint & pos, const std::string & text )
		: control( p_ctx, { pos.x, pos.y, 0, 0 } )
		, m_text( text ) {}

	void draw() override {

		BLFont & mono = m_ctx->font_mono();
		BLSize   size = m_ctx->string_size( mono, m_text.c_str(), .36f );

		// Control rect.
		double rect_expand = size.h * .7;
		BLRect rect(
			m_rect.x,
			m_rect.y,
			size.w + rect_expand * 2,
			size.h + rect_expand * 2 );

		// Update control's rect size according to text size...
		m_rect = rect;

		// Draw background
		m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 191 ) );
		m_ctx->fillRoundRect( rect, rect_expand /*round radius*/ );

		double x = rect.w / 2 - size.w / 2;
		double y = rect.h / 2 - size.h / 2;
		m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		m_ctx->fill_string( BLPoint( rect.x + x, rect.y + rect.h - y ), mono, m_text.c_str(), .36f );
	}
}; // class textbox

} // namespace san::ui
