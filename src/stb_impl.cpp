#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_impl.hpp"

namespace stb {

[[nodiscard]] std::shared_ptr <SDL_Surface> load( const char * filename ) {

	int	src_w		= 0;
	int	src_h		= 0;
	int channels	= 0; // will be actual image channels, it is not set to desired.
	uint8_t * p_image = stbi_load( filename, &src_w, &src_h, &channels, 4/*desired_channels*/ );
	if ( !p_image ) {
		fprintf( stderr, "stbi_load(): error.\n" );
	}

	SDL_Surface * p_surface = nullptr;

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

		// No copy is made of the pixel data. Pixel data is not managed automatically; you must free the surface before you free the pixel data.
		p_surface = SDL_CreateRGBSurfaceWithFormatFrom( p_image, src_w, src_h, 32, src_w * 4, SDL_PIXELFORMAT_RGBA32 ); // SDL_PIXELFORMAT_RGB888
		if ( !p_surface ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface: %s", SDL_GetError() );
			stbi_image_free( p_image );
		}
	}

	return std::shared_ptr<SDL_Surface>( p_surface, []( SDL_Surface * p_surface ) {
			if ( p_surface ) {
				void * p_pixels = p_surface->pixels;
				SDL_FreeSurface( p_surface );
				if ( p_pixels ) {
					stbi_image_free( p_pixels );
				}
			}
		} );
}

} // namespace stb
