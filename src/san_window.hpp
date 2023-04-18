#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>

namespace san {

class window {
	bool			m_is_valid	= false;

protected:
	SDL_Window *	m_window	= nullptr;
	SDL_Surface *	m_surface	= nullptr;
	bool			m_wait_mode	= true;

public:
	window( int width, int height, const char * title = "" ) {
		SDL_LogSetPriority( SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO );
		SDL_SetMainReady();

		if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s\n", SDL_GetError() );
			return;
		}

		SDL_DisableScreenSaver();

		m_window = SDL_CreateWindow( title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_HIDDEN /*| SDL_WINDOW_BORDERLESS*/ );
		if ( !m_window ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError() );
			return;
		}

		// Only RGBA32 (4 bytes per pixel) format supported for use with SIMD.
		SDL_assert( SDL_BYTESPERPIXEL( SDL_GetWindowPixelFormat( m_window ) ) == 4 );

		// This surface will be freed when the window is destroyed. Do not free this surface.
		m_surface = SDL_GetWindowSurface( m_window );
		if ( !m_surface ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't get window surface: %s", SDL_GetError() );
			return;
		}

		m_is_valid = true;
	}

	virtual ~window() {
		if ( m_window ) SDL_DestroyWindow( m_window );
		SDL_Quit();
	}

	explicit operator bool () const { return m_is_valid; }

	void set_title( const char * title ) {
		SDL_SetWindowTitle( m_window, title );
	}

	// Time in milliseconds since SDL was initialized.
	//uint64_t time() const { return SDL_GetTicks64(); }

	// Accessors
	const SDL_Surface * surface()		const { return m_surface; }
	      SDL_Surface * surface()			  { return m_surface; }
	int					width()			const { return m_surface->w; }
	int					height()		const { return m_surface->h; }
	uint8_t *			ptr()			const { return reinterpret_cast<uint8_t *>( m_surface->pixels ); }
	int					stride()		const { return m_surface->pitch; }
	uint32_t			pixel_format()	const { return m_surface->format->format; }

	void show() const { SDL_ShowWindow( m_window ); }
	void hide() const { SDL_HideWindow( m_window ); }

	void quit() const {
		SDL_Event ev = {};
		ev.type = SDL_QUIT;
		SDL_PushEvent( &ev );
	}

	// Blit 'p_src' to window surface...
	bool blit( SDL_Surface * p_src ) const {
		if ( SDL_BlitSurface( p_src, nullptr, m_surface, nullptr ) ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "SDL_BlitSurface(): %s", SDL_GetError() );
			return false;
		}
		return true;
	}

	bool blit( std::shared_ptr <SDL_Surface> & p_src ) const {
		return blit( p_src.get() );
	}

	static bool blit( SDL_Surface * p_src, SDL_Surface * p_dst ) {
		if ( SDL_BlitSurface( p_src, nullptr, p_dst, nullptr ) ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "SDL_BlitSurface(): %s", SDL_GetError() );
			return false;
		}
		return true;
	}

	static bool blit_scaled( SDL_Surface * p_src, SDL_Surface * p_dst ) {
		if ( SDL_BlitScaled( p_src, nullptr, p_dst, nullptr ) ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "SDL_BlitScaled(): %s", SDL_GetError() );
			return false;
		}
		return true;
	}

	virtual bool on_event( const SDL_Event * const ) = 0;
	virtual void on_frame() = 0;

	std::shared_ptr <SDL_Surface> copy_surface() const {
		SDL_Surface * p = SDL_CreateRGBSurfaceWithFormat( 0/*unused*/, m_surface->w, m_surface->h, SDL_BYTESPERPIXEL( m_surface->format->format ), m_surface->format->format );
		if ( !p ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface: %s", SDL_GetError() );
		}
		if ( p ) blit( m_surface, p ); // Copy content
		return std::shared_ptr<SDL_Surface>( p, []( SDL_Surface * p ) { SDL_FreeSurface( p ); } );
	}

	static std::shared_ptr <SDL_Surface> create_surface( int w, int h, int depth ) {
		SDL_Surface * p = SDL_CreateRGBSurfaceWithFormat( 0/*unused*/, w, h, depth, SDL_PIXELFORMAT_RGB888 );
		if ( !p ) {
			SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface: %s", SDL_GetError() );
		}
		return std::shared_ptr<SDL_Surface>( p, []( SDL_Surface * p ) { SDL_FreeSurface( p ); } );
	}


	//  true - wait events
	// false - don't wait events
	void set_wait_mode( bool wait_mode ) {
		m_wait_mode = wait_mode;
	}

	void run() {
		for ( ; ; ) {
			SDL_Event event;
			if ( m_wait_mode ) {
				SDL_WaitEvent( &event );	// wait
			} else {
				SDL_PollEvent( &event );	// no wait
			}

			// Pass 64-bit timestamp with each event.
			// Event itself contain only 32-bit timestamp.
			if ( !on_event( &event ) ) break;
			if ( event.type == SDL_QUIT ) break;

			on_frame();
			SDL_UpdateWindowSurface( m_window );
		}
	}
}; // class window

} // namespace san
