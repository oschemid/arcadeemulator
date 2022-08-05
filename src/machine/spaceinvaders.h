#pragma once

#include <cstdint>

#include "machine.h"
#include "dipswitch.h"
#include "memory.h"
#include "cpu.h"


namespace ae
{
	namespace machine
	{
		class SpaceInvaders : public IMachine
		{
		protected:
			DIPSwitch<2> ships;
			DIPSwitch<1> extraShip;

			virtual void loadMemory() = 0;
			virtual const uint8_t in1();
			virtual const uint8_t in2();

		protected:
			void updateDisplay();
			uint16_t Pixels[224 * 256];
			uint8_t shift0, shift1;
			uint8_t shift_offset;

			virtual const uint8_t in(const uint8_t);
			void out(const uint8_t, const uint8_t);

		public:
			ae::IMemory* memory;
			ae::ICpu* cpu;

		public:
			SpaceInvaders();

			std::list<ae::IParameter*> getParameters() const override;
			bool init() override;
			bool run() override;
		};
		class SpaceInvadersMidway : public SpaceInvaders
		{
		protected:
			void loadMemory() override;

		public:
			const string getName() const override { return "Space Invaders (Midway)"; }
			const string getID() const override { return "SpaceInvadersMidway"; }
			const string getDescription() const override { return "1978 Taito"; }

			SpaceInvadersMidway();
		};
		class SpaceInvadersTV : public SpaceInvaders
		{
		protected:
			void loadMemory() override;
			const uint8_t in(const uint8_t) override;

		public:
			const string getName() const override { return "Space Invaders (TV)"; }
			const string getID() const override { return "SpaceInvadersTV"; }
			const string getDescription() const override { return "1978 Taito"; }

			SpaceInvadersTV();
		};
		class SpaceChaserCV : public SpaceInvaders
		{
		protected:
			void loadMemory() override;
			const uint8_t in1() override;
			const uint8_t in(const uint8_t) override;
			bool init() override;

		public:
			const string getName() const override { return "Space Chaser (CV)"; }
			const string getID() const override { return "SpaceChaserCV"; }
			const string getDescription() const override { return "1979 Taito"; }

			SpaceChaserCV();
		};
	}
}