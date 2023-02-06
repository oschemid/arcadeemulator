#pragma once
#include "types.h"
#include "taito8080.h"


namespace ae::taito8080
{
	class SpaceChaser : public Cartridge
	{
	protected:
		uint8_t _ships;
		uint8_t _difficulty;
		uint8_t _shift0, _shift1;
		uint8_t _shift_offset;

		uint8_t* _colorram;

		void loadRom(const string&);
	public:
		SpaceChaser();
		virtual ~SpaceChaser();
		void init(const json&) override;
		void updateDisplay(uint32_t*) override;
		uint8_t read(const uint16_t) override;
		void write(const uint16_t, const uint8_t) override;
		void out(const uint8_t, const uint8_t) override;
		uint8_t in(const uint8_t) override;
	};
}