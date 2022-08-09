#pragma once

#include "types.h"
#include <functional>


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
		virtual bool update() = 0;

	public:
		static Display* create();
	};
}
