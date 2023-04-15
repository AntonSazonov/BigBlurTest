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
	bool						m_is_rbutton_down	= false;
	bool						m_is_hovered		= false;

public:
	template <typename CallbackT>
	checkbox( const BLPoint & pos, const std::string & name, CallbackT && callback, bool initial_state = false )
		: control( { pos.x, pos.y, 0, 0 } )
		, m_name( name )
		, m_callback( std::forward<CallbackT>( callback ) )
		, m_is_checked( initial_state ) {}

	void on_event( uint64_t timestamp, const SDL_Event * const p_event ) override {
		switch ( p_event->type ) {

			case SDL_MOUSEBUTTONDOWN: [[fallthrough]];
			case SDL_MOUSEBUTTONUP: {
				const SDL_MouseButtonEvent * const p = &p_event->button;

				bool is_inside = is_point_inside( { double(p->x), double(p->y) } );
				bool is_down   = p_event->type == SDL_MOUSEBUTTONDOWN;

				// If left button is unpressed by the event and was pressed before...
				if ( p->button == SDL_BUTTON_LEFT && is_inside && !is_down && m_is_lbutton_down ) {
					m_is_checked = !m_is_checked;
					m_callback( m_is_checked );
				}

				if ( p->button == SDL_BUTTON_LEFT  ) m_is_lbutton_down = is_down && is_inside;
				if ( p->button == SDL_BUTTON_RIGHT ) m_is_rbutton_down = is_down && is_inside;

			} break;

			case SDL_MOUSEMOTION: {
				const SDL_MouseMotionEvent * const p = &p_event->motion;
				m_is_hovered = is_point_inside( { double(p->x), double(p->y) } );
			} break;
		}
	}

	void draw( ui & ctx ) /*const*/ override {

		BLFont &	font_sans = ctx.font_sans();
		BLSize		text_size = ctx.get_string_size( font_sans, m_name.c_str() );

		// Check radius
		double radius = text_size.h / 2;

		// Control rect.
		double rect_expand = text_size.h / 2;
		BLRect rect(
			m_rect.x,
			m_rect.y,
			text_size.w + rect_expand * 2 + radius * 2,
			text_size.h + rect_expand * 2 );

		// Update control's rect size according to text size...
		m_rect = rect;

		//double rect_center_x = rect.x + rect.w / 2;
		double rect_center_y = rect.y + rect.h / 2;


		// Draw background
		ctx.setFillStyle( BLRgba32( 0, 0, 0, 159 ) );
		ctx.fillRoundRect( rect, rect_expand /*round radius*/ );

		if ( m_is_hovered ) {
			ctx.setStrokeStyle( BLRgba32( 255, 255, 255 ) ); // hovered
		} else {
			ctx.setStrokeStyle( BLRgba32( 127, 127, 127 ) ); // unhovered
		}
#if 0
		ctx.setStrokeWidth( 1.5 );
		ctx.strokeRoundRect( rect, rect_expand /*round radius*/ );
#endif

		// Draw check circle
		if ( m_is_checked ) {
			ctx.setFillStyle( BLRgba32( 255, 255, 255 ) );
			ctx.fillCircle( rect.x + radius + rect_expand, rect_center_y, radius / 2 );
		}
		ctx.setStrokeWidth( radius / 4 );
		ctx.strokeCircle( rect.x + radius + rect_expand, rect_center_y, radius );

		// Draw text
		if ( m_is_hovered ) {
			ctx.setFillStyle( BLRgba32( 255, 255, 255 ) );
		} else {
			ctx.setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		double x = rect.w / 2 - text_size.w / 2;
		double y = rect.h / 2 - text_size.h / 2;
		ctx.fillUtf8Text( BLPoint( rect.x + x + radius * 2 - rect_expand, rect.y + rect.h - y ), font_sans, m_name.c_str() );
	}
}; // class checkbox

} // namespace san::ui
