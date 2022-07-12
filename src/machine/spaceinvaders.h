#pragma once

#include <cstdint>

#include "machine.h"
#include "../memory.h"
#include "cpu.h"


namespace ae
{
	namespace machine
	{
		class SpaceInvaders : public IMachine
		{
		protected:
			uint8_t ships;

			virtual void loadMemory() = 0;

		protected:
			void updateDisplay();
			uint16_t Pixels[224 * 256];
			uint8_t port1;
			uint8_t port2;
			std::uint8_t shift0, shift1;
			std::uint8_t shift_offset;

			virtual const uint8_t in(const uint8_t);
			void out(const uint8_t, const uint8_t);

		public:
			ae::memory memory;
			ae::ICpu* cpu;

		public:
			SpaceInvaders();

			bool init() override;
			bool run() override;


		};
		class SpaceInvadersMidway : public SpaceInvaders
		{
		protected:
			void loadMemory() override;

		public:
			SpaceInvadersMidway();
		};
		class SpaceInvadersTV : public SpaceInvaders
		{
		protected:
			void loadMemory() override;
			const uint8_t in(const uint8_t) override;

		public:
			SpaceInvadersTV();
		};
	}
}