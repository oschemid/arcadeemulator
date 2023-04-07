#pragma once
#include "types.h"
#include "xprocessors.h"
#include "memorymap.h"


namespace ae::gameboy
{
	class SerialLink : public xprocessors::Device
	{
	protected:
		uint16_t _clock;

		bool _enabled;
		uint8_t _count;

		// IO registers
		uint8_t _sb;
		uint8_t _sc;

	public:
		SerialLink();

		uint8_t getRegister(const MemoryMap) const;
		void setRegister(const MemoryMap, const uint8_t);

		void tick();
	};
}