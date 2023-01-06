#pragma once

#include "types.h"
#include "machine.h"

#include "memory.h"
#include "xprocessors.h"
#include "display.h"


namespace ae
{
	namespace machine
	{
		class Taito8080 : public IMachine
		{
		public:
			static const ae::Layout::zones invaders_layout;

		protected:
			size_t _memorySize;
			virtual void updateDisplay(uint32_t*);
			uint8_t shift0, shift1;
			uint8_t shift_offset;

			virtual void loadMemory() = 0;
			virtual const uint8_t in(const uint8_t) = 0;
			virtual void out(const uint8_t, const uint8_t);

		public:
			ae::IMemory* memory;
			xprocessors::Cpu* cpu;
			ae::Display* display;
			ae::Layout* layout;
			ae::Layout::zones _zones;

		public:
			Taito8080(const size_t = 0x3fff,
					  const ae::Layout::zones = {});
			virtual ~Taito8080() = default;

			bool init() override;
			bool run() override;
		};
	}
}