#pragma once

#include "types.h"
#include "taito8080.h"
#include "dipswitch.h"
#include "memory.h"


namespace ae
{
	namespace machine
	{
		class SpaceChaserCV : public Taito8080
		{
			DIPSwitch<1> ships;
			DIPSwitch<1> difficulty;

			const uint8_t in1();
			const uint8_t in2();

			std::list<ae::IParameter*> getParameters() const override;

		protected:
			void loadMemory() override;
			const uint8_t in(const uint8_t) override;

		public:
			const string getName() const override { return "Space Chaser (CV)"; }
			const string getID() const override { return "SpaceChaserCV"; }
			const string getDescription() const override { return "1979 Taito"; }

			SpaceChaserCV();
		};
	}
}