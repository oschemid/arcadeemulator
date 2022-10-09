#pragma once

#include "types.h"

namespace ae {
	namespace cpu {
		namespace z80family {
			template<typename F> class BaseState {
				enum register8 {
					F = 0,
					A,
					C,
					B,
					E,
					D,
					L,
					H
				};

				enum register16 {
					AF = 0,
					BC,
					DE,
					HL
				};

			protected:
				union {
					uint16_t word[4];	// AF, BC, DE, HL
					uint8_t byte[8];
				} _pairs;
				uint16_t _sp;

			public:
				// Registers 8 bits
				uint8_t a() const { return _pairs.byte[register8::A]; }
				uint8_t& a() { return _pairs.byte[register8::A]; }
				uint8_t f() const { return _pairs.byte[register8::F]; }
				uint8_t& f() { return _pairs.byte[register8::F]; }
				uint8_t b() const { return _pairs.byte[register8::B]; }
				uint8_t& b() { return _pairs.byte[register8::B]; }
				uint8_t c() const { return _pairs.byte[register8::C]; }
				uint8_t& c() { return _pairs.byte[register8::C]; }
				uint8_t d() const { return _pairs.byte[register8::D]; }
				uint8_t& d() { return _pairs.byte[register8::D]; }
				uint8_t e() const { return _pairs.byte[register8::E]; }
				uint8_t& e() { return _pairs.byte[register8::E]; }
				uint8_t h() const { return _pairs.byte[register8::H]; }
				uint8_t& h() { return _pairs.byte[register8::H]; }
				uint8_t l() const { return _pairs.byte[register8::L]; }
				uint8_t& l() { return _pairs.byte[register8::L]; }

				// Registers 16 bits
				uint16_t af() const { return _pairs.word[register16::AF]; }
				uint16_t& af() { return _pairs.word[register16::AF]; }
				uint16_t bc() const { return _pairs.word[register16::BC]; }
				uint16_t& bc() { return _pairs.word[register16::BC]; }
				uint16_t de() const { return _pairs.word[register16::DE]; }
				uint16_t& de() { return _pairs.word[register16::DE]; }
				uint16_t hl() const { return _pairs.word[register16::HL]; }
				uint16_t& hl() { return _pairs.word[register16::HL]; }
				uint16_t sp() const { return _sp; }
				uint16_t& sp() { return _sp; }

				// Flags
				bool parityFlag() const { return (f() & F::PF) ? true : false; }
				bool carryFlag() const { return (f() & F::CF) ? true : false; }
				bool signFlag() const { return (f() & F::SF) ? true : false; }
				bool zeroFlag() const { return (f() & F::ZF) ? true : false; }
				bool halfCarryFlag() const { return (f() & F::HF) ? true : false; }
				bool addSubFlag() const { return (f() & F::NF) ? true : false; }

				void resetFlags(const uint8_t flags) { f() &= ~flags; }
				void setFlags(const uint8_t flags) { f() |= flags; }
				void setSZXY(const uint8_t value) {
					resetFlags(F::ZF | F::XF | F::SF | F::YF);
					if (value == 0)
						setFlags(F::ZF);
					else
						setFlags(value & (F::XF | F::SF | F::YF));
				}
				void setSZXY(const uint16_t value) {
					resetFlags(F::ZF | F::XF | F::SF | F::YF);
					if (value == 0)
						setFlags(F::ZF);
					else
						setFlags((value >> 8) & (F::XF | F::SF | F::YF));
				}
				void setP(const uint8_t value) {
					uint8_t t = value;
					t ^= t >> 4;
					t ^= t >> 2;
					if ((t ^= t >> 1) & 1)
						resetFlags(F::PF);
					else
						setFlags(F::PF);
				}

				BaseState() {
					reset();
				}
				void reset() {
					for (int i = 0; i < 4; _pairs.word[i++] = 0);
					_sp = 0;
				}
			};

			enum LR35902Flags {
				CF = 0x10
			};

			class LR35902State : public BaseState<LR35902Flags> {

			};
		}
	}
}