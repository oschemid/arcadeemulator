#pragma once

#include "types.h"
#include <functional>
#include "SDL2/SDL.h"


namespace ae
{
	class Display {
	public:
		typedef std::function<void(uint16_t*)> updatefn;

	public:
		virtual bool setSize(const uint16_t, const uint16_t) = 0;
		virtual bool registerCallback(updatefn) = 0;

		virtual bool setPixel(const uint16_t, const uint16_t, const uint16_t) = 0;
		virtual bool init() = 0;
		virtual bool update(const SDL_Rect) = 0;

	public:
		static Display* create();
	};

	class Layout {
	public:
		typedef std::vector<std::pair<rgb_t, rect_t>> zones;

	protected:
		uint16_t _width;
		uint16_t _height;
		zones _zones;
		SDL_Texture* _texture;

	public:
		Layout();
		~Layout();

		bool setSize(const uint16_t, const uint16_t);
		bool setZones(zones);

		bool init();
		bool update(const SDL_Rect);

	public:
		static Layout* create();
	};
}
