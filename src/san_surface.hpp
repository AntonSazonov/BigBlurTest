#pragma once

namespace san {

class surface {
	bool		m_managed_outside;
	int			m_width;
	int			m_height;
	int			m_components;	// 3 - RGB/BGR, 4 - RGBA/ARGB/...
	int			m_stride;
	uint8_t *	m_data;

	void free() {
		if ( !m_managed_outside ) {
			delete [] m_data;
		}
	}

public:
	surface( int width, int height, int components )
		: m_managed_outside( false )
		, m_width( width )
		, m_height( height )
		, m_components( components )
		, m_stride( (((m_width * m_components * 8) + 31) & ~31) >> 3 )
		, m_data( new (std::nothrow) uint8_t [m_stride * m_height] ) {}

	surface( uint8_t * p, int width, int height, int stride, int components )
		: m_managed_outside( true )
		, m_width( width )
		, m_height( height )
		, m_components( components )
		, m_stride( stride )
		, m_data( p ) {}

	surface( const surface & other )
		: m_managed_outside( false )
		, m_width( other.m_width )
		, m_height(	other.m_height )
		, m_components(	other.m_components )
		, m_stride( other.m_stride )
		, m_data( new (std::nothrow) uint8_t [m_stride * m_height] )
	{
		if ( m_data && other ) other.blit_to( this );
	}

	surface & operator = ( const surface & other ) {
		if ( this == &other ) return *this;
		free();
		m_managed_outside	= false;
		m_width				= other.m_width;
		m_height			= other.m_height;
		m_components		= other.m_components;
		m_stride			= other.m_stride;
		m_data				= new (std::nothrow) uint8_t [m_stride * m_height];
		if ( m_data && other ) other.blit_to( this );
		return *this;
	}

	virtual ~surface() { free(); }

	explicit operator bool () const { return m_data != nullptr; }

	int			width()					const { return m_width; }
	int			height()				const { return m_height; }
	int			stride()				const { return m_stride; }
	int			components()			const { return m_components; }

	uint8_t *	ptr()					const { return m_data; }
	uint8_t *	row_ptr( int y )		const { return ptr() + y * m_stride; }
	uint8_t *	col_ptr( int x )		const { return ptr() + x * m_components; }
	uint8_t *	pix_ptr( int x, int y )	const { return row_ptr( y ) + x * m_components; }

	// Swap 2 components
	void swap_components( uint8_t a, uint8_t b ) {
		for ( int y = 0; y < m_height; y++ ) {
			uint8_t * p = row_ptr( y );
			for ( int x = 0; x < m_width; x++ ) {
				std::swap( p[a], p[b] );
				p += m_components;
			}
		}
	}

	void blit_to( surface & p_dst ) const {
		assert( m_components == p_dst.components() );

		if ( m_width  == p_dst.width() &&
			 m_height == p_dst.height() )
		{
			int len = m_width * m_components;
			for ( int y = 0; y < m_height; y++ ) {
				uint8_t * ps = row_ptr( y );
				uint8_t * pd = p_dst.row_ptr( y );
				std::memcpy( pd, ps, len );
			}
		} else {
			// Resize...
			stbir_resize_uint8(
					m_data, m_width, m_height, m_stride,						// src
					p_dst.ptr(), p_dst.width(), p_dst.height(), p_dst.stride(),	// dst
					m_components );
		}
	}

	void blit_to(                  surface * p_dst ) const { blit_to( *p_dst ); }
	void blit_to( std::shared_ptr <surface>  p_dst ) const { blit_to( *p_dst ); }
}; // class surface


// Same as 'surface' but doesn't manages memory allocation
class surface_view : public surface {
public:
	surface_view( surface & s ) : surface( s.ptr(), s.width(), s.height(), s.stride(), s.components() ) {}
	surface_view( std::shared_ptr <surface> & s ) : surface_view( *s ) {}

	void blit_to( surface_view & p_dst ) const {
		surface::blit_to( p_dst );
	}
}; // class surface_view


[[nodiscard]] inline std::shared_ptr <san::surface> load_image( const char * filename ) {

	int	src_w		= 0;
	int	src_h		= 0;
	int channels	= 0; // will be actual image channels, it is not set to desired.
	uint8_t * p_image = stbi_load( filename, &src_w, &src_h, &channels, 4/*desired_channels*/ );
	if ( !p_image ) {
		std::fprintf( stderr, "stbi_load(): error.\n" );
	}

	san::surface * p_surface = nullptr;

	if ( p_image ) {
		p_surface = new (std::nothrow) san::surface( p_image, src_w, src_h, src_w * 4, 4 );
		if ( !p_surface ) {
			std::fprintf( stderr, "Couldn't create surface.\n" );
			stbi_image_free( p_image );
		} else {
			p_surface->swap_components( 0, 2 );
		}
	}

	return std::shared_ptr<san::surface>( p_surface, []( san::surface * p_surface ) {
			if ( p_surface ) {
				stbi_image_free( (void *)p_surface->ptr() );
				delete p_surface;
			}
		} );
}

} // namespace san
