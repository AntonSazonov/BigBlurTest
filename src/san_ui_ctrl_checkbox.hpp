#pragma once

#include <string>
#include <functional>
#include <blend2d.h>
#include "san_ui_ctrl.hpp"

namespace san::ui {

//
// Checkbox
//
class checkbox : public control {
	std::string					m_name;
	std::function <void(bool)>	m_callback;

	static constexpr float		m_font_scale		= .6f;
	BLFont &					m_font;
	BLSize						m_text_size;

	bool						m_is_checked		= false;
	bool						m_is_lbutton_down	= false;
	bool						m_is_hovered		= false;

public:
	template <typename CallbackT>
	checkbox( ui <control> * p_ctx, const BLPoint & pos, const std::string & name, CallbackT && callback, bool initial_state = false )
		: control( p_ctx, { pos.x, pos.y, 0, 0 } )
		, m_name( name )
		, m_callback( std::forward<CallbackT>( callback ) )
		, m_font( p_ctx->font_sans() )
		, m_text_size( fit_to_string( m_font, m_name.c_str(), m_font_scale ) )
		, m_is_checked( initial_state )
	{
		// Expand size...
		m_rect.w += m_text_size.h * 2;
		m_rect.h += m_text_size.h / 2;
	}

	void on_mouse_button( const BLPoint & xy, mouse_button_e button, bool is_pressed ) override {

		// If: button is left && current state is up && previous state is down...
		if ( button == mouse_button_e::left && !is_pressed && m_is_lbutton_down ) {
			m_is_checked = !m_is_checked;
			m_callback( m_is_checked );
		}

		m_is_lbutton_down = is_pressed && button == mouse_button_e::left;
	}

	void on_mouse_enter( const BLPoint & xy ) override { m_is_hovered = true; }
	void on_mouse_leave( const BLPoint & xy ) override { m_is_hovered = false; }

	void draw() override {

		// Check-circle radius
		double expand = m_text_size.h / 2;
		double radius = expand;
		double round  = radius * 2;


		// Rounded rect (background)
		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 32, 32, 32, 255 ) );	// Solid
			m_ctx->setStrokeWidth( 1.5 );						// Stroke
			m_ctx->setStrokeStyle( BLRgba32( 127, 127, 127 ) );
		} else {
			m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 191 ) );	// Solid
			m_ctx->setStrokeWidth( 1.2 );						// Stroke
			m_ctx->setStrokeStyle( BLRgba32( 63, 63, 63 ) );
		}
		m_ctx->fillRoundRect( m_rect, round );
		m_ctx->strokeRoundRect( m_rect, round );


		// Outer circle
		if ( m_is_hovered ) {
			m_ctx->setStrokeStyle( BLRgba32( 255, 255, 255 ) );
		} else {
			m_ctx->setStrokeStyle( BLRgba32( 127, 127, 127 ) );
		}

		double circle_x = m_rect.x + expand + radius;
		double circle_y = m_rect.y + m_rect.h / 2;
		m_ctx->setStrokeWidth( radius / 4 );
		m_ctx->strokeCircle( circle_x, circle_y, radius );

		// Inner circle
		if ( m_is_checked ) {
			m_ctx->setFillStyle( BLRgba32( 255, 255, 255 ) );
			m_ctx->fillCircle( circle_x, circle_y, radius / 2 );
		}


		// Text
		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 255, 255, 255 ) );
		} else {
			m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		double x = m_rect.x +            (m_rect.w / 2 - m_text_size.w / 2) + radius * 2 - expand;
		double y = m_rect.y + m_rect.h - (m_rect.h / 2 - m_text_size.h / 2);
		m_ctx->fill_string( BLPoint( x, y ), m_font, m_name.c_str(), m_font_scale );
	}
}; // class checkbox

} // namespace san::ui
