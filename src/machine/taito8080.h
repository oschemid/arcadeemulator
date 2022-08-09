#pragma once

#include "types.h"
#include "machine.h"

#include "memory.h"
#include "cpu.h"
#include "display.h"


namespace ae
{
	namespace machine
	{
		class Taito8080 : public IMachine
		{
		protected:
			size_t _memorySize;
			void updateDisplay(uint16_t*);
			uint8_t shift0, shift1;
			uint8_t shift_offset;

			virtual void loadMemory() = 0;
			virtual const uint8_t in(const uint8_t) = 0;
			void out(const uint8_t, const uint8_t);

		public:
			ae::IMemory* memory;
			ae::ICpu* cpu;
			ae::Display* display;

		public:
			Taito8080(const size_t = 0x3fff);

			bool init() override;
			bool run() override;
		};
	}
}