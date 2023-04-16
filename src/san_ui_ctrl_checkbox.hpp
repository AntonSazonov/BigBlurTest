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

	bool						m_is_checked		= false;
	bool						m_is_lbutton_down	= false;
	bool						m_is_hovered		= false;

public:
	template <typename CallbackT>
	checkbox( ui * p_ctx, const BLPoint & pos, const std::string & name, CallbackT && callback, bool initial_state = false )
		: control( p_ctx, { pos.x, pos.y, 0, 0 } )
		, m_name( name )
		, m_callback( std::forward<CallbackT>( callback ) )
		, m_is_checked( initial_state ) {}

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

		BLFont & sans = m_ctx->font_sans();
		BLSize   size = m_ctx->string_size( sans, m_name.c_str(), .6f );

		// Check radius
		double radius = size.h / 2;

		// Control rect.
		double rect_expand = size.h / 2;
		BLRect rect(
			m_rect.x,
			m_rect.y,
			size.w + rect_expand * 2 + radius * 2,
			size.h + rect_expand * 2 );

		// Update control's rect size according to text size...
		m_rect = rect;

		//double rect_center_x = rect.x + rect.w / 2;
		double rect_center_y = rect.y + rect.h / 2;


		// Draw background
		m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 159 ) );
		//m_ctx->fillRoundRect( rect, rect_expand /*round radius*/ );
		m_ctx->fillRoundRect( rect, radius * 2 );

		if ( m_is_hovered ) {
			m_ctx->setStrokeStyle( BLRgba32( 255, 255, 255 ) ); // hovered
		} else {
			m_ctx->setStrokeStyle( BLRgba32( 127, 127, 127 ) ); // unhovered
		}

		// Draw check circle
		if ( m_is_checked ) {
			m_ctx->setFillStyle( BLRgba32( 255, 255, 255 ) );
			m_ctx->fillCircle( rect.x + radius + rect_expand, rect_center_y, radius / 2 );
		}
		m_ctx->setStrokeWidth( radius / 4 );
		m_ctx->strokeCircle( rect.x + radius + rect_expand, rect_center_y, radius );

		// Draw text
		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 255, 255, 255 ) );
		} else {
			m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		double x = rect.w / 2 - size.w / 2;
		double y = rect.h / 2 - size.h / 2;
		m_ctx->fill_string( BLPoint( rect.x + x + radius * 2 - rect_expand, rect.y + rect.h - y ), sans, m_name.c_str(), .6f );
	}
}; // class checkbox

} // namespace san::ui
