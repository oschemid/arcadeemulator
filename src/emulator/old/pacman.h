#pragma once

#include "types.h"
#include "emulator.h"

#include "memory.h"
#include "xprocessors.h"
#include "display.h"
#include "dipswitch.h"
#include "../gui/vulkan/engine.h"


namespace ae
{
	namespace machine
	{
		class Pacman : public emulator::Emulator
		{
		protected:
			DIPSwitch<2> coinage;
			DIPSwitch<2> lives;
			DIPSwitch<2> bonus;
			DIPSwitch<1> difficulty;
			DIPSwitch<1> ghostname;
			DIPSwitch<1> rackadvance;

			void load_palettes();
			void draw();
			void draw_tile(uint32_t*, const uint8_t, const uint8_t, const uint8_t, const uint8_t) const;
			void draw_sprite(uint32_t*, const uint8_t, const uint8_t, const uint8_t, const uint8_t, const bool, const bool) const;
			virtual void updateDisplay(uint32_t*);

			virtual void loadMemory();
			bool writeMemory(const uint16_t, const uint8_t);
			uint8_t readMemory(const uint16_t) const;
			const uint8_t in0() const;
			const uint8_t in1() const;
			const uint8_t dip() const;

			bool interrupt_enabled;
			uint8_t interrupt_vector;
			bool sound_enabled;
			bool flip_screen;

			ae::gui::RasterDisplay* _raster;
			uint32_t* _src;

			std::chrono::steady_clock::time_point StartTime;

			uint64_t LastDraw = 0;
			uint8_t DrawFull = 0;
			uint64_t LastInput = 0;
			uint64_t LastThrottle = 0;
			uint64_t LastDisplay = 0;
			uint32_t ClocksPerMS = 3720;
			uint64_t ClockCompensation = 0;
			uint64_t ClockCount = 0;
			SDL_Event ev;

		public:
			ae::IMemory* memory;
			ae::IMemory* videorom;
			ae::IMemory* paletterom;

			uint32_t colors[32];
			uint8_t* tiles;
			uint8_t* sprites;
			uint8_t* spritesxy;
			xprocessors::UCpu cpu;

		public:
			Pacman();
			virtual ~Pacman() = default;

			emulator::SystemInfo getSystemInfo() const override;
/*			std::list<ae::IParameter*> getParameters() const override {
				return { (ae::IParameter*)&coinage,
						 (ae::IParameter*)&lives,
						 (ae::IParameter*)&bonus,
						 (ae::IParameter*)&difficulty,
						 (ae::IParameter*)&ghostname,
						 (ae::IParameter*)&rackadvance };
			}
			*/
			void init() override;
			void load(const json&) override {}
			void run(ae::gui::RasterDisplay*) override;
		};
	}
}