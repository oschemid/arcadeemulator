#pragma once
#include "types.h"
#include "cpu.h"
#include "z80family/states.h"

#include <stdexcept>
#include <array>
#include <functional>

namespace ae
{
	namespace cpu
	{
		class LR35902 : public Cpu
		{
		protected:
			z80family::LR35902State _state;

			// Decode registers from opcode
			uint16_t decode16(const opcode_t, const bool stack = false) const;
			uint16_t& decode16(const opcode_t, const bool stack = false);
			uint8_t decode8(const opcode_t) const;
			uint8_t& decode8(const opcode_t);
			bool checkCondition3(const opcode_t) const;
			bool checkCondition2(const opcode_t) const;

			// Decode opcode
			void decode_opcode(const uint8_t);

			// (HL) <- fn((HL))
			void apply_hl(const fnuint8_t, const int8_t = 0);
			// r <- fn(r)
			void apply_r(const fnuint8_t, const opcode_t);

		protected:
			enum flags {
				zeroFlag = 0x40,
				parityFlag = 0x04,
			};
			bool iff1_waiting;
			bool iff1;
			bool iff2;
			bool halted;

			bool interrupt_waiting;
			uint8_t interrupt_request;
		public:
			// Temporary access
			uint16_t pc; // program counter
			uint8_t c() const { return _state.c(); }
			uint8_t e() const { return _state.e(); }
			uint16_t de() const { return _state.de(); }
			enum class interrupt_mode {
				mode_0,
				mode_1,
				mode_2
			} im;

		protected:
			/* opcodes */
			void rla(const bool);
			void rra(const bool);
			void daa();
			void add_ss(const opcode_t);
			void add(const uint8_t, const uint8_t = 0);
			void sub(const uint8_t, const uint8_t = 0);
			void ana(const uint8_t);
			void ora(const uint8_t);
			void xra(const uint8_t);
			void cp(const uint8_t);
			uint8_t inc(const uint8_t);
			uint8_t dec(const uint8_t);
			void exchange_de_hl();
			uint16_t exchange_sp(const uint16_t);
			void call(const uint16_t, const uint8_t = true);

			/* opcodes CB */
			uint8_t rlc(const uint8_t);
			uint8_t rl(const uint8_t);
			uint8_t rrc(const uint8_t);
			uint8_t rr(const uint8_t);
			uint8_t sla(const uint8_t);
			uint8_t sll(const uint8_t);
			uint8_t sra(const uint8_t);
			uint8_t srl(const uint8_t);
			void bit(const uint8_t, const uint8_t);

			/* opcodes ED */
			void rrd();
			void rld();
			void adc_ss(const opcode_t);
			void sbc_ss(const opcode_t);
			void neg();
			void cpd();
			void cpi();
			void ldd();
			void ldi();
			void in_c(const opcode_t);
			void ini();
			void ind();
			void out_c(const opcode_t);
			void outi();
			void outd();

			void unimplemented();
			void illegal();

			void pushToStack(const uint16_t);
			const uint16_t popOfStack();

			const uint8_t readOpcode() {
				_elapsed_cycles += 4;
				return _handlerRead(pc++);
			}
			const uint8_t readArgument8() {
				_elapsed_cycles += 4;
				return _handlerRead(pc++);
			}
			const uint16_t readArgument16() {
				_elapsed_cycles += 6;
				return _handlerRead(pc++) | (_handlerRead(pc++) << 8);
			}
			const uint8_t read8(const uint16_t address) {
				_elapsed_cycles += 4;
				return _handlerRead(address);
			}
			const uint16_t read16(const uint16_t address) {
				_elapsed_cycles += 6;
				return _handlerRead(address) | (_handlerRead(address + 1) << 8);
			}
			void write8(const uint16_t address, const uint8_t value) {
				_elapsed_cycles += 3;
				_handlerWrite(address, value);
			}
			void write16(const uint16_t address, const uint16_t value) {
				_elapsed_cycles += 6;
				_handlerWrite(address, value & 0xFF);
				_handlerWrite(address + 1, value >> 8);
			}

		public:
			LR35902();
			bool reset(const uint16_t = 0) override;

			const string disassemble() override;
			const uint8_t executeOne() override;
			bool interrupt(const uint8_t) override;
		};
	}
}
