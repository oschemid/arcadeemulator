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
			void updateDisplay(uint32_t*) override;
			const uint8_t in(const uint8_t) override;

			ae::IMemory* colorram;
			const uint8_t read_colorram(const uint16_t);
			bool write_colorram(const uint16_t, const uint8_t);
		public:
//			std::list<ae::IParameter*> getParameters() const override;

			void init() override;
//			MachineRequirements getRequirements() const override;
			SpaceChaserCV();
			virtual ~SpaceChaserCV() = default;
		};
	}
}