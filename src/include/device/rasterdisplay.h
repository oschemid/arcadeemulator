#pragma once
#include "device.h"
#include "ui.h"
#include <array>


namespace aos::device
{
	class RasterDisplay : public DisplayDevice
	{
	public:
		using Ptr = std::shared_ptr<RasterDisplay>;

	public:
		static Ptr create(const geometry_t);

		virtual ~RasterDisplay();

		virtual void init() override;

		void set(const uint16_t, const uint16_t, const rgb_t);
		void set(const uint16_t, const uint16_t, const uint32_t);
		void refresh() { _current = 1 - _current; _refresh = true; }
		//bool needRefresh() const { return _refresh; }
		//geometry_t getGeometry() const { return (_geometry.rotation == geometry_t::rotation_t::NONE) ? _geometry : geometry_t{ .width = _geometry.height, .height = _geometry.width }; }
		//uint32_t* getBuffer() { _refresh = false; return _buffers[1 - _current]; }

	protected:
		RasterDisplay(const geometry_t);
		uint8_t _current{ 0 };
		geometry_t _geometry;
		std::array<std::uint32_t*, 2> _buffers{ nullptr };
		bool _refresh{ false };
	};
}
