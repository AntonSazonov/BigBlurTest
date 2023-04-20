#pragma once

#include <string>
#include <functional>
#include <blend2d.h>

namespace san::ui {

class control {
	control( const control & ) = delete;
	control & operator = ( const control & ) = delete;

protected:
	ui <control> *	m_ctx;
	BLRect			m_rect;
	bool			m_visible	= true;

public:
	control( ui <control> * p_ctx, const BLRect & rect )
		: m_ctx( p_ctx )
		, m_rect( rect )
	{
		m_rect.x -= .5;
		m_rect.y -= .5;
	}

	virtual ~control() {}

	virtual void on_mouse_button( const BLPoint &, mouse_button_e, bool/*pressed?*/ ) {}
	virtual void on_mouse_motion( const BLPoint & ) {}
	virtual void on_mouse_enter( const BLPoint & ) {}
	virtual void on_mouse_leave( const BLPoint & ) {}
	virtual void draw() = 0;

	// Fit control's rect. to string dimentions + small expand
	BLSize fit_to_string( BLFont & font, const char * str, float font_size ) {
		//printf( "fit_to_string(): %f\n", font_size );
		BLSize size = m_ctx->string_size( font, str, font_size );
		m_rect.w = size.w + size.h / 2;
		m_rect.h = size.h + size.h / 2;
		return size; // string size
	}

	BLBox bbox() const {
		return BLBox(
			m_rect.x,
			m_rect.y,
			m_rect.x + m_rect.w - 1,
			m_rect.y + m_rect.h - 1 );
	}

	bool is_point_inside( const BLPoint & pt ) const { 	return bbox().contains( pt ); }
	bool is_visible() const { return m_visible; }

	void show() { m_visible =  true; }
	void hide() { m_visible = false; }
}; // control

} // namespace san::ui
