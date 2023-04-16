#pragma once

#include <string>
#include <functional>
#include <blend2d.h>
#include "san_ui_ctrl.hpp"

namespace san::ui {

//
// Ordinary button
//
class button : public control {
	std::string				m_name;
	std::function <void()>	m_callback;

	bool					m_is_lbutton_down	= false;
	//bool					m_is_rbutton_down	= false;
	bool					m_is_hovered		= false;

public:
	template <typename CallbackT>
	button( ui * p_ctx, const BLPoint & pos, const std::string & name, CallbackT && callback )
		: control( p_ctx, { pos.x, pos.y, 0, 0 } )
		, m_name( name )
		, m_callback( std::forward<CallbackT>( callback ) ) {}

	void on_mouse_button( const BLPoint & xy, mouse_button_e button, bool is_pressed ) override {

		// If: button is left && current state is up && previous state is down...
		if ( button == mouse_button_e::left && !is_pressed && m_is_lbutton_down ) {
			m_callback();
		}

		m_is_lbutton_down = is_pressed && button == mouse_button_e::left;
		//m_is_rbutton_down = is_pressed && button == mouse_button_e::right;
	}

	void on_mouse_enter( const BLPoint & xy ) override { m_is_hovered = true; }
	void on_mouse_leave( const BLPoint & xy ) override { m_is_hovered = false; }

	void draw() override {

		BLFont & font = m_ctx->font_sans();
		BLSize   size = m_ctx->string_size( font, m_name.c_str(), .6f );


		double expand = size.h / 2;
		BLRect rect( m_rect.x, m_rect.y, size.w + expand * 2, size.h + expand * 2 );
		m_rect = rect;

		double round_radius = expand * 1.5;
		if ( m_is_hovered ) {

			// Solid
			m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 255 ) );
			m_ctx->fillRoundRect( rect, round_radius );

			// Stroke
			//m_ctx->setStrokeStyle( BLRgba32( 127, 255, 191 ) ); // hovered
			m_ctx->setStrokeStyle( BLRgba32( 255, 255, 255 ) ); // hovered
			m_ctx->setStrokeWidth( 1.5 );
			m_ctx->strokeRoundRect( rect, round_radius );
		} else {
			// Solid
			m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 191 ) );
			m_ctx->fillRoundRect( rect, round_radius );

			// Stroke
			m_ctx->setStrokeStyle( BLRgba32( 63, 63, 63 ) ); // unhovered
			m_ctx->setStrokeWidth( 1.2 );
			m_ctx->strokeRoundRect( rect, round_radius );
		}

		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 255, 255, 255 ) );
		} else {
			m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		double x = rect.w / 2 - size.w / 2;
		double y = rect.h / 2 - size.h / 2;
		m_ctx->fill_string( BLPoint( rect.x + x, rect.y + rect.h - y ), font, m_name.c_str(), .6f );
	}
}; // class button

} // namespace san::ui
