#pragma once

namespace san::adaptor {

class agg_image {
	san::surface_view &	m_image;

	// Bounds
	int				m_x = 0;
	int				m_y = 0;
	int				m_w;
	int				m_h;

public:
	static constexpr size_t pix_width = sizeof( ::agg::rgba8::long_type );

	using value_type	= ::agg::rgba8::value_type;
	using color_type	= ::agg::rgba8;

	enum order_type { B = 0, G, R, A };

	agg_image( surface_view & iv )
		: m_image( iv )
		, m_w( m_image.width() )
		, m_h( m_image.height() ) {}

// May be useful in future...
#if 0
	// Bound surface inside surface
	agg_image_adaptor( sdl::surface & p_surface, int x, int y, int w, int h ) :
		m_surface( p_surface ),
		m_x( x ), m_y( y ), m_w( w ), m_h( h )
	{
		if ( m_x < 0 ) { m_w += m_x; m_x = 0; }
		if ( m_y < 0 ) { m_h += m_y; m_y = 0; }

		/*if ( m_x < 0 ) m_x = 0; else*/ if ( m_x >= m_surface.get_width()  ) m_x = m_surface.get_width()  - 1;
		/*if ( m_y < 0 ) m_y = 0; else*/ if ( m_y >= m_surface.get_height() ) m_y = m_surface.get_height() - 1;
		if ( m_w < 0 ) m_w = 0; else if ( m_x + m_w > m_surface.get_width()  ) m_w = m_surface.get_width()  - m_x;
		if ( m_h < 0 ) m_h = 0; else if ( m_y + m_h > m_surface.get_height() ) m_h = m_surface.get_height() - m_y;
	}
#endif

	int width()  const { return m_w; }
	int height() const { return m_h; }
	int stride() const { return m_image.stride(); }

	value_type * pix_ptr( int x, int y ) const {
		return m_image.pix_ptr( m_x + x, m_y + y );
	}

	color_type pixel( int x, int y ) const {
		return ::agg::argb8_packed( *reinterpret_cast<uint32_t *>( m_image.pix_ptr( m_x + x, m_y + y ) ) );
	}

	value_type * row_ptr( int /*x*/, int y, unsigned /*len*/ ) const { 
		return m_image.row_ptr( y );
	}

	void copy_color_hspan( int x, int y, unsigned len, const color_type * colors ) const {
		x += m_x;
		y += m_y;

		value_type * p = row_ptr( x, y, len ) + x * 4;
		do {
			p[order_type::R] = colors->r;
			p[order_type::G] = colors->g;
			p[order_type::B] = colors->b;
			p[order_type::A] = colors->a;
			++colors;
			p += 4;
		} while ( --len );
	}

	void copy_color_vspan( int x, int y, unsigned len, const color_type * colors ) const {
		x += m_x;
		y += m_y;

		value_type * p = m_image.pix_ptr( x, y );
		do {
			p[order_type::R] = colors->r;
			p[order_type::G] = colors->g;
			p[order_type::B] = colors->b;
			p[order_type::A] = colors->a;
			++colors;
			p += m_image.stride();
		} while ( --len );
	}
}; // class agg_image

} // namespace san::adaptor
