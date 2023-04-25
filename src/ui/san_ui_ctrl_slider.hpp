#pragma once

namespace san::ui {

template <typename T>
class clamped_value {
	T &	m_value;
	T	m_min;
	T	m_max;

public:
	clamped_value( T & value ) : m_value( value ) {}

	void set_bounds( T min, T max ) {
		m_min  = min;
		m_max  = max;
	}

	void operator = ( const T & value ) {
		m_value = value < m_min ? m_min :
				 (value > m_max ? m_max : value);
	}

	void map_from( T value, T min, T max ) {
		T norm = (value - min) / (max - min); // [min_x;max_x] --> [0;1]
		m_value = (m_max - m_min) * norm + m_min;
	}

	T map_to( T min, T max ) const {
		T norm = (m_value - m_min) / (m_max - m_min); // [m_min_x;m_max_x] --> [0;1]
		return (max - min) * norm + min;
	}
}; // class clamped_value


class slider : public control {
//	bool						m_is_lbutton_down	= false;
	bool						m_is_hovered		= false;

	std::function <void(float)>	m_callback;

//	std::string					m_name;
//	static constexpr float		m_font_scale		= .6f;
//	BLFont &					m_font;
//	BLSize						m_text_size;

	float						m_half_h;		// Half of height
	BLRect						m_midline;

	float						m_value_min;
	float						m_value_max;

	BLCircle								m_spot;
	clamped_value <decltype(BLCircle::cx)>	m_spot_x;

	bool									m_spot_drag		= false;
	float									m_spot_drag_init_x;

	bool hit_spot( const BLPoint & pt ) const {
		float dx = pt.x - m_spot.cx;
		float dy = pt.y - m_spot.cy;
		return std::sqrt( dx * dx + dy * dy ) <= m_spot.r;
	}

public:
	template <typename CallbackT>
	slider( ui <control> * p_ctx, const BLRect & rect/*, const std::string & name*/, CallbackT && callback, float value, float min, float max )
		: control( p_ctx, rect )
		, m_callback( std::forward<CallbackT>( callback ) )
		//, m_name( name )
		//, m_font( p_ctx->font_sans() )
		//, m_text_size( fit_to_string( m_font, m_name.c_str(), m_font_scale ) )
		, m_value_min( min )
		, m_value_max( max )
		, m_spot_x( m_spot.cx )
	{
		// This is the base unit of size
		m_half_h = m_rect.h / 2;

		m_midline = m_rect;
		m_midline.w -= m_half_h * 1.75;
		m_midline.h -= m_half_h * 1.75;
		m_midline.x += m_half_h - m_midline.h / 2;
		m_midline.y += m_half_h - m_midline.h / 2;

		m_midline.x += 2;
		m_midline.w -= 4;

		m_spot_x.set_bounds( m_midline.x, m_midline.x + m_midline.w );
		m_spot_x.map_from( value, min, max );

		m_spot.cy = m_rect.y + m_half_h;
		m_spot.r  = m_half_h / 1.75;

		m_callback( value );
	}

	void on_mouse_button( const BLPoint & pt, mouse_button_e button, bool is_pressed ) override {
		if ( control::is_inside( pt ) ) {
			m_spot_drag = button == mouse_button_e::left && is_pressed && hit_spot( pt );
			if ( m_spot_drag ) m_spot_drag_init_x = pt.x - m_spot.cx;
		}

		if ( button == mouse_button_e::left && !is_pressed && m_spot_drag ) {
			//printf( "LB released\n" );
			m_spot_drag = false;
		}
	}

	void on_mouse_motion( const BLPoint & pt ) override {
		if ( m_spot_drag ) {
			m_spot_x = pt.x - m_spot_drag_init_x; // clamp
			m_callback( m_spot_x.map_to( m_value_min, m_value_max ) );
		}
	}

	void on_mouse_enter( const BLPoint & ) override { m_is_hovered = true; }
	void on_mouse_leave( const BLPoint & ) override { m_is_hovered = false; }

	void draw() override {

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
		m_ctx->fillRoundRect( m_rect, m_half_h );
		m_ctx->strokeRoundRect( m_rect, m_half_h );

		// Midline
		m_ctx->setFillStyle( BLRgba32( 127, 127, 127, 127 ) );
		m_ctx->fillRoundRect( m_midline, m_half_h * .1 );

		// Draw spot
		m_ctx->setFillStyle( BLRgba32( 255, 255, 255 ) );
		m_ctx->fillCircle( m_spot );

#if 0
		// Text
		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 255, 255, 255 ) );
		} else {
			m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		double x = m_rect.x +            (m_rect.w / 2 - m_text_size.w / 2) + radius * 2 - expand;
		double y = m_rect.y + m_rect.h - (m_rect.h / 2 - m_text_size.h / 2);
		m_ctx->fill_string( BLPoint( x, y ), m_font, m_name.c_str(), m_font_scale );
#endif
	}
}; // class slider

} // namespace san::ui
