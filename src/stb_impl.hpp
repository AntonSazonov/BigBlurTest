#pragma once

#define STBI_ONLY_JPEG
#define STBI_NO_LINEAR
#define STBI_NO_HDR

//#define STBIR_DEFAULT_FILTER_UPSAMPLE
//#define STBIR_DEFAULT_FILTER_DOWNSAMPLE

#include "stb/stb_image.h"
//#include "stb/stb_image_resize.h"

#include <SDL.h>		// for SDL_Surface
#include <memory>		// for std::shared_ptr

namespace stb {

[[nodiscard]] std::shared_ptr <SDL_Surface> load( const char * );

} // namespace stb
