#pragma once

#include <cstdint>

#include "taito8080.h"
#include "dipswitch.h"
#include "memory.h"
#include "cpu.h"


namespace ae
{
	namespace machine
	{
		class SpaceInvaders : public Taito8080
		{
		protected:
			DIPSwitch<2> ships;
			DIPSwitch<1> extraShip;
			DIPSwitch<1> coinInfo;

			virtual const uint8_t in1();
			virtual const uint8_t in2();

		protected:
			virtual const uint8_t in(const uint8_t);

		public:
			SpaceInvaders();

			std::list<ae::IParameter*> getParameters() const override;
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
			virtual const uint8_t in2() override;

		public:
			const string getName() const override { return "Space Invaders (TV)"; }
			const string getID() const override { return "SpaceInvadersTV"; }
			const string getDescription() const override { return "1978 Taito"; }

			SpaceInvadersTV();
		};
	}
}