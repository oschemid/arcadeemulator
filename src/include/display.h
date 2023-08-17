#pragma once
#include "types.h"
#include <array>

using aos::string;
using aos::uint8_t;
using aos::uint16_t;
using aos::rgb_t;
using aos::geometry_t;


namespace aos::display
{
	class RasterDisplay
	{
	public:
		RasterDisplay(const geometry_t);
		~RasterDisplay();

		void init();
		void set(const uint16_t, const uint16_t, const rgb_t);
		void set(const uint16_t, const uint16_t, const uint32_t);
		void refresh() { _current = 1 - _current; _refresh = true; }
		bool needRefresh() const { return _refresh; }
		geometry_t getGeometry() const { return (_geometry.rotation == geometry_t::rotation_t::NONE) ? _geometry : geometry_t{.width = _geometry.height, .height=_geometry.width}; }
		uint32_t* getBuffer() { _refresh = false; return _buffers[1-_current]; }

	protected:
		uint8_t _current{ 0 };
		std::array<std::uint32_t*,2> _buffers{ nullptr };
		geometry_t _geometry;
		bool _refresh{ false };
	};
}