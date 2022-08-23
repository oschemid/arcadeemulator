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
		protected:
			DIPSwitch<1> ships;
			DIPSwitch<1> difficulty;

			const uint8_t in1();
			const uint8_t in2();

		protected:
			void loadMemory() override;
			void updateDisplay(uint16_t*) override;
			const uint8_t in(const uint8_t) override;

			ae::IMemory* colorram;
			const uint8_t read_colorram(const uint16_t);
			bool write_colorram(const uint16_t, const uint8_t);
		public:
			const string getName() const override { return "Space Chaser (CV)"; }
			const string getID() const override { return "SpaceChaserCV"; }
			const string getDescription() const override { return "1979 Taito"; }
			std::list<ae::IParameter*> getParameters() const override;

			bool init() override;
			SpaceChaserCV();
			virtual ~SpaceChaserCV() = default;
		};
	}
}