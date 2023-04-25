#pragma once

namespace san::ui {

//
// Ordinary button
//
class button : public control {
	std::string				m_name;
	std::function <void()>	m_callback;

	static constexpr float	m_font_scale		= .6f;
	BLFont &				m_font;
	BLSize					m_text_size;

	bool					m_is_lbutton_down	= false;
	//bool					m_is_rbutton_down	= false;
	bool					m_is_hovered		= false;

public:
	template <typename CallbackT>
	button( ui <control> * p_ctx, const BLPoint & pos, const std::string & name, CallbackT && callback )
		: control( p_ctx, { pos.x, pos.y, 0, 0 } )
		, m_name( name )
		, m_callback( std::forward<CallbackT>( callback ) )
		, m_font( p_ctx->font_sans() )
		, m_text_size( fit_to_string( m_font, m_name.c_str(), m_font_scale ) )
	{
		// Expand size...
		m_rect.w += m_text_size.h / 2;
		m_rect.h += m_text_size.h / 2;
	}

	void on_mouse_button( const BLPoint & pt, mouse_button_e button, bool is_pressed ) override {
		if ( !control::is_inside( pt ) ) return;

		// If: button is left && current state is up && previous state is down...
		if ( button == mouse_button_e::left && !is_pressed && m_is_lbutton_down ) {
			m_callback();
		}

		m_is_lbutton_down = is_pressed && button == mouse_button_e::left;
		//m_is_rbutton_down = is_pressed && button == mouse_button_e::right;
	}

	void on_mouse_enter( const BLPoint & ) override { m_is_hovered = true; }
	void on_mouse_leave( const BLPoint & ) override { m_is_hovered = false; }

	void draw() override {

		double expand = m_text_size.h / 2;
		double round = expand * 1.5;

		// Rect.
		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 32, 32, 32, 255 ) );	// Solid
			m_ctx->setStrokeStyle( BLRgba32( 255, 255, 255 ) );	// Stroke
			m_ctx->setStrokeWidth( 1.5 );
		} else {
			m_ctx->setFillStyle( BLRgba32( 0, 0, 0, 191 ) );	// Solid
			m_ctx->setStrokeStyle( BLRgba32( 63, 63, 63 ) );	// Stroke
			m_ctx->setStrokeWidth( 1.2 );
		}
		m_ctx->fillRoundRect( m_rect, round );
		m_ctx->strokeRoundRect( m_rect, round );

		// Text
		if ( m_is_hovered ) {
			m_ctx->setFillStyle( BLRgba32( 255, 255, 255 ) );
		} else {
			m_ctx->setFillStyle( BLRgba32( 191, 191, 191 ) );
		}

		double x = m_rect.w / 2 - m_text_size.w / 2;
		double y = m_rect.h / 2 - m_text_size.h / 2;
		m_ctx->fill_string( BLPoint( m_rect.x + x, m_rect.y + m_rect.h - y ), m_font, m_name.c_str(), m_font_scale );
	}
}; // class button

} // namespace san::ui
