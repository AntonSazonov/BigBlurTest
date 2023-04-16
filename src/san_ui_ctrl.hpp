#pragma once

#include <string>
#include <functional>
#include <blend2d.h>

namespace san::ui {

enum class mouse_button_e : uint8_t {
	left	= SDL_BUTTON_LEFT,
	middle	= SDL_BUTTON_MIDDLE,
	right	= SDL_BUTTON_RIGHT,
	x1		= SDL_BUTTON_X1,
	x2		= SDL_BUTTON_X2
}; // enum class mouse_button_e


class ui;

class control {
	control( const control & ) = delete;
	control & operator = ( const control & ) = delete;

protected:
	ui *		m_ctx;
	BLRect		m_rect;
	bool		m_visible	= true;

public:
	control( ui * p_ctx, const BLRect & rect )
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

	BLBox bbox() const { return BLBox( m_rect.x, m_rect.y, m_rect.x + m_rect.w - 1, m_rect.y + m_rect.h - 1 ); }

	bool is_point_inside( const BLPoint & pt ) const { 	return bbox().contains( pt ); }
	bool is_visible() const { return m_visible; }

	void show() { m_visible =  true; }
	void hide() { m_visible = false; }
}; // control

} // namespace san::ui
