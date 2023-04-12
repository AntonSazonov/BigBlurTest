#pragma once

#include <memory>
#include <SDL_image.h>

namespace sdl {

[[nodiscard]] static std::shared_ptr <SDL_Surface> load( const char * filename ) {
	static bool initialized = false;
	if ( !initialized ) {
		IMG_Init( IMG_INIT_JPG );
		initialized = true;
	}

	SDL_Surface * p_image = IMG_Load( filename );
	if ( !p_image ) {
		SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "IMG_Load(): %s", SDL_GetError() );
	}

	return std::shared_ptr<SDL_Surface>( p_image, []( SDL_Surface * p ) {
#ifndef NDEBUG
			fprintf( stderr, "Free image: %p\n", p );
#endif
			SDL_FreeSurface( p );
		} );
}

} // namespace sdl
