#pragma once

namespace san {

class image_view {
	uint8_t *	m_data		= nullptr;
	int			m_width;
	int			m_height;
	int			m_stride;
	int			m_bytes_per_pixel;

public:

#ifdef SDL_MAJOR_VERSION
 #if SDL_MAJOR_VERSION >= 2

	image_view( SDL_Surface * p )
		: m_data( reinterpret_cast<uint8_t *>(p->pixels) )
		, m_width( p->w )
		, m_height( p->h )
		, m_stride( p->pitch )
		, m_bytes_per_pixel( SDL_BYTESPERPIXEL( p->format->format ) ) {}

 #endif
#else
 #error "No SDL2 found."
#endif

	explicit operator bool () const { return m_data != nullptr; }

	int			width()					const { return m_width; }
	int			height()				const { return m_height; }
	int			stride()				const { return m_stride; }
	uint8_t *	ptr()					const { return m_data; }
	uint8_t *	row_ptr( int y )		const { return ptr() + y * m_stride; }
	uint8_t *	col_ptr( int x )		const { return ptr() + x * m_bytes_per_pixel; }
	uint8_t *	pix_ptr( int x, int y )	const { return row_ptr( y ) + x * m_bytes_per_pixel; }

}; // class image_view

} // namespace san
