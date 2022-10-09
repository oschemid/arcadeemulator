#include "i8080.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <format>

using ae::Cpu;

namespace ae::cpu {
	Intel8080::Intel8080() :
		Cpu() {
		reset();
	}
	uint8_t Intel8080::get_m() const {
		return _handlerRead(_state.hl());
	}

	uint8_t Intel8080::dcr(const uint8_t value) {
		uint8_t result = value - 1;

		zeroBit = zero(result);
		signBit = sign(result);
		parityBit = parity(result);
		auxCarryBit = !((result & 0x0f) == 0x0f);
		return result;
	}
	uint8_t Intel8080::inr(const uint8_t value) {
		uint8_t result = value + 1;
		zeroBit = zero(result);
		signBit = sign(result);
		parityBit = parity(result);
		auxCarryBit = (result & 0x0f) == 0;
		return result;
	}

	void Intel8080::xra(const uint8_t value) {
		_state.a() ^= value;
		_state.resetFlags(Intel8080Flags::CF);
		//		carryBit = 0;
		auxCarryBit = 0;
		parityBit = parity(_state.a());
		signBit = sign(_state.a());
		zeroBit = zero(_state.a());
	}
	void Intel8080::ora(const uint8_t value) {
		_state.a() |= value;
		_state.resetFlags(Intel8080Flags::CF);
		//		carryBit = 0;
		auxCarryBit = 0;
		parityBit = parity(_state.a());
		signBit = sign(_state.a());
		zeroBit = zero(_state.a());
	}
	void Intel8080::ana(const uint8_t value) {
		uint8_t result = _state.a() & value;
		_state.resetFlags(Intel8080Flags::CF);
		//		carryBit = 0;
		parityBit = parity(result);
		signBit = sign(result);
		zeroBit = zero(result);
		auxCarryBit = ((_state.a() | value) & 0x08) != 0;
		_state.a() = result;
	}
	void Intel8080::sub(const uint8_t value, const uint8_t flag) {
		if (value + flag > _state.a())
			_state.setFlags(Intel8080Flags::CF);
		else
			_state.resetFlags(Intel8080Flags::CF);
		//		carryBit = (value + flag > _state.a()) ? 1 : 0;
		auxCarryBit = (_state.a() & 0x0f) - (value & 0x0f) - flag >= 0;
		_state.a() -= value + flag;
		signBit = sign(_state.a());
		zeroBit = zero(_state.a());
		parityBit = parity(_state.a());
	}
	void Intel8080::sbb(const uint8_t value) {
		sub(value, _state.carryFlag() ? 1 : 0);
	}
	void Intel8080::cmp(const uint8_t value) {
		if (value > _state.a())
			_state.setFlags(Intel8080Flags::CF);
		else
			_state.resetFlags(Intel8080Flags::CF);
		//		carryBit = (value > _state.a()) ? 1 : 0;
		auxCarryBit = (value & 0xf) > (_state.a() & 0xf) ? 0 : 1;
		uint8_t r = _state.a() - value;
		signBit = sign(r);
		zeroBit = zero(r);
		parityBit = parity(r);
	}
	void Intel8080::add(const uint8_t value, const uint8_t flag)
	{
		uint16_t sum = _state.a() + value + flag;
		auxCarryBit = ((value & 0x0f) + (_state.a() & 0x0f) + flag) > 0x0f;
		_state.a() = sum & 0xff;
		zeroBit = zero(_state.a());
		signBit = sign(_state.a());
		parityBit = parity(_state.a());
		if (sum > 0xff)
			_state.setFlags(Intel8080Flags::CF);
		else
			_state.resetFlags(Intel8080Flags::CF);
		//		carryBit = (sum > 0xff);
	}
	void Intel8080::adc(const uint8_t value)
	{
		add(value, _state.carryFlag() ? 1 : 0);
	}
	void Intel8080::daa()
	{
		uint16_t temp = _state.a();
		if (((_state.a() & 0xf) > 9) || (auxCarryBit)) {
			temp += 6;
			auxCarryBit = ((_state.a() & 0xf) > 9) ? 1 : 0;
		}
		if ((temp >> 4 > 9) || (_state.carryFlag())) {
			temp += 0x60;
			_state.setFlags(Intel8080Flags::CF);
		}
		_state.a() = temp & 0xff;
		signBit = sign(_state.a());
		parityBit = parity(_state.a());
		zeroBit = zero(_state.a());
	}
	void Intel8080::dad(const uint16_t value)
	{
		uint32_t res = _state.hl() + value;
		_state.h() = res >> 8;
		_state.l() = res & 0xff;
		if ((res & 0xffff0000) != 0)
			_state.setFlags(Intel8080Flags::CF);
		else
			_state.resetFlags(Intel8080Flags::CF);
		//		carryBit = (res & 0xffff0000) != 0;
	}

	const string Intel8080::disassemble() {
		std::stringstream stream;
		stream << std::setfill('0') << std::setw(4) << std::hex << pc << ": ";

		const std::uint8_t opcode = readOpcode();
		switch (opcode) {
		case 0x00:
			stream << "NOP";
			break;
		case 0x01:
			stream << std::format("LXI B,{:04X}", readArgument16());
			break;
		case 0x02:
			stream << "STAX B";
			break;
		case 0x03:
			stream << "INX B";
			break;
		case 0x04:
			stream << "INR B";
			break;
		case 0x05:
			stream << "DCR B";
			break;
		case 0x06:
			stream << std::format("MVI B, {:02X}", readArgument8());
			break;
		case 0x07:
			stream << "RLC";
			break;
		case 0x09:
			stream << "DAD BC";
			break;
		case 0x0A:
			stream << "LDAX B";
			break;
		case 0x0B:
			stream << "DCX B";
			break;
		case 0x0C:
			stream << "INR C";
			break;
		case 0x0D:
			stream << "DCR C";
			break;
		case 0x0E:
			stream << std::format("MVI C,{:02X}", readArgument8());
			break;
		case 0x0F:
			stream << "RRC";
			break;
		case 0x11:
			stream << std::format("LXI D, {:04X}", readArgument16());
			break;
		case 0x12:
			stream << "STAX D";
			break;
		case 0x13:
			stream << "INX D";
			break;
		case 0x14:
			stream << "INR D";
			break;
		case 0x15:
			stream << "DCR D";
			break;
		case 0x16:
			stream << std::format("MVI D, {:02X}", readArgument8());
			break;
		case 0x17:
			stream << "RAL";
			break;
		case 0x19:
			stream << "DAD D";
			break;
		case 0x1A:
			stream << "LDAX D";
			break;
		case 0x1B:
			stream << "DCX D";
			break;
		case 0x1C:
			stream << "INR E";
			break;
		case 0x1D:
			stream << "DCR E";
			break;
		case 0x1E:
			stream << std::format("MVI E, {:02X}", readArgument8());
			break;
		case 0x1F:
			stream << "RAR";
			break;
		case 0x21:
			stream << std::format("LXI H, {:04X}", readArgument16());
			break;
		case 0x22:
			stream << std::format("SHLD {:04X}", readArgument16());
			break;
		case 0x23:
			stream << "INX H";
			break;
		case 0x24:
			stream << "INR H";
			break;
		case 0x25:
			stream << "DCR H";
			break;
		case 0x26:
			stream << std::format("MVI H,{:02X}", readArgument8());
			break;
		case 0x27:
			stream << "DAA";
			break;
		case 0x29:
			stream << "DAD HL";
			break;
		case 0x2A:
			stream << std::format("LHLD {:04X}", readArgument16());
			break;
		case 0x2B:
			stream << "DCX H";
			break;
		case 0x2C:
			stream << "INR L";
			break;
		case 0x2D:
			stream << "DCR L";
			break;
		case 0x2E:
			stream << std::format("MVI L, {:02X}", readArgument8());
			break;
		case 0x2F:
			stream << "CMA";
			break;
		case 0x31:
			stream << std::format("LXI SP, {:04X}", readArgument16());
			break;
		case 0x32:
			stream << std::format("STA {:04X}", readArgument16());
			break;
		case 0x33:
			stream << "INX SP";
			break;
		case 0x34:
			stream << "INR M";
			break;
		case 0x35:
			stream << "DCR M";
			break;
		case 0x36:
			stream << std::format("MVI M,{:02X}", readArgument8());
			break;
		case 0x37:
			stream << "STC";
			break;
		case 0x39:
			stream << "DAD SP";
			break;
		case 0x3A:
			stream << std::format("LDA {:04X}", readArgument16());
			break;
		case 0x3B:
			stream << "DCX SP";
			break;
		case 0x3C:
			stream << "INR A";
			break;
		case 0x3D:
			stream << "DCR A";
			break;
		case 0x3E:
			stream << std::format("MVI A,{:02X}", readArgument8());
			break;
		case 0x3F:
			stream << "CMC";
			break;
		case 0x40:
			stream << "MOV B,B";
			break;
		case 0x41:
			stream << "MOV B,C";
			break;
		case 0x42:
			stream << "MOV B,D";
			break;
		case 0x43:
			stream << "MOV B,E";
			break;
		case 0x44:
			stream << "MOV B,H";
			break;
		case 0x45:
			stream << "MOV B,L";
			break;
		case 0x46:
			stream << "MOV B,M";
			break;
		case 0x47:
			stream << "MOV B,A";
			break;
		case 0x48:
			stream << "MOV C,B";
			break;
		case 0x49:
			stream << "MOV C,C";
			break;
		case 0x4A:
			stream << "MOV C,D";
			break;
		case 0x4B:
			stream << "MOV C,E";
			break;
		case 0x4C:
			stream << "MOV C,H";
			break;
		case 0x4D:
			stream << "MOV C,L";
			break;
		case 0x4E:
			stream << "MOV C,M";
			break;
		case 0x4F:
			stream << "MOV C,A";
			break;
		case 0x50:
			stream << "MOV D,B";
			break;
		case 0x51:
			stream << "MOV D,C";
			break;
		case 0x52:
			stream << "MOV D,D";
			break;
		case 0x53:
			stream << "MOV D,E";
			break;
		case 0x54:
			stream << "MOV D,H";
			break;
		case 0x55:
			stream << "MOV D,L";
			break;
		case 0x56:
			stream << "MOV D,M";
			break;
		case 0x57:
			stream << "MOV D,A";
			break;
		case 0x58:
			stream << "MOV E,B";
			break;
		case 0x59:
			stream << "MOV E,C";
			break;
		case 0x5A:
			stream << "MOV E,D";
			break;
		case 0x5B:
			stream << "MOV E,E";
			break;
		case 0x5C:
			stream << "MOV E,H";
			break;
		case 0x5D:
			stream << "MOV E,L";
			break;
		case 0x5E:
			stream << "MOV E,M";
			break;
		case 0x5F:
			stream << "MOV E,A";
			break;
		case 0x60:
			stream << "MOV H,B";
			break;
		case 0x61:
			stream << "MOV H,C";
			break;
		case 0x62:
			stream << "MOV H,D";
			break;
		case 0x63:
			stream << "MOV H,E";
			break;
		case 0x64:
			stream << "MOV H,H";
			break;
		case 0x65:
			stream << "MOV H,L";
			break;
		case 0x66:
			stream << "MOV H,M";
			break;
		case 0x67:
			stream << "MOV H,A";
			break;
		case 0x68:
			stream << "MOV L,B";
			break;
		case 0x69:
			stream << "MOV L,C";
			break;
		case 0x6A:
			stream << "MOV L,D";
			break;
		case 0x6B:
			stream << "MOV L,E";
			break;
		case 0x6C:
			stream << "MOV L,H";
			break;
		case 0x6D:
			stream << "MOV L,L";
			break;
		case 0x6E:
			stream << "MOV L,M";
			break;
		case 0x6F:
			stream << "MOV L,A";
			break;
		case 0x70:
			stream << "MOV M,B";
			break;
		case 0x71:
			stream << "MOV M,C";
			break;
		case 0x72:
			stream << "MOV M,D";
			break;
		case 0x73:
			stream << "MOV M,E";
			break;
		case 0x74:
			stream << "MOV M,H";
			break;
		case 0x75:
			stream << "MOV M,L";
			break;
		case 0x76:
			stream << "HLT";
			break;
		case 0x77:
			stream << "MOV M,A";
			break;
		case 0x78:
			stream << "MOV A,B";
			break;
		case 0x79:
			stream << "MOV A,C";
			break;
		case 0x7A:
			stream << "MOV A,D";
			break;
		case 0x7B:
			stream << "MOV A,E";
			break;
		case 0x7C:
			stream << "MOV A,H";
			break;
		case 0x7D:
			stream << "MOV A,L";
			break;
		case 0x7E:
			stream << "MOV A,M";
			break;
		case 0x7F:
			stream << "MOV A,A";
			break;
		case 0x80:
			stream << "ADD B";
			break;
		case 0x81:
			stream << "ADD C";
			break;
		case 0x82:
			stream << "ADD D";
			break;
		case 0x83:
			stream << "ADD E";
			break;
		case 0x84:
			stream << "ADD H";
			break;
		case 0x85:
			stream << "ADD L";
			break;
		case 0x86:
			stream << "ADD M";
			break;
		case 0x87:
			stream << "ADD A";
			break;
		case 0x88:
			stream << "ADC B";
			break;
		case 0x89:
			stream << "ADC C";
			break;
		case 0x8A:
			stream << "ADC D";
			break;
		case 0x8B:
			stream << "ADC E";
			break;
		case 0x8C:
			stream << "ADC H";
			break;
		case 0x8D:
			stream << "ADC L";
			break;
		case 0x8E:
			stream << "ADC M";
			break;
		case 0x8F:
			stream << "ADC A";
			break;
		case 0x90:
			stream << "SUB B";
			break;
		case 0x91:
			stream << "SUB C";
			break;
		case 0x92:
			stream << "SUB D";
			break;
		case 0x93:
			stream << "SUB E";
			break;
		case 0x94:
			stream << "SUB H";
			break;
		case 0x95:
			stream << "SUB L";
			break;
		case 0x96:
			stream << "SUB M";
			break;
		case 0x97:
			stream << "SUB A";
			break;
		case 0x98:
			stream << "SBB B";
			break;
		case 0x99:
			stream << "SBB C";
			break;
		case 0x9A:
			stream << "SBB D";
			break;
		case 0x9B:
			stream << "SBB E";
			break;
		case 0x9C:
			stream << "SBB H";
			break;
		case 0x9D:
			stream << "SBB L";
			break;
		case 0x9E:
			stream << "SBB M";
			break;
		case 0x9F:
			stream << "SBB A";
			break;
		case 0xA0:
			stream << "ANA B";
			break;
		case 0xA1:
			stream << "ANA C";
			break;
		case 0xA2:
			stream << "ANA D";
			break;
		case 0xA3:
			stream << "ANA E";
			break;
		case 0xA4:
			stream << "ANA H";
			break;
		case 0xA5:
			stream << "ANA L";
			break;
		case 0xA6:
			stream << "ANA M";
			break;
		case 0xA7:
			stream << "ANA A";
			break;
		case 0xA8:
			stream << "XRA B";
			break;
		case 0xA9:
			stream << "XRA C";
			break;
		case 0xAA:
			stream << "XRA D";
			break;
		case 0xAB:
			stream << "XRA E";
			break;
		case 0xAC:
			stream << "XRA H";
			break;
		case 0xAD:
			stream << "XRA L";
			break;
		case 0xAE:
			stream << "XRA M";
			break;
		case 0xAF:
			stream << "XRA A";
			break;
		case 0xB0:
			stream << "ORA B";
			break;
		case 0xB1:
			stream << "ORA C";
			break;
		case 0xB2:
			stream << "ORA D";
			break;
		case 0xB3:
			stream << "ORA E";
			break;
		case 0xB4:
			stream << "ORA H";
			break;
		case 0xB5:
			stream << "ORA L";
			break;
		case 0xB6:
			stream << "ORA M";
			break;
		case 0xB7:
			stream << "ORA A";
			break;
		case 0xB8:
			stream << "CMP B";
			break;
		case 0xB9:
			stream << "CMP C";
			break;
		case 0xBA:
			stream << "CMP D";
			break;
		case 0xBB:
			stream << "CMP E";
			break;
		case 0xBC:
			stream << "CMP H";
			break;
		case 0xBD:
			stream << "CMP L";
			break;
		case 0xBE:
			stream << "CMP M";
			break;
		case 0xBF:
			stream << "CMP A";
			break;
		case 0xC0:
			stream << "RNZ";
			break;
		case 0xC1:
			stream << "POP BC";
			break;
		case 0xC2:
			stream << std::format("JNZ {:04X}", readArgument16());
			break;
		case 0xC3:
			stream << std::format("JMP {:04X}", readArgument16());
			break;
		case 0xC4:
			stream << std::format("CNZ {:04X}", readArgument16());
			break;
		case 0xC5:
			stream << "PUSH BC";
			break;
		case 0xC6:
			stream << std::format("ADI {:02X}", readArgument8());
			break;
		case 0xC7:
			stream << "RST 0";
			break;
		case 0xC8:
			stream << "RZ";
			break;
		case 0xC9:
			stream << "RET";
			break;
		case 0xCA:
			stream << std::format("JZ {:04X}", readArgument16());
			break;
		case 0xCC:
			stream << std::format("CZ {:04X}", readArgument16());
			break;
		case 0xCD:
			stream << std::format("CALL {:04X}", readArgument16());
			break;
		case 0xCE:
			stream << std::format("ACI {:02X}", readArgument8());
			break;
		case 0xCF:
			stream << "RST 1";
			break;
		case 0xD0:
			stream << "RNC";
			break;
		case 0xD1:
			stream << "POP DE";
			break;
		case 0xD2:
			stream << std::format("JNC {:04X}", readArgument16());
			break;
		case 0xD3:
			stream << std::format("OUT {:02X}", readArgument8());
			break;
		case 0xD4:
			stream << std::format("CNC {:04X}", readArgument16());
			break;
		case 0xD5:
			stream << "PUSH DE";
			break;
		case 0xD6:
			stream << std::format("SUI {:02X}", readArgument8());
			break;
		case 0xD7:
			stream << "RST 2";
			break;
		case 0xD8:
			stream << "RC";
			break;
		case 0xDA:
			stream << std::format("JC {:04X}", readArgument16());
			break;
		case 0xDB:
			stream << std::format("IN {:02X}", readArgument8());
			break;
		case 0xDC:
			stream << std::format("CC {:04X}", readArgument16());
			break;
		case 0xDE:
			stream << std::format("SBI {:02X}", readArgument8());
			break;
		case 0xDF:
			stream << "RST 3";
			break;
		case 0xE0:
			stream << "RPO";
			break;
		case 0xE1:
			stream << "POP HL";
			break;
		case 0xE2:
			stream << std::format("JPO {:04X}", readArgument16());
			break;
		case 0xE3:
			stream << "XTHL";
			break;
		case 0xE4:
			stream << std::format("CPO {:04X}", readArgument16());
			break;
		case 0xE5:
			stream << "PUSH HL";
			break;
		case 0xE6:
			stream << std::format("ANI {:02X}", readArgument8());
			break;
		case 0xE7:
			stream << "RST 4";
			break;
		case 0xE8:
			stream << "RPE";
			break;
		case 0xE9:
			stream << "PCHL";
			break;
		case 0xEA:
			stream << std::format("JPE {:04X}", readArgument16());
			break;
		case 0xEB:
			stream << "XCHG";
			break;
		case 0xEC:
			stream << std::format("CPE {:04X}", readArgument16());
			break;
		case 0xEE:
			stream << std::format("XRI {:02X}", readArgument8());
			break;
		case 0xEF:
			stream << "RST 5";
			break;
		case 0xF0:
			stream << "RP";
			break;
		case 0xF1:
			stream << "POP PSW";
			break;
		case 0xF2:
			stream << std::format("JP {:04X}", readArgument16());
			break;
		case 0xF3:
			stream << "DI";
			break;
		case 0xF4:
			stream << std::format("CP {:04X}", readArgument16());
			break;
		case 0xF5:
			stream << "PUSH PSW";
			break;
		case 0xF6:
			stream << std::format("ORI {:02X}", readArgument8());
			break;
		case 0xF7:
			stream << "RST 6";
			break;
		case 0xF8:
			stream << "RM";
			break;
		case 0xF9:
			stream << "SPHL";
			break;
		case 0xFA:
			stream << std::format("JM {:04X}", readArgument16());
			break;
		case 0xFB:
			stream << "EI";
			break;
		case 0xFC:
			stream << std::format("CM {:04X}", readArgument16());
			break;
		case 0xFE:
			stream << std::format("CPI {:02X}", readArgument8());
			break;
		case 0xFF:
			stream << "RST 7";
			break;
		default:
			stream << "illegal opcode " << std::hex << int(opcode);
			break;
		}
		stream << std::endl;
		return stream.str();
	}

	void Intel8080::unimplemented()
	{
		pc--;
		auto opcode = readOpcode();
		std::cout << std::hex << int(opcode) << std::endl;
		pc--;
		std::cout << disassemble();
		throw std::runtime_error("Instruction not implemented");
	}
	void Intel8080::illegal()
	{
		pc--;
		auto opcode = readOpcode();
		throw std::runtime_error("Illegal instruction " + opcode);
	}

	void Intel8080::pushToStack(const uint16_t value)
	{
		_state.sp() -= 2;
		write(_state.sp(), value);
	}
	const uint16_t Intel8080::popOfStack()
	{
		uint16_t value = _handlerRead(_state.sp()) | (_handlerRead(_state.sp() + 1) << 8);
		_state.sp() += 2;
		return value;
	}
	const uint8_t Intel8080::executeOne() {
		if (interrupt_enabled == 0 && interrupt_request < 8) {
			interrupt_enabled = 2;
			pushToStack(pc);
			pc = interrupt_request << 3;
			interrupt_request = 8;
		}
		else if (interrupt_enabled == 1)
			interrupt_enabled = 0;

		const std::uint8_t opcode = readOpcode();
		decode_opcode(opcode);
		return 4;
	}

	bool Intel8080::interrupt(const uint8_t inte) {
		if (interrupt_enabled == 0)
			interrupt_request = inte;
		return (interrupt_enabled == 0) ? true : false;
	}

	bool Intel8080::reset(const uint16_t address) {
		pc = address;
		_state.reset();
		interrupt_enabled = 2;
		interrupt_request = 8;
		return true;
	}

	uint8_t Intel8080::decode8(const opcode_t opcode) const
	{
		switch (opcode & 0x07) {
		case 0x00:
			return _state.b();
		case 0x01:
			return _state.c();
		case 0x02:
			return _state.d();
		case 0x03:
			return _state.e();
		case 0x04:
			return _state.h();
		case 0x05:
			return _state.l();
		case 0x07:
			return _state.a();
		}
		throw std::runtime_error("Unexpected opcode in decode8 " + opcode);
	}
	uint8_t& Intel8080::decode8(const opcode_t opcode)
	{
		switch (opcode & 0x07) {
		case 0x00:
			return _state.b();
		case 0x01:
			return _state.c();
		case 0x02:
			return _state.d();
		case 0x03:
			return _state.e();
		case 0x04:
			return _state.h();
		case 0x05:
			return _state.l();
		case 0x07:
			return _state.a();
		}
		throw std::runtime_error("Unexpected opcode in decode8 " + opcode);
	}
}

