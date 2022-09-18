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
			uint32_t* _pixels;
			updatefn _callback;
			SDL_Texture* _texture;

		public:
			RasterDisplay();
			~RasterDisplay();

			bool setSize(const uint16_t w, const uint16_t h) override;
			bool registerCallback(updatefn) override;

			bool setPixel(const uint16_t, const uint16_t, const uint32_t) override;
			bool init() override;
			bool update(const SDL_Rect) override;
		};
	}
}