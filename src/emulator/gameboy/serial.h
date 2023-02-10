#pragma once
#include "types.h"
#include "xprocessors.h"


namespace ae::gameboy
{
	class SerialLink : public xprocessors::Device
	{
	protected:
		uint16_t _clock;
		bool _enabled;
		uint8_t _count;

		uint8_t _sb;
		uint8_t _sc;

	public:
		SerialLink();

		uint8_t in(const uint8_t) const;
		void out(const uint8_t, const uint8_t);

		void tick();
	};
}