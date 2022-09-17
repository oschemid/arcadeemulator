#include "../z80.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <format>
#include <array>


using namespace ae::cpu;


namespace ae::cpu {
	Z80::Z80() :
		Cpu() {
		reset();
	}

	// Function : (HL) <- fn((HL)) or (IX+d) <- fn((IX+d)) or (IY+d) <- fn((IY+d))
	void Z80::apply_hl(const fnuint8_t fn, const prefix p, const int8_t d) {
		switch (p) {
		case NO:
			write8(_state.hl(), fn(read8(_state.hl())));
			break;
		case DD:
			write8(_state.ix() + d, fn(read8(_state.ix() + d)));
			break;
		case FD:
			write8(_state.iy() + d, fn(read8(_state.iy() + d)));
			break;
		}
	}

	void Z80::apply_r(const fnuint8_t fn, const opcode_t opcode, const prefix p) {
		decode8(opcode, p) = fn(decode8(opcode, p));
	}

	void Z80::apply_ixy_r(const fnuint8_t fn, const opcode_t opcode, const prefix p, const int8_t d) {
		switch (p) {
		case NO:
			decode8(opcode) = fn(decode8(opcode));
			break;
		case DD:
			decode8(opcode) = fn(read8(_state.ix() + d));
			write8(_state.ix() + d, decode8(opcode));
			break;
		case FD:
			decode8(opcode) = fn(read8(_state.iy() + d));
			write8(_state.iy() + d, decode8(opcode));
			break;
		}
	}

	/******************************************************************************/
	const uint16_t Z80::popOfStack()
	{
		uint16_t value = read16(_state.sp());
		_state.sp() += 2;
		return value;
	}
	void Z80::pushToStack(const uint16_t value)
	{
		_state.sp() -= 2;
		write16(_state.sp(), value);
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
						pushToStack(pc);
						pc = (_state.i() << 8) | interrupt_request;
						pc = readArgument16();
						cycle = 19;
						break;
					default:
						return 0;
					}
					return cycle;
				}
			}
		}

		const std::uint8_t opcode = (halted) ? 0x00 : readOpcode();
		decode_opcode(opcode);
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
		_state.reset();

		_elapsed_cycles = 0;

		iff1 = false;
		iff2 = false;
		iff1_waiting = false;
		im = interrupt_mode::mode_0;
		halted = false;
		pc = address;
		interrupt_waiting = false;
		return true;
	}
}


uint16_t Z80::decode16(const opcode_t opcode, const prefix p, const bool stack) const
{
	switch ((opcode & 0x30) | p) {
	case 0x00:
	case 0x01:
	case 0x02:
		return _state.bc();
	case 0x10:
	case 0x11:
	case 0x12:
		return _state.de();
	case 0x20:
		return _state.hl();
	case 0x21:
		return _state.ix();
	case 0x22:
		return _state.iy();
	case 0x30:
	case 0x31:
	case 0x32:
		return (stack) ? _state.af() : _state.sp();
	}
	throw std::runtime_error("Unexpected opcode in decode16 " + opcode);
}
uint16_t& Z80::decode16(const opcode_t opcode, const prefix p, const bool stack)
{
	switch ((opcode & 0x30) | p) {
	case 0x00:
	case 0x01:
	case 0x02:
		return _state.bc();
	case 0x10:
	case 0x11:
	case 0x12:
		return _state.de();
	case 0x20:
		return _state.hl();
	case 0x21:
		return _state.ix();
	case 0x22:
		return _state.iy();
	case 0x30:
	case 0x31:
	case 0x32:
		return (stack) ? _state.af() : _state.sp();
	}
	throw std::runtime_error("Unexpected opcode in decode16 " + opcode);
}
uint8_t Z80::decode8(const opcode_t opcode, const prefix p) const
{
	switch ((opcode & 0x07) | (p << 4)) {
	case 0x00:
	case 0x10:
	case 0x20:
		return _state.b();
	case 0x01:
	case 0x11:
	case 0x21:
		return _state.c();
	case 0x02:
	case 0x12:
	case 0x22:
		return _state.d();
	case 0x03:
	case 0x13:
	case 0x23:
		return _state.e();
	case 0x04:
		return _state.h();
	case 0x14:
		return _state.ixh();
	case 0x24:
		return _state.iyh();
	case 0x05:
		return _state.l();
	case 0x15:
		return _state.ixl();
	case 0x25:
		return _state.iyl();
	case 0x07:
	case 0x17:
	case 0x27:
		return _state.a();
	}
	throw std::runtime_error("Unexpected opcode in decode8 " + opcode);
}
uint8_t& Z80::decode8(const opcode_t opcode, const prefix p)
{
	switch ((opcode & 0x07) | (p << 4)) {
	case 0x00:
	case 0x10:
	case 0x20:
		return _state.b();
	case 0x01:
	case 0x11:
	case 0x21:
		return _state.c();
	case 0x02:
	case 0x12:
	case 0x22:
		return _state.d();
	case 0x03:
	case 0x13:
	case 0x23:
		return _state.e();
	case 0x04:
		return _state.h();
	case 0x14:
		return _state.ixh();
	case 0x24:
		return _state.iyh();
	case 0x05:
		return _state.l();
	case 0x15:
		return _state.ixl();
	case 0x25:
		return _state.iyl();
	case 0x07:
	case 0x17:
	case 0x27:
		return _state.a();
	}
	throw std::runtime_error("Unexpected opcode in decode8 " + opcode);
}
bool Z80::checkCondition3(const opcode_t opcode) const
{
	switch (opcode & 0b00111000) {
	case 0b00000000:
		return !_state.zeroFlag();
	case 0b00001000:
		return _state.zeroFlag();
	case 0b00010000:
		return !_state.carryFlag();
	case 0b00011000:
		return _state.carryFlag();
	case 0b00100000:
		return !_state.parityFlag();
	case 0b00101000:
		return _state.parityFlag();
	case 0b00110000:
		return !_state.signFlag();
	case 0b00111000:
		return _state.signFlag();
	}
	throw std::runtime_error("checkCondition3 : illegal value " + opcode);
}
bool Z80::checkCondition2(const opcode_t opcode) const
{
	switch (opcode & 0b00011000) {
	case 0b00000000:
		return !_state.zeroFlag();
	case 0b00001000:
		return _state.zeroFlag();
	case 0b00010000:
		return !_state.carryFlag();
	case 0b00011000:
		return _state.carryFlag();
	}
	throw std::runtime_error("checkCondition2 : illegal value " + opcode);
}
