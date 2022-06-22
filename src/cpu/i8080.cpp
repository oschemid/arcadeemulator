#include "i8080.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <format>

using ae::memory;

namespace ae::cpu {
	// Helpers
	const uint8_t parity(const uint8_t value) {
		uint8_t v = value;
		v ^= v >> 4;
		v ^= v >> 2;
		return ~((v ^= v >> 1) & 1);
	}
	const uint8_t zero(const uint8_t value) {
		return value == 0;
	}
	const uint8_t sign(const uint8_t value) {
		return (value & 0x80) != 0;
	}

	i8080::i8080(ae::memory* m) :
		memory(m),
		a(0),
		b(0),
		c(0),
		d(0),
		e(0),
		h(0),
		l(0),
		interrupt_enabled(2),
		interrupt_request(8),
		shift0(0),
		shift1(0)
	{
	}
	uint16_t i8080::get_bc() const {
		return (b << 8) | c;
	}
	void i8080::set_bc(const uint16_t v) {
		b = v >> 8;
		c = v & 0xff;
	}
	uint16_t i8080::get_de() const {
		return (d << 8) | e;
	}
	void i8080::set_de(const uint16_t v) {
		d = v >> 8;
		e = v & 0xff;
	}
	uint16_t i8080::get_hl() const {
		return (h << 8) | l;
	}
	void i8080::set_hl(const uint16_t v) {
		h = v >> 8;
		l = v & 0xff;
	}
	uint8_t i8080::get_m() const {
		return memory->read(get_hl());
	}

	uint8_t i8080::dcr(const uint8_t value) {
		uint8_t result = value - 1;

		zeroBit = zero(result);
		signBit = sign(result);
		parityBit = parity(result);
		auxCarryBit = (result & 0x0f) == 0x0f;
		return result;
	}
	uint8_t i8080::inr(const uint8_t value) {
		uint8_t result = value + 1;
		zeroBit = zero(result);
		signBit = sign(result);
		parityBit = parity(result);
		auxCarryBit = (result & 0x0f) == 0x0f;
		return result;
	}

	void i8080::xra(const uint8_t value) {
		a ^= value;
		carryBit = 0;
		auxCarryBit = 0;
		parityBit = parity(a);
		signBit = sign(a);
		zeroBit = zero(a);
	}
	void i8080::ora(const uint8_t value) {
		a |= value;
		carryBit = 0;
		auxCarryBit = 0;
		parityBit = parity(a);
		signBit = sign(a);
		zeroBit = zero(a);
	}
	void i8080::ana(const uint8_t value) {
		uint8_t result = a & value;
		carryBit = 0;
		parityBit = parity(result);
		signBit = sign(result);
		zeroBit = zero(result);
		auxCarryBit = ((a | value) & 0x08) == 0;
		a = result;
	}
	void i8080::sub(const uint8_t value) {
		carryBit = (value > a) ? 1 : 0;
		auxCarryBit = (value & 0xf) > (a & 0xf) ? 1 : 0;
		a -= value;
		signBit = sign(a);
		zeroBit = zero(a);
		parityBit = parity(a);
	}
	void i8080::sbb(const uint8_t value) {
		sub(value + carryBit);
	}
	void i8080::cmp(const uint8_t value) {
		carryBit = (value > a) ? 1 : 0;
		auxCarryBit = (value & 0xf) > (a & 0xf) ? 1 : 0;
		uint8_t r = a - value;
		signBit = sign(r);
		zeroBit = zero(r);
		parityBit = parity(r);
	}
	void i8080::add(const uint8_t value)
	{
		uint16_t sum = a + value;
		auxCarryBit = ((value & 0x0f) + (a & 0x0f)) > 0x0f;
		a = sum & 0xff;
		zeroBit = zero(a);
		signBit = sign(a);
		parityBit = parity(a);
		carryBit = (sum > 0xff);
	}
	void i8080::adc(const uint8_t value)
	{
		add(value + carryBit);
	}
	void i8080::daa()
	{
		uint16_t temp = a;
		if (((a & 0xf) > 9) || (auxCarryBit)) {
			temp += 6;
			auxCarryBit = ((a & 0xf) > 9) ? 1 : 0;
		}
		if ((temp >> 4 > 9) || (carryBit)) {
			temp += 0x60;
			carryBit = temp > 0xff;
		}
		a = temp & 0xff;
	}
	void i8080::dad(const uint16_t value)
	{
		uint32_t res = get_hl() + value;
		h = res >> 8;
		l = res & 0xff;
		carryBit = (res & 0xffff0000) != 0;
	}

	const string i8080::disassemble() {
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

	void i8080::unimplemented()
	{
		pc--;
		auto opcode = readOpcode();
		std::cout << std::hex << int(opcode) << std::endl;
		pc--;
		std::cout << disassemble();
		throw std::runtime_error("Instruction not implemented");
	}
	void i8080::illegal()
	{
		pc--;
		auto opcode = readOpcode();
		throw std::runtime_error("Illegal instruction " + opcode);
	}

	void i8080::pushToStack(const uint16_t value)
	{
		sp -= 2;
		write(sp, value);
	}
	const uint16_t i8080::popOfStack()
	{
		uint16_t value = memory->read(sp) | (memory->read(sp + 1) << 8);
		sp += 2;
		return value;
	}
	const uint8_t i8080::executeOne()
	{
		if (interrupt_enabled == 0 && interrupt_request < 8) {
			interrupt_enabled = 2;
			pushToStack(pc);
			pc = interrupt_request << 3;
			interrupt_request = 8;
		}
		else if (interrupt_enabled == 1)
			interrupt_enabled = 0;

		uint16_t cycle = 0;
		const std::uint8_t opcode = readOpcode();
		uint16_t tmp16 = 0;

		switch (opcode) {
		case 0x00: /* NOP */
			cycle = 4;
			break;
		case 0x01: /* LXI B */
			c = readArgument8();
			b = readArgument8();
			cycle = 10;
			break;
//		case 0x02: /* STAX B */
		case 0x03: /* INX B */
			c++;
			if (c == 0)
				b++;
			cycle = 5;
			break;
		case 0x04: /* INR B */
			b = inr(b);
			cycle = 5;
			break;
		case 0x05: /* DCR B */
			b = dcr(b);
			cycle = 5;
			break;
		case 0x06: /* MVI B */
			b = readArgument8();
			cycle = 7;
			break;
		case 0x07: /* RLC */
			carryBit = (a >> 7);
			a = carryBit | (a << 1);
			cycle = 4;
			break;
		case 0x09: /* DAD BC */
			dad(get_bc());
			cycle = 10;
			break;
		case 0x0A: /* LDAX B */
			a = memory->read(get_bc());
			cycle = 7;
			break;
		case 0x0B: /* DCX B */
			set_bc(get_bc() - 1);
			cycle = 5;
			break;
		case 0x0C: /* INR C */
			c = inr(c);
			cycle = 5;
			break;
		case 0x0D: /* DCR C */
			c = dcr(c);
			cycle = 5;
			break;
		case 0x0E: /* MVI C */
			c = readArgument8();
			cycle = 7;
			break;
		case 0x0F: /* RRC */
			carryBit = (a & 1);
			a = (carryBit << 7) | (a >> 1);
			cycle = 4;
			break;

		case 0x11: /* LXI D */
			e = readArgument8();
			d = readArgument8();
			cycle = 10;
			break;
		case 0x12: /* STAX D */
			memory->write(get_de(), a);
			cycle = 7;
			break;
		case 0x13: /* INX D */
			e++;
			if (e == 0)
				d++;
			cycle = 5;
			break;
		case 0x14: /* INR D */
			d = inr(d);
			cycle = 5;
			break;
		case 0x15: /* DCR D */
			d = dcr(d);
			cycle = 5;
			break;
		case 0x16: /* MVI D */
			d = readArgument8();
			cycle = 7;
			break;
//		case 0x17: /* RAL */
		case 0x19: /* DAD D */
			dad(get_de());
			cycle = 10;
			break;
		case 0x1A: /* LDAX D */
			a = memory->read(get_de());
			cycle = 7;
			break;
		case 0x1B: /* DCX D */
			set_de(get_de() - 1);
			cycle = 5;
			break;
		case 0x1C: /* INR E */
			e = inr(e);
			cycle = 5;
			break;
		case 0x1D: /* DCR E */
			e = dcr(e);
			cycle = 5;
			break;
		case 0x1E: /* MVI E */
			e = readArgument8();
			cycle = 7;
			break;
		case 0x1F: /* RAR */
			{
				uint8_t flag = carryBit;
				carryBit = a & 0x01;
				a = (a >> 1) | (flag << 7);
			}
			cycle = 4;
			break;
		case 0x21: /* LXI H */
			l = readArgument8();
			h = readArgument8();
			cycle = 10;
			break;
		case 0x22: /* SHLD */
			tmp16 = readArgument16();
			memory->write(tmp16, l);
			memory->write(tmp16 + 1, h);
			cycle = 16;
			break;
		case 0x23: /* INX H */
			l++;
			if (l == 0)
				h++;
			cycle = 5;
			break;
		case 0x24: /* INR H */
			h = inr(h);
			cycle = 5;
			break;
		case 0x25: /* DCR H */
			h = dcr(h);
			cycle = 5;
			break;
		case 0x26: /* MVI H */
			h = readArgument8();
			cycle = 7;
			break;
		case 0x27: /* DAA */
			daa();
			cycle = 4;
			break;
		case 0x29: /* DAD H */
			dad(get_hl());
			cycle = 10;
			break;
		case 0x2A: /* LHLD */
			tmp16 = readArgument16();
			l = memory->read(tmp16);
			h = memory->read(tmp16 + 1);
			cycle = 16;
			break;
		case 0x2B: /* DCX H */
			tmp16 = get_hl() - 1;
			h = tmp16 >> 8;
			l = tmp16 & 0xff;
			cycle = 5;
			break;
		case 0x2C: /* INR L */
			l = inr(l);
			cycle = 5;
			break;
		case 0x2D: /* DCR L */
			l = dcr(l);
			cycle = 5;
			break;
		case 0x2E: /* MVI L */
			l = readArgument8();
			cycle = 7;
			break;
		case 0x2F: /* CMA */
			a = ~a;
			cycle = 4;
			break;

		case 0x31: /* LXI SP */
			sp = readArgument16();
			cycle = 10;
			break;
		case 0x32: /* STA */
			memory->write(readArgument16(), a);
			cycle = 13;
			break;
//		case 0x33: /* INX SP */
		case 0x34: /* INR M */
			memory->write(get_hl(), inr(get_m()));
			cycle = 10;
			break;
		case 0x35: /* DCR M */
			memory->write(get_hl(), dcr(get_m()));
			cycle = 10;
			break;
		case 0x36: /* MVI M */
			memory->write(get_hl(), readArgument8());
			cycle = 10;
			break;
		case 0x37: /* STC */
			carryBit = 1;
			cycle = 4;
			break;
//		case 0x39: /* DAD SP */
		case 0x3A: /* LDA */
			a = memory->read(readArgument16());
			cycle = 13;
			break;
//		case 0x3B: /* DCX SP */
		case 0x3C: /* INR A */
			a = inr(a);
			cycle = 5;
			break;
		case 0x3D: /* DCR A */
			a = dcr(a);
			cycle = 5;
			break;
		case 0x3E: /* MVI A */
			a = readArgument8();
			cycle = 7;
			break;
//		case 0x3F: /* CMC */

		case 0x40: /* MOV B,B */
			b = b;
			cycle = 5;
			break;
		case 0x41: /* MOV B,C */
			b = c;
			cycle = 5;
			break;
		case 0x42: /* MOV B,D */
			b = d;
			cycle = 5;
			break;
		case 0x43: /* MOV B,E */
			b = e;
			cycle = 5;
			break;
		case 0x44: /* MOV B,H */
			b = h;
			cycle = 5;
			break;
		case 0x45: /* MOV B,L */
			b = l;
			cycle = 5;
			break;
		case 0x46: /* MOV B,M */
			b = get_m();
			cycle = 7;
			break;
		case 0x47: /* MOV B,A */
			b = a;
			cycle = 5;
			break;
		case 0x48: /* MOV C,B */
			c = b;
			cycle = 5;
			break;
		case 0x49: /* MOV C,C */
			c = c;
			cycle = 5;
			break;
		case 0x4A: /* MOV C,D */
			c = d;
			cycle = 5;
			break;
		case 0x4B: /* MOV C,E */
			c = e;
			cycle = 5;
			break;
		case 0x4C: /* MOV C,H */
			c = h;
			cycle = 5;
			break;
		case 0x4D: /* MOV C,L */
			c = l;
			cycle = 5;
			break;
		case 0x4E: /* MOV C,M */
			c = get_m();
			cycle = 7;
			break;
		case 0x4F: /* MOV C,A */
			c = a;
			cycle = 5;
			break;

		case 0x50: /* MOV D,B */
			d = b;
			cycle = 5;
			break;
		case 0x51: /* MOV D,C */
			d = c;
			cycle = 5;
			break;
		case 0x52: /* MOV D,D */
			d = d;
			cycle = 5;
			break;
		case 0x53: /* MOV D,E */
			d = e;
			cycle = 5;
			break;
		case 0x54: /* MOV D,H */
			d = h;
			cycle = 5;
			break;
		case 0x55: /* MOV D,L */
			d = l;
			cycle = 5;
			break;
		case 0x56: /* MOV D,M */
			d = get_m();
			cycle = 7;
			break;
		case 0x57: /* MOV D,A */
			d = a;
			cycle = 5;
			break;
		case 0x58: /* MOV E,B */
			e = b;
			cycle = 5;
			break;
		case 0x59: /* MOV E,C */
			e = c;
			cycle = 5;
			break;
		case 0x5A: /* MOV E,D */
			e = d;
			cycle = 5;
			break;
		case 0x5B: /* MOV E,E */
			e = e;
			cycle = 5;
			break;
		case 0x5C: /* MOV E,H */
			e = h;
			cycle = 5;
			break;
		case 0x5D: /* MOV E,L */
			e = l;
			cycle = 5;
			break;
		case 0x5E: /* MOV E,M */
			e = get_m();
			cycle = 7;
			break;
		case 0x5F: /* MOV E,A */
			e = a;
			cycle = 5;
			break;

		case 0x60: /* MOV H,B */
			h = b;
			cycle = 5;
			break;
		case 0x61: /* MOV H,C */
			h = c;
			cycle = 5;
			break;
		case 0x62: /* MOV H,D */
			h = d;
			cycle = 5;
			break;
		case 0x63: /* MOV H,E */
			h = e;
			cycle = 5;
			break;
		case 0x64: /* MOV H,H */
			h = h;
			cycle = 5;
			break;
		case 0x65: /* MOV H,L */
			h = l;
			cycle = 5;
			break;
		case 0x66: /* MOV H,M */
			h = get_m();
			cycle = 7;
			break;
		case 0x67: /* MOV H,A */
			h = a;
			cycle = 5;
			break;
		case 0x68: /* MOV L,B */
			l = b;
			cycle = 5;
			break;
		case 0x69: /* MOV L,C */
			l = c;
			cycle = 5;
			break;
		case 0x6A: /* MOV L,D */
			l = d;
			cycle = 5;
			break;
		case 0x6B: /* MOV L,E */
			l = e;
			cycle = 5;
			break;
		case 0x6C: /* MOV L,H */
			l = h;
			cycle = 5;
			break;
		case 0x6D: /* MOV L,L */
			l = l;
			cycle = 5;
			break;
		case 0x6E: /* MOV L,M */
			l = get_m();
			cycle = 7;
			break;
		case 0x6F: /* MOV L,A */
			l = a;
			cycle = 5;
			break;

		case 0x70: /* MOV M,B */
			memory->write(get_hl(), b);
			cycle = 7;
			break;
		case 0x71: /* MOV M,C */
			memory->write(get_hl(), c);
			cycle = 7;
			break;
		case 0x72: /* MOV M,D */
			memory->write(get_hl(), d);
			cycle = 7;
			break;
		case 0x73: /* MOV M,E */
			memory->write(get_hl(), e);
			cycle = 7;
			break;
//		case 0x74: /* MOV M,H */
//		case 0x75: /* MOV M,L */
//		case 0x76: /* HLT */
		case 0x77: /* MOV M,A */
			memory->write(get_hl(), a);
			cycle = 7;
			break;
		case 0x78: /* MOV A,B */
			a = b;
			cycle = 5;
			break;
		case 0x79: /* MOV A,C */
			a = c;
			cycle = 5;
			break;
		case 0x7A: /* MOV A,D */
			a = d;
			cycle = 5;
			break;
		case 0x7B: /* MOV A,E */
			a = e;
			cycle = 5;
			break;
		case 0x7C: /* MOV A,H */
			a = h;
			cycle = 5;
			break;
		case 0x7D: /* MOV A,L */
			a = l;
			cycle = 5;
			break;
		case 0x7E: /* MOV A,M */
			a = get_m();
			cycle = 7;
			break;
		case 0x7F: /* MOV A,A */
			a = a;
			cycle = 5;
			break;

		case 0x80: /* ADD B */
			add(b);
			cycle = 4;
			break;
		case 0x81: /* ADD C */
			add(c);
			cycle = 4;
			break;
		case 0x82: /* ADD D */
			add(d);
			cycle = 4;
			break;
		case 0x83: /* ADD E */
			add(e);
			cycle = 4;
			break;
		case 0x84: /* ADD H */
			add(h);
			cycle = 4;
			break;
		case 0x85: /* ADD L */
			add(l);
			cycle = 4;
			break;
		case 0x86: /* ADD M */
			add(get_m());
			cycle = 7;
			break;
		case 0x87: /* ADD A */
			add(a);
			cycle = 4;
			break;
		case 0x88: /* ADC B */
			adc(b);
			cycle = 4;
			break;
		case 0x89: /* ADC C */
			adc(c);
			cycle = 4;
			break;
		case 0x8A: /* ADC D */
			adc(d);
			cycle = 4;
			break;
		case 0x8B: /* ADC E */
			adc(e);
			cycle = 4;
			break;
		case 0x8C: /* ADC H */
			adc(h);
			cycle = 4;
			break;
		case 0x8D: /* ADC L */
			adc(l);
			break;
//		case 0x8E: /* ADC M */
		case 0x8F: /* ADC A */
			adc(a);
			cycle = 4;
			break;

		case 0x90: /* SUB B */
			sub(b);
			cycle = 4;
			break;
		case 0x91: /* SUB C */
			sub(c);
			cycle = 4;
			break;
		case 0x92: /* SUB D */
			sub(d);
			cycle = 4;
			break;
		case 0x93: /* SUB E */
			sub(e);
			cycle = 4;
			break;
		case 0x94: /* SUB H */
			sub(h);
			cycle = 4;
			break;
		case 0x95: /* SUB L */
			sub(l);
			cycle = 4;
			break;
//		case 0x96: /* SUB M */
		case 0x97: /* SUB A */
			sub(a);
			cycle = 4;
			break;
//		case 0x98: /* SBB B */
//		case 0x99: /* SBB C */
//		case 0x9A: /* SBB D */
//		case 0x9B: /* SBB E */
//		case 0x9C: /* SBB H */
//		case 0x9D: /* SBB L */
//		case 0x9E: /* SBB M */
//		case 0x9F: /* SBB A */

		case 0xA0: /* ANA B */
			ana(b);
			cycle = 4;
			break;
		case 0xA1: /* ANA C */
			ana(c);
			cycle = 4;
			break;
		case 0xA2: /* ANA D */
			ana(d);
			cycle = 4;
			break;
		case 0xA3: /* ANA E */
			ana(e);
			cycle = 4;
			break;
		case 0xA4: /* ANA H */
			ana(h);
			cycle = 4;
			break;
		case 0xA5: /* ANA L */
			ana(l);
			cycle = 4;
			break;
		case 0xA6: /* ANA M */
			ana(get_m());
			cycle = 7;
			break;
		case 0xA7: /* ANA A */
			ana(a);
			cycle = 4;
			break;
		case 0xA8: /* XRA B */
			xra(b);
			cycle = 4;
			break;
		case 0xA9: /* XRA C */
			xra(c);
			cycle = 4;
			break;
		case 0xAA: /* XRA D */
			xra(d);
			cycle = 4;
			break;
		case 0xAB: /* XRA E */
			xra(e);
			cycle = 4;
			break;
		case 0xAC: /* XRA H */
			xra(h);
			cycle = 4;
			break;
		case 0xAD: /* XRA L */
			xra(l);
			cycle = 4;
			break;
		case 0xAE: /* XRA M */
			xra(get_m());
			cycle = 7;
			break;
		case 0xAF: /* XRA A */
			xra(a);
			cycle = 4;
			break;

		case 0xB0: /* ORA B */
			ora(b);
			cycle = 4;
			break;
		case 0xB1: /* ORA C */
			ora(c);
			cycle = 4;
			break;
		case 0xB2: /* ORA D */
			ora(d);
			cycle = 4;
			break;
		case 0xB3: /* ORA E */
			ora(e);
			cycle = 4;
			break;
		case 0xB4: /* ORA H */
			ora(h);
			cycle = 4;
			break;
		case 0xB5: /* ORA L */
			ora(l);
			cycle = 4;
			break;
		case 0xB6: /* ORA M */
			ora(get_m());
			cycle = 7;
			break;
		case 0xB7: /* ORA A */
			ora(a);
			cycle = 4;
			break;
		case 0xB8: /* CMP B */
			cmp(b);
			cycle = 4;
			break;
		case 0xB9: /* CMP C */
			cmp(c);
			cycle = 4;
			break;
		case 0xBA: /* CMP D */
			cmp(d);
			cycle = 4;
			break;
		case 0xBB: /* CMP E */
			cmp(e);
			cycle = 4;
			break;
		case 0xBC: /* CMP H */
			cmp(h);
			cycle = 4;
			break;
		case 0xBD: /* CMP L */
			cmp(l);
			cycle = 4;
			break;
		case 0xBE: /* CMP M */
			cmp(get_m());
			cycle = 7;
			break;
		case 0xBF: /* CMP A */
			cmp(a);
			cycle = 4;
			break;

		case 0xC0: /* RNZ */
			if (!zeroBit) {
				pc = popOfStack();
				cycle = 11;
			}
			else {
				cycle = 5;
			}
			break;
		case 0xC1: /* POP BC */ { uint16_t t = popOfStack(); b = t >> 8; c = t & 0xff; } cycle = 10; break;
		case 0xC2: /* JNZ */
			tmp16 = readArgument16();
			if (!zeroBit)
				pc = tmp16;
			cycle = 10;
			break;
		case 0xC3: /* JMP */
			pc = readArgument16();
			cycle = 10;
			break;
		case 0xC4: /* CNZ */
			tmp16 = readArgument16();
			if (!zeroBit) {
				pushToStack(pc);
				pc = tmp16;
				cycle = 17;
			}
			else {
				cycle = 11;
			}
			break;
		case 0xC5: /* PUSH BC */ pushToStack((b << 8) | c); cycle = 11; break;
		case 0xC6: /* ADI */
			add(readArgument8());
			cycle = 7;
			break;
//		case 0xC7: /* RST 0 */
		case 0xC8: /* RZ */
			if (zeroBit) {
				pc = popOfStack();
				cycle = 11;
			}
			else {
				cycle = 5;
			}
			break;
		case 0xC9: /* RET */ pc = popOfStack(); cycle = 10; break;
		case 0xCA: /* JZ */
			tmp16 = readArgument16();
			if (zeroBit)
				pc = tmp16;
			cycle = 10;
			break;
		case 0xCC: /* CZ */
			tmp16 = readArgument16();
			if (zeroBit) {
				pushToStack(pc);
				pc = tmp16;
				cycle = 17;
			}
			else {
				cycle = 11;
			}
			break;
		case 0xCD: /* CALL */ tmp16 = readArgument16(); pushToStack(pc); pc = tmp16; cycle = 17; break;
//		case 0xCE: /* ACI */
//		case 0xCF: /* RST 1 */

		case 0xD0: /* RNC */
			if (!carryBit) {
				pc = popOfStack();
				cycle = 11;
			}
			else {
				cycle = 5;
			}
			break;
		case 0xD1: /* POP DE */
			set_de(popOfStack());
			cycle = 10;
			break;
		case 0XD2: /* JNC */
			tmp16 = readArgument16();
			if (!carryBit)
				pc = tmp16;
			cycle = 10;
			break;
		case 0xD3: /* OUT */
			out(readArgument8(), a);
			cycle = 10;
			break; // TEMPORAIRE
		case 0xD4: /* CNC */
			tmp16 = readArgument16();
			if (!carryBit) {
				pushToStack(pc);
				pc = tmp16;
				cycle = 17;
			}
			else {
				cycle = 11;
			}
			break;
		case 0xD5: /* PUSH DE */
			pushToStack(get_de());
			cycle = 11;
			break;
		case 0XD6: /* SUI */
			sub(readArgument8());
			cycle = 7;
			break;
//		case 0xD7: /* RST 2 */
		case 0XD8: /* RC */
			if (carryBit) {
				pc = popOfStack();
				cycle = 11;
			}
			else {
				cycle = 5;
			}
			break;
		case 0xDA: /* JC */
			tmp16 = readArgument16();
			if (carryBit)
				pc = tmp16;
			cycle = 10;
			break;
		case 0XDB: /* IN */
			a = in(readArgument8());
			cycle = 10;
			break;
//		case 0xDC: /* CC */
		case 0xDE: /* SBI */
			sbb(readArgument8());
			cycle = 7;
			break;
//		case 0xDF: /* RST 3 */

//		case 0xE0: /* RPO */
		case 0xE1: /* POP HL */
			set_hl(popOfStack());
			cycle = 10;
			break;
//		case 0xE2: /* JPO */
		case 0xE3: /* XTHL */
		   tmp16 = memory->read(sp) | (memory->read(sp+1)<<8);
		   memory->write(sp, l);
		   memory->write(sp + 1, h);
		   set_hl(tmp16);
		   cycle = 18;
		   break;
//		case 0xE4: /* CPO */
		case 0xE5: /* PUSH HL */ pushToStack((h << 8) | l); cycle = 11; break;
		case 0xE6: /* ANI */
			ana(readArgument8());
			cycle = 7;
			break;
//		case 0XE7: /* RST 4 */
//		case 0xE8: /* RPE */
		case 0xE9: /* PCHL */
			pc = get_hl();
			cycle = 5;
			break;
//		case 0xEA: /* JPE */
		case 0xEB: /* XCHG */ { uint8_t tmp = d; d = h; h = tmp; tmp = e; e = l; l = tmp; } cycle = 4; break;
//		case 0xEC: /* CPE */
//		case 0xEE: /* XRI */
//		case 0xEF: /* RST 5 */

//		case 0xF0: /* RP */
		case 0xF1: /* POP PSW */ { uint16_t t = popOfStack(); a = t >> 8; parityBit = (t >> 7) & 0x1; zeroBit = (t >> 6) & 1; signBit = (t >> 5) & 1; carryBit = (t >> 4) & 1; auxCarryBit = (t >> 3) & 1; } cycle = 10; break;
//		case 0xF2: /* JP */
//		case 0xF3: /* DI */
//		case 0xF4: /* CP */
		case 0xF5: /* PUSH PSW */ pushToStack((a << 8) | (parityBit << 7) | (zeroBit << 6) | (signBit << 5) | (carryBit << 4) | (auxCarryBit << 3)); cycle = 11; break;
		case 0xF6: /* ORI */
			ora(readArgument8());
			cycle = 7;
			break;
//		case 0xF7: /* RST 6 */
//		case 0xF8: /* RM */
//		case 0xF9: /* SPHL */
		case 0xFA: /* JM */
			tmp16 = readArgument16();
			if (signBit)
				pc = tmp16;
			cycle = 10;
			break;
		case 0xFB: /* EI */
			interrupt_enabled = 1;
			cycle = 4;
			break;
		case 0xFC: /* CM */
			tmp16 = readArgument16();
			if (signBit) {
				pushToStack(pc);
				pc = tmp16;
				cycle = 17;
			}
			else {
				cycle = 11;
			}
			break;
		case 0xFE: /* CPI */
			cmp(readArgument8());
			cycle = 7;
			break;
//		case 0xFF: /* RST 7 */

		default: unimplemented(); break;
		}
		return cycle;
	}
	
	const uint8_t i8080::in(const uint8_t port)
	{
		uint8_t a=0;
		switch (port)
		{
		case 1:
			a = inPort[1];
			break;
		case 2:
			a = inPort[2];
			break;
		case 3:
		{
			uint16_t v = (shift1 << 8) | shift0;
			a = ((v >> (8 - shift_offset)) & 0xff);
		}
		break;
		}
		return a;
	}

	void i8080::out(const uint8_t port, const uint8_t value)
	{
		switch (port)
		{
		case 2:
			shift_offset = value & 0x7;
			break;
		case 4:
			shift0 = shift1;
			shift1 = value;
			break;
		}
	}

	bool i8080::interrupt(const uint8_t inte) {
		if (interrupt_enabled == 0)
			interrupt_request = inte;
		return (interrupt_enabled==0)?true:false;
	}
}

