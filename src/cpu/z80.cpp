#include "z80.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <format>

namespace ae::cpu {
	Z80::Z80() :
		Cpu() {
		reset();
	}

	uint16_t Z80::get_hl() const {
		return (main_registers.h << 8) | main_registers.l;
	}
	uint8_t Z80::get_m() const {
		return _handlerRead(get_hl());
	}

	void Z80::add(const uint8_t value, const uint8_t flag)
	{
		uint16_t sum = main_registers.a + value + flag;
		main_registers.f = 0;
		main_registers.f |= (((value & 0x0f) + (main_registers.a & 0x0f) + flag) > 0x0f) ? flags::halfCarryFlag : 0;
		main_registers.a = sum & 0xff;
		main_registers.f |= (parity(main_registers.a)) ? flags::parityFlag : 0;
		main_registers.f |= (sign(main_registers.a)) ? flags::signFlag : 0;
		main_registers.f |= (zero(main_registers.a)) ? flags::zeroFlag : 0;
		main_registers.f |= (sum > 0xff) ? flags::carryFlag : 0;
	}
	void Z80::ana(const uint8_t value) {
		uint8_t result = main_registers.a & value;
		main_registers.f = 0;
		main_registers.f |= (parity(result)) ? flags::parityFlag : 0;
		main_registers.f |= (sign(result)) ? flags::signFlag : 0;
		main_registers.f |= (zero(result)) ? flags::zeroFlag : 0;
		main_registers.f |= flags::halfCarryFlag;
		main_registers.a = result;
	}

	void Z80::cmp(const uint8_t value) {
		main_registers.f = 0;
		main_registers.f |= (value > main_registers.a) ? flags::carryFlag : 0;
		main_registers.f |= (value & 0xf) > (main_registers.a & 0xf) ? 0 : flags::halfCarryFlag;
		uint8_t r = main_registers.a - value;
		main_registers.f |= (parity(r)) ? flags::parityFlag : 0;
		main_registers.f |= (sign(r)) ? flags::signFlag : 0;
		main_registers.f |= (zero(r)) ? flags::zeroFlag : 0;
	}

	void Z80::xra(const uint8_t value) {
		main_registers.a ^= value;
		main_registers.f = 0;
		main_registers.f |= (parity(main_registers.a)) ? flags::parityFlag : 0;
		main_registers.f |= (sign(main_registers.a)) ? flags::signFlag : 0;
		main_registers.f |= (zero(main_registers.a)) ? flags::zeroFlag : 0;
	}
	uint8_t Z80::inr(const uint8_t value) {
		uint8_t result = value + 1;
		main_registers.f = 0;
		main_registers.f |= (parity(result)) ? flags::parityFlag : 0;
		main_registers.f |= (sign(result)) ? flags::signFlag : 0;
		main_registers.f |= (zero(result)) ? flags::zeroFlag : 0;
		main_registers.f |= ((result & 0x0f) == 0) ? flags::halfCarryFlag : 0;
		return result;
	}

	void Z80::unimplemented()
	{
		pc--;
		auto opcode = readOpcode();
		std::cout << std::hex << int(opcode) << std::endl;
		pc--;
		std::cout << disassemble();
		throw std::runtime_error("Instruction not implemented");
	}
	void Z80::illegal()
	{
		pc--;
		auto opcode = readOpcode();
		throw std::runtime_error("Illegal instruction " + opcode);
	}

	const string Z80::disassemble() {
		std::stringstream stream;
		stream << std::setfill('0') << std::setw(4) << std::hex << pc << ": ";

		const std::uint8_t opcode = readOpcode();
		switch (opcode) {
		case 0xF3:
			stream << "DI";
			break;
		default:
			stream << "illegal opcode " << std::hex << int(opcode);
			break;
		}
		stream << std::endl;
		return stream.str();
	}

	const uint8_t Z80::executeOne() {
		uint16_t cycle = 0;
		if (iff1) {
			if (iff1_waiting)
				iff1_waiting = false;
			else {
				if (interrupt_waiting) {
					interrupt_waiting = false;
					iff1 = false;
					iff2 = false;
					halted = false;
					switch (im) {
					case interrupt_mode::mode_2:
						pc = (i << 8) | interrupt_request;
						pc = readArgument16();
						cycle = 19;
						break;
					defaut:
						return 0;
					}
				}
			}
			return cycle;
		}
		const std::uint8_t opcode = (halted) ? 0x00 : readOpcode();
		uint8_t tmp8 = 0;
		uint16_t tmp16 = 0;


		switch (opcode) {
		case 0x00: /* NOP */
			cycle = 4;
			break;
		case 0x01: /* LD BC, NN */
			main_registers.c = readArgument8();
			main_registers.b = readArgument8();
			cycle = 10;
			break;
		case 0x06: /* LD B, n */
			main_registers.b = readArgument8();
			cycle = 7;
			break;
		case 0x08: /* EX AF, AF' */
			tmp8 = alternative_registers.a;
			alternative_registers.a = main_registers.a;
			main_registers.a = tmp8;
			tmp8 = alternative_registers.f;
			alternative_registers.f = main_registers.f;
			main_registers.f = tmp8;
			cycle = 4;
			break;
		case 0x10: /* DJNZ e */
		{
			uint8_t e = readArgument8();
			if (--main_registers.b == 0) {
				cycle = 8;
			}
			else {
				pc = pc + static_cast<signed char>(e);
				cycle = 13;
			}}
		break;
		case 0x20: /* JR NZ, e */
		{
			uint8_t e = readArgument8();
			if (main_registers.f & flags::zeroFlag) {
				cycle = 7;
			}
			else {
				pc = pc + static_cast<signed char>(e);
				cycle = 12;
			}
			break;
		}
		case 0x21: /* LD HL, nn */
			main_registers.l = readArgument8();
			main_registers.h = readArgument8();
			cycle = 10;
			break;
		case 0x24: /* INC H */
			main_registers.h = inr(main_registers.h);
			cycle = 4;
			break;
		case 0x26: /* LD H, n */
			main_registers.h = readArgument8();
			cycle = 7;
			break;
		case 0x2C: /* INC L */
			main_registers.l = inr(main_registers.l);
			cycle = 4;
			break;
		case 0x30: /* JR NC, e */
		{
			uint8_t e = readArgument8();
			if (main_registers.f & flags::carryFlag) {
				cycle = 7;
			}
			else {
				pc = pc + static_cast<signed char>(e);
				cycle = 12;
			}
			break;
		}
		case 0x32: /* LD (nn), A */
			_handlerWrite(readArgument16(), main_registers.a);
			cycle = 13;
			break;
		case 0x3C: /* INC A */
			main_registers.a = inr(main_registers.a);
			cycle = 4;
			break;
		case 0x3E: /* LD A, n */
			main_registers.a = readArgument8();
			cycle = 7;
			break;
		case 0x4F: /* LD C, A */
			main_registers.c = main_registers.a;
			cycle = 4;
			break;
		case 0x6F: /* LD L,A */
			main_registers.l = main_registers.a;
			cycle = 4;
			break;
		case 0x76: /* HALT */
			halted = true;
			cycle = 4;
			break;
		case 0x77: /* LD (HL),A */
			_handlerWrite(make16(main_registers.h, main_registers.l), main_registers.a);
			cycle = 7;
			break;
		case 0x79: /* LD A,C */
			main_registers.a = main_registers.c;
			cycle = 4;
			break;
		case 0x7C: /* LD A,H */
			main_registers.a = main_registers.h;
			cycle = 4;
			break;
		case 0x7D: /* LD A,L */
			main_registers.a = main_registers.l;
			cycle = 4;
			break;
		case 0x86: /* ADD A, (HL) */
			add(get_m());
			cycle = 7;
			break;
		case 0xA7: /* AND A */
			ana(main_registers.a);
			cycle = 4;
			break;
		case 0xAF: /* XRA A */
			xra(main_registers.a);
			cycle = 4;
			break;
		case 0xC2: /* JP NZ, e */
			tmp16 = readArgument16();
			if (!(main_registers.f & flags::zeroFlag)) {
				pc = tmp16;
			}
			cycle = 10;
			break;
		case 0xC3: /* JMP */
			pc = readArgument16();
			cycle = 10;
			break;
		case 0xC6: /* ADD A,n */
			add(readArgument8());
			cycle = 7;
			break;
		case 0xD2: /* JP NC, e*/
			tmp16 = readArgument16();
			if (!(main_registers.f & flags::carryFlag)) {
				pc = tmp16;
			}
			cycle = 10;
			break;
		case 0xD3: /* OUT (N), A */
			_handlerOut(readArgument8(), main_registers.a);
			cycle = 11;
			break;
		case 0xDA: /* JP C, e */
			tmp16 = readArgument16();
			if (main_registers.f & flags::carryFlag) {
				pc = tmp16;
			}
			cycle = 10;
			break;
		case 0xED: /* ED */
			cycle = decode_ed_prefix();
			break;
		case 0xF3: /* DI */
			iff1 = false;
			iff2 = false;
			cycle = 4;
			break;
		case 0xFB: /* EI */
			iff1 = true;
			iff2 = true;
			iff1_waiting = true;
			cycle = 4;
			break;
		case 0xFE: /* CP n */
			cmp(readArgument8());
			cycle = 7;
			break;
		default: unimplemented(); break;
		}
		return cycle;
	}

	uint16_t Z80::decode_ed_prefix() {
		uint16_t cycle = 0;
		const std::uint8_t opcode = readOpcode();

		switch (opcode) {
		case 0x47: /* LD I,A */
			i = main_registers.a;
			cycle = 9;
			break;
		case 0x5E: /* IM 2 */
			im = interrupt_mode::mode_2;
			cycle = 8;
			break;
		default: unimplemented(); break;
		}
		return cycle;
	}
	bool Z80::interrupt(const uint8_t inte) {
		if (iff1) {
			interrupt_waiting = true;
			interrupt_request = inte;
		}
		return true;
	}

	bool Z80::reset(const uint16_t address) {
		iff1 = false;
		iff2 = false;
		iff1_waiting = false;
		im = interrupt_mode::mode_0;
		i = 0;
		halted = false;
		pc = address;
		main_registers.a = 0;
		main_registers.f = 0;
		main_registers.b = 0;
		main_registers.c = 0;
		main_registers.d = 0;
		main_registers.e = 0;
		main_registers.h = 0;
		main_registers.l = 0;
		interrupt_waiting = false;
		return true;
	}
}

