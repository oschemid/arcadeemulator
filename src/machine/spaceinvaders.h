#pragma once

#include <cstdint>

#include "machine.h"
#include "../memory.h"
#include "../cpu/i8080.h"


namespace ae
{
	namespace machine
	{
		class spaceinvaders : public IMachine
		{
		protected:
			string version;
			uint8_t ships;

		public:
			ae::memory memory;
			ae::cpu::i8080 cpu;

		public:
			spaceinvaders(const string&);

			bool init() override;
			bool run() override;

		protected:
			void updateDisplay();
			uint16_t Pixels[224 * 256];

		};
	}
}