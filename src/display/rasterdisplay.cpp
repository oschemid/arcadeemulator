#include "rasterdisplay.h"
#include "../ui/ui.h"

using namespace ae;

display::RasterDisplay::RasterDisplay() :
	_width(0),
	_height(0),
	_pixels(nullptr),
	_texture(nullptr) {
}
display::RasterDisplay::~RasterDisplay() {
	if (_pixels)
		delete[] _pixels;
	if (_texture)
		SDL_DestroyTexture(_texture);
}

bool display::RasterDisplay::setSize(const uint16_t w, const uint16_t h) {
	_width = w;
	_height = h;
	return true;
}
bool display::RasterDisplay::registerCallback(updatefn fn) {
	_callback = fn;
	return true;
}
bool display::RasterDisplay::setPixel(const uint16_t x, const uint16_t y, const uint16_t c) {
	_pixels[y * _width + x] = c;
	return true;
}

bool display::RasterDisplay::init() {
	_pixels = new uint16_t[_width * _height];
	_texture = SDL_CreateTexture(ae::ui::getRenderer(),
								 SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING,
								 _width, _height);
	if (!_texture) {
		return false;
	}
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_NONE);
	return true;
}
bool display::RasterDisplay::update(const SDL_Rect rect) {
	_callback(_pixels);
	SDL_UpdateTexture(_texture, NULL, _pixels, 2 * _width);
	SDL_RenderCopy(ae::ui::getRenderer(), _texture, NULL, &rect);
	return true;
}