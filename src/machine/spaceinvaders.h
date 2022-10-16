#pragma once

#include <cstdint>

#include "taito8080.h"
#include "dipswitch.h"
#include "memory.h"
#include "cpu.h"
#include "../emulator/sound.h"


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
			ae::Sound* s1;
			ae::AudioDevice* audiodevice;

			virtual const uint8_t in1();
			virtual const uint8_t in2();
			virtual  void out(const uint8_t, const uint8_t);

		protected:
			virtual const uint8_t in(const uint8_t);

		public:
			SpaceInvaders();
			virtual ~SpaceInvaders() = default;

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
			virtual ~SpaceInvadersMidway() = default;
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
			virtual ~SpaceInvadersTV() = default;
		};
	}
}