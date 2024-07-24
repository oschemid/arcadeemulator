#pragma once
#include "device.h"
#include <array>


namespace aos::device
{
	class RasterDisplay : public Device
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

	protected:
		RasterDisplay(const geometry_t);
		uint8_t _current{ 0 };
		geometry_t _geometry;
		std::array<std::uint32_t*, 2> _buffers{ nullptr };
		bool _refresh{ false };
	};
}
