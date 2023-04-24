#pragma once

#ifndef WIN32
 #error "Only Windows platform is suported."
#endif

#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cassert>
#include <cinttypes>

#include "san_surface.hpp"
#include "san_window_base.hpp"

namespace san {

class dib_section {
	struct bitmap_info {
		BITMAPV5HEADER	m_bi = {};

		bitmap_info( int width, int height ) {
			m_bi.bV5Size		= sizeof( BITMAPV5HEADER );
			m_bi.bV5Width		= width;
			m_bi.bV5Height		= -height;
			m_bi.bV5Planes		= 1;
			m_bi.bV5BitCount	= 32;
			m_bi.bV5Compression	= BI_BITFIELDS;
			m_bi.bV5RedMask		= 0x00ff0000;
			m_bi.bV5GreenMask	= 0x0000ff00;
			m_bi.bV5BlueMask	= 0x000000ff;
			m_bi.bV5AlphaMask	= 0xff000000;
		}
	}; // struct bitmap_info

	bitmap_info			m_bitmap_info;
	HDC					m_hdc;
	void *				m_ptr			= nullptr;
	HBITMAP				m_dib;
	surface				m_surface;

public:
	dib_section( int width, int height )
		: m_bitmap_info( width, height )
		, m_hdc( CreateCompatibleDC( NULL ) )
		, m_dib( CreateDIBSection( GetDC( NULL ), (BITMAPINFO *)&m_bitmap_info.m_bi, DIB_RGB_COLORS, reinterpret_cast<void **>(&m_ptr), NULL, 0 ) )
		, m_surface( (uint8_t *)m_ptr, width, height, width * 4, 4 )
	{
		SelectObject( m_hdc, m_dib );
	}

	virtual ~dib_section() {
		if ( m_dib ) DeleteObject( m_dib );
		if ( m_hdc ) DeleteDC( m_hdc );
	}

	BITMAPV5HEADER &	get_bi()		{ return m_bitmap_info.m_bi; }
	HDC					get_hdc() const	{ return m_hdc; }

	const surface &		get_surface() const { return m_surface; }
	      surface &		get_surface()       { return m_surface; }

	void *	get_ptr()		const { return m_surface.ptr(); }
	int		get_width()		const { return m_surface.width(); }
	int		get_height()	const { return m_surface.height(); }
}; // class dib_section


class window : public window_base {
	LARGE_INTEGER		m_frequency;
	LARGE_INTEGER		m_start;

	dib_section			m_dib_section;

	HWND				m_wnd;
	HDC					m_hdc;
	bool				m_wait_events				= true;
	bool				m_are_input_events_enabled	= true;

	mutable bool		m_is_valid					= false;

	static HINSTANCE	m_instance;
	static ATOM			m_class_atom;
	static int			m_ref_count;

	// Non copyable
	window( const window & ) = delete;
	window & operator = ( const window & ) = delete;

	static LRESULT CALLBACK window_proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
		if ( uMsg == WM_CREATE ) {
			SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( (reinterpret_cast<LPCREATESTRUCT>( lParam ))->lpCreateParams ) );
		}

		/* These messages are sent before WM_CREATE: WM_GETMINMAXINFO, WM_NCCREATE, WM_NCCALCSIZE
		   So we have p_this == nullptr before WM_CREATE. */
		window * p_this = reinterpret_cast<window *>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );

		if ( p_this ) {
			switch ( uMsg ) {
				case WM_MOUSEMOVE:		p_this->on_mouse_motion( LOWORD( lParam ), HIWORD( lParam ) ); break;

				case WM_LBUTTONDOWN:	p_this->on_mouse_button( LOWORD( lParam ), HIWORD( lParam ), mouse_button_e::left, true );  break;
				case WM_LBUTTONUP:		p_this->on_mouse_button( LOWORD( lParam ), HIWORD( lParam ), mouse_button_e::left, false ); break;

				case WM_KEYDOWN:		p_this->on_key( int(wParam), true  ); break;
				case WM_KEYUP:			p_this->on_key( int(wParam), false ); break;

				case WM_PAINT: {
					PAINTSTRUCT	ps;
					BeginPaint( hWnd, &ps );
					p_this->update_surface( ps.hdc );
					EndPaint( hWnd, &ps );
				} break;

				case WM_CLOSE: DestroyWindow( p_this->m_wnd ); break;
				case WM_DESTROY: p_this->quit(); break;
			}
		}

		// We are control window destruction irrespective of user
		if ( uMsg == WM_DESTROY ) {
			--m_ref_count;

			// Exit message loop if no more windows left
			if ( m_ref_count <= 0 ) PostQuitMessage( 0 );
		}

		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	static void print_last_error( const char * p_title, int err = 0 ) {
		char * p_msg;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err ? err : GetLastError(), MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), (LPTSTR)&p_msg, 0, nullptr );
		fprintf( stderr, "%s: %s", p_title, p_msg );
		LocalFree( p_msg );
	}

	void update_surface( HDC hdc ) const {
		BitBlt( m_hdc, 0, 0, m_dib_section.get_width(), m_dib_section.get_height(), m_dib_section.get_hdc(), 0, 0, SRCCOPY );
	}

public:
	// width, height - client area dimentions
	window( int width, int height, const char * title ) : m_dib_section( width, height ) {

		QueryPerformanceFrequency( &m_frequency );
		QueryPerformanceCounter( &m_start );

		if ( !m_instance ) m_instance = GetModuleHandle( NULL );

		if ( !m_class_atom && m_ref_count <= 0 ) {
			WNDCLASS wc = {};
			wc.style			= CS_CLASSDC;//CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
			wc.lpfnWndProc		= window_proc;
			wc.hInstance		= m_instance;
			wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
			wc.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
			wc.lpszClassName	= "san_window_cls";
			m_class_atom = RegisterClass( &wc );
		}
		++m_ref_count;

		const DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

		// Calculate client rect...
		RECT rc = { 0, 0, width, height };
		AdjustWindowRect( &rc, style, FALSE ); // WS_OVERLAPPED can't be used

		m_wnd = CreateWindowEx( 0, reinterpret_cast<LPCTSTR>( LOWORD( m_class_atom ) ),
			title, style, 0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, m_instance, this );
		if ( !m_wnd ) {
			print_last_error( "CreateWindowEx()" );
			return;
		}

		m_hdc = GetDC( m_wnd );

		// Screen work area dimentions
		RECT scr_work_area;
		SystemParametersInfo( SPI_GETWORKAREA, 0, (PVOID)&scr_work_area, 0 );
		int scr_w = scr_work_area.right - scr_work_area.left;
		int scr_h = scr_work_area.bottom - scr_work_area.top;

		// Center window...
		RECT rcWnd;
		GetWindowRect( m_wnd, &rcWnd );
		SetWindowPos( m_wnd, NULL,
			scr_w / 2 - (rcWnd.right - rcWnd.left) / 2,
			scr_h / 2 - (rcWnd.bottom - rcWnd.top) / 2,
			0, 0, SWP_NOSIZE | SWP_NOZORDER );

		m_is_valid = true;
	}

	virtual ~window() {
		if ( m_hdc ) ReleaseDC( m_wnd, m_hdc );
		if ( m_wnd ) DestroyWindow( m_wnd );
		if ( m_ref_count > 0 ) --m_ref_count;
		if ( m_class_atom && m_ref_count <= 0 ) {
			UnregisterClass( reinterpret_cast<LPCTSTR>( m_class_atom ), m_instance );
			m_class_atom = 0;
		}
	}

	explicit operator bool () const override { return m_is_valid; }

	void show() const override { ShowWindow( m_wnd, SW_SHOW ); }
	void hide() const override { ShowWindow( m_wnd, SW_HIDE ); }

	void quit() const override {
		m_is_valid = false; // In case if derived constructor go fail...
		PostQuitMessage( 0 );
	}

	// Enable/disable mouse and keyboard events.
	void enable_input_events( bool state ) override {
		m_are_input_events_enabled = state;
	}

	double time_us() const override {
		LARGE_INTEGER counter;
		QueryPerformanceCounter( &counter );
		return counter.QuadPart * 1.e6 / m_frequency.QuadPart;
	}

	double time_ms() const override {
		LARGE_INTEGER counter;
		QueryPerformanceCounter( &counter );
		return counter.QuadPart * 1.e3 / m_frequency.QuadPart;
	}

	void update() const override {
		update_surface( m_hdc );
	}

	surface_view get_surface_view() override {
		return surface_view( m_dib_section.get_surface() );
	}

	std::shared_ptr <surface> get_surface_copy() const {
		surface * p = new (std::nothrow) surface( m_dib_section.get_surface() );
		return std::shared_ptr<surface>( p, []( surface * p ) { delete p; } );
	}

	//  true - wait events
	// false - don't wait events
	void set_wait_events( bool wait_events ) override {
		m_wait_events = wait_events;
	}

	void run() override {
		if ( window::m_ref_count > 0 ) {
			for ( ; ; ) {
				MSG		msg;
				BOOL	res;

				if ( m_wait_events ) {
					res = GetMessage( &msg, NULL, 0, 0 );
					if ( !res ) break; // WM_QUIT
				} else {
					res = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
				}

				if ( res ) {
					if ( msg.message == WM_QUIT ) break;
					TranslateMessage( &msg );

					bool is_mouse_or_key = 
						(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) ||
						(msg.message >= WM_KEYFIRST   && msg.message <= WM_KEYLAST  );

					// Filter events
					if ( m_are_input_events_enabled ||
					   (!m_are_input_events_enabled && !is_mouse_or_key ) )
					{
						DispatchMessage( &msg );
					}
				}

				on_frame();
				update();
			}
		}
	}
}; // class window

HINSTANCE	window::m_instance		= nullptr;
ATOM		window::m_class_atom	= 0;
int			window::m_ref_count		= 0;

} // namespace san
