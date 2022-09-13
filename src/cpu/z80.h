#pragma once
#include "types.h"
#include "cpu.h"
#include "z80/z80state.h"

#include <stdexcept>
#include <array>
#include <functional>

namespace ae
{
	namespace cpu
	{
		class Z80 : public Cpu
		{
		protected:
			Z80State _state;

			uint64_t _elapsed_cycles;

			enum prefix {
				NO = 0,
				DD,
				FD
			};

			// Decode registers from opcode
			uint16_t decode16(const opcode_t, const prefix = NO) const;
			uint16_t& decode16(const opcode_t, const prefix = NO);
			uint8_t decode8(const opcode_t, const prefix = NO) const;
			uint8_t& decode8(const opcode_t, const prefix = NO);

			// Decode opcode
			uint16_t decode_opcode(const uint8_t, const prefix = NO);
			void decode_opcode_cb(const prefix);
			uint16_t decode_opcode_ed();

			// (HL) <- fn((HL))
			void apply_hl(const fnuint8_t, const prefix, const int8_t = 0);
			// r <- fn(r)
			void apply_r(const fnuint8_t, const opcode_t, const prefix);
			// (IXY+d) <- r <- fn((IXY+d))
			void apply_ixy_r(const fnuint8_t, const opcode_t, const prefix, const int8_t = 0);

			// Flags updater
			void setZSP(const uint8_t);

		protected:
			enum flags {
				signFlag = 0x80,
				zeroFlag = 0x40,
				halfCarryFlag = 0x10,
				parityFlag = 0x04,
				addSubFlag = 0x02,
				carryFlag = 0x01
			};
			bool iff1_waiting;
			bool iff1;
			bool iff2;
			bool halted;

			bool interrupt_waiting;
			uint8_t interrupt_request;
		public:
			const uint64_t elapsed_cycles() const { return _elapsed_cycles; }

			// Temporary access
			uint16_t pc; // program counter
			uint8_t c() const { return _state.c(); }
			uint8_t e() const { return _state.e(); }
			uint16_t de() const { return _state.de(); }
			uint8_t i;
			enum class interrupt_mode {
				mode_0,
				mode_1,
				mode_2
			} im;

		protected:
			/* opcodes */

			uint16_t ldd();
			uint16_t ldi();

			/* opcodes */
			void rla(const bool);
			void rra(const bool);
			void daa();
			void add_ss(const opcode_t, const prefix);
			void add(const uint8_t, const uint8_t = 0);
			void sub(const uint8_t, const uint8_t = 0);
			void ana(const uint8_t);
			void ora(const uint8_t);
			void xra(const uint8_t);
			void cp(const uint8_t);
			uint8_t inc(const uint8_t);
			uint8_t dec(const uint8_t);

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

			uint16_t decode_dd_opcode(const uint8_t);
			uint16_t decode_ed_opcode(const uint8_t);
			uint16_t decode_fd_opcode(const uint8_t);

			const uint8_t decode_flags(const uint8_t) const;
			uint8_t& decode_register(const uint8_t);

			uint16_t execute_call(const uint16_t, const uint8_t);

			uint8_t get_m() const;

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
				return _handlerRead(pc++) | (_handlerRead(pc++) << 8);
			}
			const uint8_t read(const uint16_t address) {
				_elapsed_cycles += 4;
				return _handlerRead(address);
			}
			void write(const uint16_t address, const uint8_t value) {
				_elapsed_cycles += 3;
				_handlerWrite(address, value);
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
