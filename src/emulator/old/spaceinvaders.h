#pragma once

#include <cstdint>

#include "taito8080.h"
#include "dipswitch.h"
#include "memory.h"
#include "xprocessors.h"
#include "../sound/sound.h"


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

//			std::list<ae::IParameter*> getParameters() const override;
		};
		class SpaceInvadersMidway : public SpaceInvaders
		{
		protected:
			void loadMemory() override;

		public:
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
			SpaceInvadersTV();
			virtual ~SpaceInvadersTV() = default;
		};
	}
}