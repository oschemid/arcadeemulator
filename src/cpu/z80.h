#pragma once
#include "types.h"
#include "cpu.h"


namespace ae
{
	namespace cpu
	{
		class Z80 : public Cpu
		{
		protected:
			enum flags {
				signFlag = 0x80,
				zeroFlag = 0x40,
				halfCarryFlag = 0x10,
				parityFlag = 0x04,
				addSubFlag = 0x02,
				carryFlag = 0x01
			};
			typedef struct {
				uint8_t a;
				uint8_t f;
				uint8_t b;
				uint8_t c;
				uint8_t d;
				uint8_t e;
				uint8_t h;
				uint8_t l;
			} registers_t;
			bool iff1_waiting;
			bool iff1;
			bool iff2;
			bool halted;

			bool interrupt_waiting;
			uint8_t interrupt_request;

			//			uint8_t carryBit : 1; // carry bit
			//			uint8_t auxCarryBit : 1; // auxiliary carry bit
			//			uint8_t signBit : 1; // sign bit
			//			uint8_t zeroBit : 1; // zero bit
			//			uint8_t parityBit : 1; // parity bit

			registers_t main_registers;
			registers_t alternative_registers;
		public:
			// Temporary access
			uint16_t pc; // program counter
			uint16_t sp; // stack pointer
//			uint8_t a; // accumulator
//			uint8_t b, c, d, e, h, l; // registers
			uint8_t i;
			enum class interrupt_mode {
				mode_0,
				mode_1,
				mode_2
			} im;

		protected:
			uint16_t decode_ed_prefix();

			uint16_t get_bc() const;
			void set_bc(const uint16_t);
			uint16_t get_de() const;
			void set_de(const uint16_t);
			uint16_t get_hl() const;
			void set_hl(const uint16_t);
			uint8_t get_m() const;

			uint8_t dcr(const uint8_t);
			uint8_t inr(const uint8_t);
			void xra(const uint8_t);
			void ana(const uint8_t);
			void ora(const uint8_t);
			void sub(const uint8_t, const uint8_t = 0);
			void sbb(const uint8_t);
			void add(const uint8_t, const uint8_t = 0);
			void adc(const uint8_t);
			void cmp(const uint8_t);
			void daa();
			void dad(const uint16_t);

			void unimplemented();
			void illegal();

			void pushToStack(const uint16_t);
			const uint16_t popOfStack();

			const uint8_t readOpcode() {
				return _handlerRead(pc++);
			}
			const uint8_t readArgument8() {
				return _handlerRead(pc++);
			}
			const uint16_t readArgument16() {
				return _handlerRead(pc++) | (_handlerRead(pc++) << 8);
			}
			void write(const uint16_t address, const uint16_t value) {
				_handlerWrite(address, value & 0xFF);
				_handlerWrite(address + 1, value >> 8);
			}

		public:
			Z80();
			bool reset(const uint16_t = 0) override;

			const string disassemble() override;
			const uint8_t executeOne() override;
			bool interrupt(const uint8_t) override;
		};
	}
}
