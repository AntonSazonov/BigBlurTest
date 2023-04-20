#pragma once

namespace san {

enum class mouse_button_e : uint8_t {
	left	= MK_LBUTTON,//SDL_BUTTON_LEFT,
	middle	= MK_MBUTTON,//SDL_BUTTON_MIDDLE,
	right	= MK_RBUTTON,//SDL_BUTTON_RIGHT,
	x1		= MK_XBUTTON1,//SDL_BUTTON_X1,
	x2		= MK_XBUTTON2,//SDL_BUTTON_X2
}; // enum class mouse_button_e


class window_base {
public:
	virtual ~window_base() {}

	virtual explicit operator bool () const = 0;

	virtual void show() const = 0;
	virtual void hide() const = 0;
	virtual void quit() const = 0;

	virtual double get_time_ms() const = 0;

	virtual surface_view get_surface_view() = 0;
	virtual void update() const = 0;

	virtual void on_mouse_motion( int, int ) = 0;
	virtual void on_mouse_button( int, int, mouse_button_e, bool ) = 0;
	virtual void on_key( int, bool ) = 0;
	virtual void on_frame() = 0;

	virtual void set_wait_events( bool ) = 0;
	virtual void run() = 0;

}; // class window_base

} // namespace san
