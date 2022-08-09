#include "rasterdisplay.h"
#include "../ui/ui.h"

using namespace ae;

display::RasterDisplay::RasterDisplay() :
	_width(0),
	_height(0),
	_pixels(nullptr) {
}
display::RasterDisplay::~RasterDisplay() {
	if (_pixels)
		delete[] _pixels;
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
	ae::ui::createDisplay(_width, _height);
	return true;
}
bool display::RasterDisplay::update() {
	_callback(_pixels);
	ae::ui::updateDisplay(_pixels);
	ae::ui::refresh();
	return true;
}