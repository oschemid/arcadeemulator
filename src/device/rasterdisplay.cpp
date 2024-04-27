#pragma once
#include "device/rasterdisplay.h"

#include <cassert>


namespace aos::device
{
	RasterDisplay::Ptr RasterDisplay::create(const geometry_t geometry)
	{
		return std::shared_ptr<RasterDisplay>(new RasterDisplay(geometry));
	}

	RasterDisplay::RasterDisplay(const geometry_t g) :
		_geometry{ g }
	{
	}

	RasterDisplay::~RasterDisplay()
	{
		if (_buffers[0])
			delete[] _buffers[0];
		if (_buffers[1])
			delete[] _buffers[1];
	}

	void RasterDisplay::init()
	{
		_buffers[0] = new uint32_t[_geometry.width * _geometry.height];
		_buffers[1] = new uint32_t[_geometry.width * _geometry.height];
	}

	void RasterDisplay::set(const uint16_t x, const uint16_t y, const rgb_t color)
	{
		assert(x < _geometry.width);
		assert(y < _geometry.height);
		uint32_t offset;
		if (_geometry.rotation == geometry_t::rotation_t::NONE)
			offset = y * _geometry.width + x;
		else
			offset = x * _geometry.height + (_geometry.height - y - 1);

		_buffers[_current][offset] = 0xff000000 | (color.blue << 16) | (color.green << 8) | color.red;
	}

	void RasterDisplay::set(const uint16_t x, const uint16_t y, const uint32_t color)
	{
		assert(x < _geometry.width);
		assert(y < _geometry.height);
		uint32_t offset;
		if (_geometry.rotation == geometry_t::rotation_t::NONE)
			offset = y * _geometry.width + x;
		else
			offset = x * _geometry.height + (_geometry.height - y - 1);
		_buffers[_current][offset] = color;
	}
}