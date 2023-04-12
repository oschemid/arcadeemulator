#pragma once
#include "types.h"


namespace ae::display
{
	class RasterDisplay
	{
	public:
		RasterDisplay(const geometry_t);
		~RasterDisplay();

		void init();
		void set(const uint16_t, const uint16_t, const rgb_t);
		void set(const uint16_t, const uint16_t, const uint32_t);
		void refresh() { _refresh = true; }
		bool needRefresh() const { return _refresh; }
		geometry_t getGeometry() const { return _geometry; }
		uint32_t* getBuffer() { _refresh = false; return _buffer; }

	protected:
		uint32_t* _buffer{ nullptr };
		geometry_t _geometry;
		bool _refresh{ false };
	};
}