#pragma once

#include "types.h"
#include "machine.h"

#include "memory.h"
#include "cpu.h"
#include "display.h"


namespace ae
{
	namespace machine
	{
		class Pacman : public IMachine
		{
		protected:
			virtual void updateDisplay(uint16_t*);

			virtual void loadMemory();
			bool writeMemory(const uint16_t, const uint8_t);

			bool interrupt_enabled;
			uint8_t interrupt_vector;
			bool sound_enabled;
			bool flip_screen;
		public:
			ae::IMemory* memory;
			ae::Cpu* cpu;
			ae::Display* display;

		public:
			Pacman();

			const string getName() const override { return "Pacman"; }
			const string getID() const override { return "Pacman"; }
			const string getDescription() const override { return "Namco"; }
			std::list<ae::IParameter*> getParameters() const override { return {}; }

			bool init() override;
			bool run() override;
		};
	}
}