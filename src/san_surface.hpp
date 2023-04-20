#pragma once

#include <cstring>		// std::memcpy
#include <memory>		// std::shared_ptr
#include "stb_impl.hpp"

namespace san {

class surface {
	bool		m_managed_outside	= false;
	uint8_t *	m_data				= nullptr;
	int			m_width;
	int			m_height;
	int			m_stride;
	int			m_components;	// 3 - RGB/BGR, 4 - RGBA/ARGB/...

public:
	surface( int width, int height, int components )
		: m_data( new (std::nothrow) uint8_t [width * height * components] )
		, m_width( width )
		, m_height( height )
		, m_stride( width * components )
		, m_components( components ) {}

	surface( uint8_t * p, int width, int height, int stride, int components )
		: m_managed_outside( true )
		, m_data( p )
		, m_width( width )
		, m_height( height )
		, m_stride( stride )
		, m_components( components ) {}

	surface( const surface & s )
		: surface( s.width(), s.height(), s.components() )
	{
		if ( m_data && s ) {
			s.blit_to( this );
		}
	}

	virtual ~surface() {
		if ( !m_managed_outside ) {
			delete [] m_data;
		}
	}

	explicit operator bool () const { return m_data != nullptr; }

	bool		is_managed()			const { return m_managed_outside; }

	int			width()					const { return m_width; }
	int			height()				const { return m_height; }
	int			stride()				const { return m_stride; }
	int			components()			const { return m_components; }

//#ifdef __GNUG__
//#warning "TODO: template pointer type impls."
//#endif
	uint8_t *	ptr()					const { return m_data; }
	uint8_t *	row_ptr( int y )		const { return ptr() + y * m_stride; }
	uint8_t *	col_ptr( int x )		const { return ptr() + x * m_components; }
	uint8_t *	pix_ptr( int x, int y )	const { return row_ptr( y ) + x * m_components; }

	// Swap 2 components
	void swap_components( int a, int b ) {
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
			for ( int y = 0; y < m_height; y++ ) {
				uint8_t * ps = row_ptr( y );
				uint8_t * pd = p_dst.row_ptr( y );
				std::memcpy( pd, ps, m_width * m_components );
			}
		} else {
			// Resize...
#if 0
			bool result = stbir_resize_uint8_generic(
					m_data, m_width, m_height, m_stride,							// src
					p_dst.ptr(), p_dst.width(), p_dst.height(), p_dst.stride(),	// dst
					m_components, STBIR_ALPHA_CHANNEL_NONE, 0/*flags*/, STBIR_EDGE_CLAMP/*STBIR_EDGE_ZERO*/,
					STBIR_FILTER_BOX, STBIR_COLORSPACE_LINEAR, nullptr/*alloc_context*/ );
#else
			stbir_resize_uint8(
					m_data, m_width, m_height, m_stride,							// src
					p_dst.ptr(), p_dst.width(), p_dst.height(), p_dst.stride(),	// dst
					m_components );
#endif
		}
	}

	void blit_to( surface * p_dst ) const {
		blit_to( *p_dst );
	}

	void blit_to( std::shared_ptr <surface> p_dst ) const {
		blit_to( *(p_dst.get()) );
	}
}; // class surface


// Same as 'surface' but doesn't manages memory allocation
class surface_view : public surface {
public:
	surface_view( surface & s ) : surface( s.ptr(), s.width(), s.height(), s.stride(), s.components() ) {}
	surface_view( surface * s ) : surface( s->ptr(), s->width(), s->height(), s->stride(), s->components() ) {}
	surface_view( std::shared_ptr <surface> & s ) : surface_view( s.get() ) {}

	void blit_to( surface_view p_dst ) const {
		surface::blit_to( p_dst );
	}
}; // class surface_view


[[nodiscard]] inline std::shared_ptr <san::surface> load_image( const char * filename ) {

	int	src_w		= 0;
	int	src_h		= 0;
	int channels	= 0; // will be actual image channels, it is not set to desired.
	uint8_t * p_image = stbi_load( filename, &src_w, &src_h, &channels, 4/*desired_channels*/ );
	if ( !p_image ) {
		fprintf( stderr, "stbi_load(): error.\n" );
	}

	san::surface * p_surface = nullptr;

	if ( p_image ) {

		// I don't care of alignment unless i using load-from-memory SIMD instructions.
#if 0
		printf( "p_image = %p\n", p_image );
		printf( "TODO: check pointer alignment of 'stbi_load'!\n" );

		uintptr_t p = uintptr_t(p_image);
		int alignment = 1;
		while ( !(p & 1) ) {
			alignment <<= 1;
			p >>= 1;
		}
		printf( "p_image align.: %d\n", alignment );
#endif

		p_surface = new (std::nothrow) san::surface( p_image, src_w, src_h, src_w * 4, 4 );
		if ( !p_surface ) {
			fprintf( stderr, "Couldn't create surface.\n" );
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
