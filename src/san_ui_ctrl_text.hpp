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
	std::string				m_text;

	static constexpr float	m_font_scale		= .36f;
	BLFont &				m_font;
	BLSize					m_text_size;

public:
	textbox( ui <control> * p_ctx, const BLPoint & pos, const std::string & text )
		: control( p_ctx, { pos.x, pos.y, 0, 0 } )
		, m_text( text )
		, m_font( p_ctx->font_mono() )
		, m_text_size( fit_to_string( m_font, m_text.c_str(), m_font_scale ) )
	{
		// Expand size...
		const double expand = m_text_size.h * .7;
		m_rect.w += expand * 2;
		m_rect.h += expand;
	}

	void draw() override {

		// Background
		m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 191 ) );
		m_ctx->fillRoundRect( m_rect, m_text_size.h * .7 );

		// Text
		double x = m_rect.w / 2 - m_text_size.w / 2;
		double y = m_rect.h / 2 - m_text_size.h / 2;
		m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		m_ctx->fill_string( BLPoint( m_rect.x + x, m_rect.y + m_rect.h - y ), m_font, m_text.c_str(), .36f );
	}
}; // class textbox

} // namespace san::ui
