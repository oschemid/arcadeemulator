#pragma once

#include "types.h"


namespace ae {
	namespace cpu {
		class Z80State {
		public:
			enum flags {
				SF = 0x80,
				ZF = 0x40,
				XF = 0x20,
				HF = 0x10,
				YF = 0x08,
				PF = 0x04,
				VF = 0x04,
				NF = 0x02,
				CF = 0x01
			};

			enum register8 {
				F = 0,
				A,
				C,
				B,
				E,
				D,
				L,
				H,
				IXl,
				IXh,
				IYl,
				IYh,
				R,
				I
			};

			enum register16 {
				AF = 0,
				BC,
				DE,
				HL,
				IX,
				IY,
				IR
			};

		protected:
			union {
				uint16_t word[11];	// AF, BC, DE, HL, IX, IY, AF', BC', DE', HL', IR
				uint8_t byte[22];
			} _pairs;
			uint16_t _sp;
			bool _alternative[4];

		public:
			// Registers 8 bits
			uint8_t a() const { return (_alternative[0]) ? _pairs.byte[register8::A + 12] : _pairs.byte[register8::A]; }
			uint8_t& a() { return (_alternative[0]) ? _pairs.byte[register8::A + 12] : _pairs.byte[register8::A]; }
			uint8_t f() const { return (_alternative[0]) ? _pairs.byte[register8::F + 12] : _pairs.byte[register8::F]; }
			uint8_t& f() { return (_alternative[0]) ? _pairs.byte[register8::F + 12] : _pairs.byte[register8::F]; }
			uint8_t b() const { return (_alternative[1]) ? _pairs.byte[register8::B + 12] : _pairs.byte[register8::B]; }
			uint8_t& b() { return (_alternative[1]) ? _pairs.byte[register8::B + 12] : _pairs.byte[register8::B]; }
			uint8_t c() const { return (_alternative[1]) ? _pairs.byte[register8::C + 12] : _pairs.byte[register8::C]; }
			uint8_t& c() { return (_alternative[1]) ? _pairs.byte[register8::C + 12] : _pairs.byte[register8::C]; }
			uint8_t d() const { return (_alternative[2]) ? _pairs.byte[register8::D + 12] : _pairs.byte[register8::D]; }
			uint8_t& d() { return (_alternative[2]) ? _pairs.byte[register8::D + 12] : _pairs.byte[register8::D]; }
			uint8_t e() const { return (_alternative[2]) ? _pairs.byte[register8::E + 12] : _pairs.byte[register8::E]; }
			uint8_t& e() { return (_alternative[2]) ? _pairs.byte[register8::E + 12] : _pairs.byte[register8::E]; }
			uint8_t h() const { return (_alternative[3]) ? _pairs.byte[register8::H + 12] : _pairs.byte[register8::H]; }
			uint8_t& h() { return (_alternative[3]) ? _pairs.byte[register8::H + 12] : _pairs.byte[register8::H]; }
			uint8_t l() const { return (_alternative[3]) ? _pairs.byte[register8::L + 12] : _pairs.byte[register8::L]; }
			uint8_t& l() { return (_alternative[3]) ? _pairs.byte[register8::L + 12] : _pairs.byte[register8::L]; }
			uint8_t ixh() const { return _pairs.byte[register8::IXh]; }
			uint8_t& ixh() { return _pairs.byte[register8::IXh]; }
			uint8_t ixl() const { return _pairs.byte[register8::IXl]; }
			uint8_t& ixl() { return _pairs.byte[register8::IXl]; }
			uint8_t iyh() const { return _pairs.byte[register8::IYh]; }
			uint8_t& iyh() { return _pairs.byte[register8::IYh]; }
			uint8_t iyl() const { return _pairs.byte[register8::IYl]; }
			uint8_t& iyl() { return _pairs.byte[register8::IYl]; }
			uint8_t i() const { return _pairs.byte[register8::I]; }
			uint8_t& i() { return _pairs.byte[register8::I]; }
			uint8_t r() const { return _pairs.byte[register8::R]; }
			uint8_t& r() { return _pairs.byte[register8::R]; }

			// Registers 16 bits
			uint16_t af() const { return (_alternative[0]) ? _pairs.word[register16::AF + 6] : _pairs.word[register16::AF]; }
			uint16_t& af() { return (_alternative[0]) ? _pairs.word[register16::AF + 6] : _pairs.word[register16::AF]; }
			uint16_t bc() const { return (_alternative[1]) ? _pairs.word[register16::BC + 6] : _pairs.word[register16::BC]; }
			uint16_t& bc() { return (_alternative[1]) ? _pairs.word[register16::BC + 6] : _pairs.word[register16::BC]; }
			uint16_t de() const { return (_alternative[2]) ? _pairs.word[register16::DE + 6] : _pairs.word[register16::DE]; }
			uint16_t& de() { return (_alternative[2]) ? _pairs.word[register16::DE + 6] : _pairs.word[register16::DE]; }
			uint16_t hl() const { return (_alternative[3]) ? _pairs.word[register16::HL + 6] : _pairs.word[register16::HL]; }
			uint16_t& hl() { return (_alternative[3]) ? _pairs.word[register16::HL + 6] : _pairs.word[register16::HL]; }
			uint16_t sp() const { return _sp; }
			uint16_t& sp() { return _sp; }
			uint16_t ix() const { return _pairs.word[register16::IX]; }
			uint16_t& ix() { return _pairs.word[register16::IX]; }
			uint16_t iy() const { return _pairs.word[register16::IY]; }
			uint16_t& iy() { return _pairs.word[register16::IY]; }
			uint16_t ir() const { return _pairs.word[register16::IR]; }
			uint16_t& ir() { return _pairs.word[register16::IR]; }

			// Flags
			bool parityFlag() const { return (f() & flags::PF) ? true : false; }
			bool carryFlag() const { return (f() & flags::CF) ? true : false; }
			bool signFlag() const { return (f() & flags::SF) ? true : false; }
			bool zeroFlag() const { return (f() & flags::ZF) ? true : false; }
			bool halfCarryFlag() const { return (f() & flags::HF) ? true : false; }
			bool addSubFlag() const { return (f() & flags::NF) ? true : false; }

			void resetFlags(const uint8_t flags) { f() &= ~flags; }
			void setFlags(const uint8_t flags) { f() |= flags; }
			void setSZXY(const uint8_t value) {
				resetFlags(flags::ZF | flags::XF | flags::SF | flags::YF);
				if (value == 0)
					setFlags(flags::ZF);
				else
					setFlags(value & (flags::XF | flags::SF | flags::YF));
			}
			void setSZXY(const uint16_t value) {
				resetFlags(flags::ZF | flags::XF | flags::SF | flags::YF);
				if (value == 0)
					setFlags(flags::ZF);
				else
					setFlags((value >> 8) & (flags::XF | flags::SF | flags::YF));
			}
			void setP(const uint8_t value) {
				uint8_t t = value;
				t ^= t >> 4;
				t ^= t >> 2;
				if ((t ^= t >> 1) & 1)
					resetFlags(flags::PF);
				else
					setFlags(flags::PF);
			}

			// Alternative registers
			void exchange(const register16 r) { _alternative[r] = (_alternative[r]) ? false : true; }

			// Reset state
			Z80State() {
				reset();
			}
			void reset() {
				for (int i = 0; i < 12; _pairs.word[i++] = 0);
				for (int i = 0; i < 4; _alternative[i++] = false);
				_sp = 0;
			}
		};
	}
}