#pragma once
#include "display.h"


namespace ae
{
	namespace display
	{
		class RasterDisplay : public Display
		{
		protected:
			uint16_t _width;
			uint16_t _height;
			uint16_t* _pixels;
			updatefn _callback;

		public:
			RasterDisplay();
			~RasterDisplay();

			bool setSize(const uint16_t w, const uint16_t h) override;
			bool registerCallback(updatefn) override;

			bool setPixel(const uint16_t, const uint16_t, const uint16_t) override;
			bool init() override;
			bool update() override;
		};
	}
}