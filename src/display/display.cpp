#include "display.h"


using namespace ae::display;

RasterDisplay::RasterDisplay(const geometry_t g) :
	_geometry{ g }
{
}

RasterDisplay::~RasterDisplay()
{
	if (_buffer)
		delete[] _buffer;
}

void RasterDisplay::init()
{
	_buffer = new uint32_t[_geometry.width * _geometry.height];
}

void RasterDisplay::set(const uint16_t x, const uint16_t y, const rgb_t color)
{
	_buffer[y*_geometry.width+x] = 0xff000000 | (color.blue << 16) | (color.green << 8) | color.red;
}

void RasterDisplay::set(const uint16_t x, const uint16_t y, const uint32_t color)
{
	_buffer[y * _geometry.width + x] = color;
}
