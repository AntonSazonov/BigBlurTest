#pragma once

#include <string>
#include <functional>
#include <blend2d.h>

namespace san::ui {

class ui;

#if 0
enum class alignment : uint8_t {
	left	= 0,
	right,
	top,
	bottom,
	center
}; // enum class alignment
#endif

class control {
	control( const control & ) = delete;
	control & operator = ( const control & ) = delete;

protected:
	bool		m_visible	= true;
	//alignment	m_align_h	= alignment::left;
	//alignment	m_align_v	= alignment::top;
	BLRect		m_rect;

public:
	control( const BLRect & rect ) : m_rect( rect ) {
		m_rect.x -= .5;
		m_rect.y -= .5;
	}

	virtual ~control() {}

	virtual void on_event( uint64_t /*timestamp*/, SDL_Event * ) = 0;
	virtual void draw( ui & ) /*const*/ = 0;

	void move_rel( const BLPoint & rel ) {
		m_rect.x += rel.x;
		m_rect.y += rel.y;
	}

	BLRect get_rect() const { return m_rect; }

	BLBox get_bbox() const {
		return BLBox(
			m_rect.x,
			m_rect.y,
			m_rect.x + m_rect.w - 1,
			m_rect.y + m_rect.h - 1 );
	}

	bool is_point_inside( const BLPoint & pt ) const {
		return get_bbox().contains( pt );
	}

	bool is_visible() const { return m_visible; }

	void show() { m_visible =  true; }
	void hide() { m_visible = false; }
}; // control

} // namespace san::ui
