#pragma once

#include "types.h"
#include "machine.h"

#include "memory.h"
#include "cpu.h"
#include "display.h"
#include "dipswitch.h"


namespace ae
{
	namespace machine
	{
		class Pacman : public IMachine
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
		public:
			ae::IMemory* memory;
			ae::IMemory* videorom;
			ae::IMemory* paletterom;

			uint32_t colors[32];
			uint8_t* tiles;
			uint8_t* sprites;
			uint8_t* spritesxy;
			xprocessors::Cpu* cpu;
			ae::Display* display;


		public:
			Pacman();
			virtual ~Pacman() = default;

			const string getName() const override { return "Pacman"; }
			const string getID() const override { return "Pacman"; }
			const string getDescription() const override { return "Namco"; }
			std::list<ae::IParameter*> getParameters() const override {
				return { (ae::IParameter*)&coinage,
						 (ae::IParameter*)&lives,
						 (ae::IParameter*)&bonus,
						 (ae::IParameter*)&difficulty,
						 (ae::IParameter*)&ghostname,
						 (ae::IParameter*)&rackadvance };
			}

			bool init() override;
			bool run() override;
		};
	}
}