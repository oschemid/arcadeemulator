#pragma once
#include "types.h"
#include "cpu.h"

#include <cstdint>


namespace ae
{
	namespace cpu
	{
		class Intel8080 : public ICpu
		{
		protected:
			uint8_t interrupt_enabled; // 0 ok, 1 wait one, 2 no
			uint8_t interrupt_request;

			uint8_t carryBit : 1; // carry bit
			uint8_t auxCarryBit : 1; // auxiliary carry bit
			uint8_t signBit : 1; // sign bit
			uint8_t zeroBit : 1; // zero bit
			uint8_t parityBit : 1; // parity bit

		public:
			// Temporary access
			std::uint16_t pc; // program counter
			std::uint16_t sp; // stack pointer
			std::uint8_t a; // accumulator
			std::uint8_t b, c, d, e, h, l; // registers

		protected:
			ae::memory* memory;

			ICpu::infn handlerIn;
			ICpu::outfn handlerOut;

		protected:
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
				return memory->read(pc++);
			}
			const uint8_t readArgument8() {
				return memory->read(pc++);
			}
			const uint16_t readArgument16() {
				return memory->read(pc++) | (memory->read(pc++) << 8);
			}
			void write(const uint16_t address, const uint16_t value) {
				memory->write(address, value & 0xFF);
				memory->write(address + 1, value >> 8);
			}

		public:
			Intel8080();
			bool reset(const uint16_t = 0) override;
			bool link(ae::memory&) override;

			const string disassemble() override;
			const uint8_t executeOne() override;
			bool interrupt(const uint8_t) override;
			virtual bool in(const ICpu::infn) override;
			virtual bool out(const ICpu::outfn) override;


			std::uint8_t outPort[8];
			std::uint8_t inPort[8];
		};
	}
}
