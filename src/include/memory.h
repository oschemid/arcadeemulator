#pragma once

#include "types.h"
#include <functional>


namespace ae
{
	class IMemory {
	public:
		enum class type {
			NOTHING = 0,
			ROM,
			RAM
		};
	public:
		virtual const uint8_t read(const uint16_t) const = 0;
		virtual bool write(const uint16_t, const uint8_t) = 0;
		virtual bool map(const uint16_t, const uint16_t, type) = 0;
		virtual bool load(const uint16_t, const string) = 0;
	};

	IMemory* newMemory(const uint16_t = 0);
}