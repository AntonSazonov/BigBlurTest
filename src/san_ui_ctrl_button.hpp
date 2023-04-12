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
	bool					m_is_rbutton_down	= false;
	bool					m_is_hovered		= false;

public:
	template <typename CallbackT>
	button( const BLPoint & pos, const std::string & name, CallbackT && callback )
		: control( { pos.x, pos.y, 0, 0 } )
		, m_name( name )
		, m_callback( std::forward<CallbackT>( callback ) ) {}

	void on_event( uint64_t timestamp, SDL_Event * p_event ) override {
		switch ( p_event->type ) {

			case SDL_MOUSEBUTTONDOWN: [[fallthrough]];
			case SDL_MOUSEBUTTONUP: {
				SDL_MouseButtonEvent * p = &p_event->button;

				bool is_inside = is_point_inside( { double(p->x), double(p->y) } );
				bool is_down   = p_event->type == SDL_MOUSEBUTTONDOWN;

				// If left button is unpressed by the event and was pressed before...
				if ( p->button == SDL_BUTTON_LEFT && is_inside && !is_down && m_is_lbutton_down ) {
					m_callback();
				}

				if ( p->button == SDL_BUTTON_LEFT  ) m_is_lbutton_down = is_down && is_inside;
				if ( p->button == SDL_BUTTON_RIGHT ) m_is_rbutton_down = is_down && is_inside;

			} break;

			case SDL_MOUSEMOTION: {
				SDL_MouseMotionEvent * p = &p_event->motion;
				m_is_hovered = is_point_inside( { double(p->x), double(p->y) } );

#if 0
				// Move control by right mouse button
				if ( m_is_rbutton_down ) {
					move_rel( { double(p->xrel), double(p->yrel) } );
				}
#endif
			} break;
		}
	}

	void draw( ui & ctx ) /*const*/ override {

		BLFont & sans = ctx.font_sans();
		float prev_size = sans.size();
		sans.setSize( prev_size * .6f );

		BLSize size = ctx.get_string_size( sans, m_name.c_str() );


		double expand = size.h / 2;
		BLRect rect( m_rect.x, m_rect.y, size.w + expand * 2, size.h + expand * 2 );
		//BLRect rect = m_rect;
		m_rect = rect;


		//ctx.setFillStyle( BLRgba32( 0, 0, 0, 159 ) );

		if ( m_is_hovered ) {
			// Solid
			ctx.setFillStyle( BLRgba32( 0, 0, 0, 255 ) );
			ctx.fillRoundRect( rect, expand );

			// Stroke
			//ctx.setStrokeStyle( BLRgba32( 127, 255, 191 ) ); // hovered
			ctx.setStrokeStyle( BLRgba32( 255, 255, 255 ) ); // hovered
			ctx.setStrokeWidth( 1.5 );
			ctx.strokeRoundRect( rect, expand );

#if 0
			ctx.setStrokeStyle( BLRgba32( 0, 0, 0 ) );
			ctx.setStrokeWidth( 1. );
			ctx.strokeRoundRect( { rect.x - 2, rect.y - 2, rect.w + 4, rect.h + 4 }, expand );
#endif
		} else {
			// Solid
			ctx.setFillStyle( BLRgba32( 0, 0, 0, 191 ) );
			ctx.fillRoundRect( rect, expand );

#if 1
			// Stroke
			ctx.setStrokeStyle( BLRgba32( 63, 63, 63 ) ); // unhovered
			ctx.setStrokeWidth( 1.2 );
			ctx.strokeRoundRect( rect, expand );
#endif
		}


		//ctx.draw_text( BLPoint( m_rect.x + 10, m_rect.y + 50 ), m_name.c_str() );

		if ( m_is_hovered ) {
			ctx.setFillStyle( BLRgba32( 255, 255, 255 ) );
		} else {
			ctx.setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		// b = 200
		// t = 100
		double x = rect.w / 2 - size.w / 2;
		double y = rect.h / 2 - size.h / 2;
		ctx.fillUtf8Text( BLPoint( rect.x + x, rect.y + rect.h - y ), sans, m_name.c_str() );

		// Restore font size
		sans.setSize( prev_size );

#if 0
		BLGlyphBuffer gb;
		gb.setUtf8Text( m_name.c_str() );

		BLTextMetrics tm;
		sans.getTextMetrics( gb, tm );

//		std::printf( "text width = %f, height = %f\n",
//			tm.boundingBox.x1 - tm.boundingBox.x0 + 1, sans.size() );

		ctx.fillGlyphRun( BLPoint( m_rect.x + 10, m_rect.y + 50 ), sans, gb.glyphRun() );
#endif
	}
}; // class button

} // namespace san::ui
