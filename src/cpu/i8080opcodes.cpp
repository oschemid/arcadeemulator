#include "i8080.h"
#include "types.h"
#include <array>

enum class opcode {
	UNIMPLEMENTED,
	HALT,
	LD_R_HL,
	LD_R_R,
	LD_HL_R,
};

/*********************************************************************************************************************/
// OPCODES TABLES
constexpr auto opcodes{ []() constexpr {
	std::array<opcode,256> result{opcode::UNIMPLEMENTED};
	for (int i = 0; i < 256; ++i) {
		if ((i & 0b11000000) == 0b01000000) {
			if ((i & 0b11000111) == 0b01000110)
				//result[i] = (i == 0b01110110) ? opcode::HALT : opcode::LD_R_HL;
				result[i] = opcode::UNIMPLEMENTED;
			else
				result[i] = ((i & 0b11111000) == 0b01110000) ? /*opcode::LD_HL_R*/ opcode::UNIMPLEMENTED : opcode::LD_R_R;
		}
		//switch (i) {
		//}
	}
	return result;
}()
};


using namespace ae::cpu;

/*********************************************************************************************************************/
void Intel8080::decode_opcode(const uint8_t opcode) {
	if (opcodes[opcode] != opcode::UNIMPLEMENTED) {
		switch (opcodes[opcode]) {
		case opcode::HALT:
			break;
		case opcode::LD_R_R:
			decode8(opcode >> 3) = decode8(opcode);
			break;
		}
		return;
	}
	uint16_t cycle = 0;
	uint16_t tmp16 = 0;

	switch (opcode) {
	case 0x00: /* NOP */
		cycle = 4;
		break;
	case 0x01: /* LXI B */
		_state.c() = readArgument8();
		_state.b() = readArgument8();
		cycle = 10;
		break;
	case 0x02: /* STAX B */
		_handlerWrite(_state.bc(), _state.a());
		cycle = 7;
		break;
	case 0x03: /* INX B */
		_state.c()++;
		if (_state.c() == 0)
			_state.b()++;
		cycle = 5;
		break;
	case 0x04: /* INR B */
		_state.b() = inr(_state.b());
		cycle = 5;
		break;
	case 0x05: /* DCR B */
		_state.b() = dcr(_state.b());
		cycle = 5;
		break;
	case 0x06: /* MVI B */
		_state.b() = readArgument8();
		cycle = 7;
		break;
	case 0x07: /* RLC */
		if (_state.a() >> 7)
			_state.setFlags(Intel8080Flags::CF);
		else
			_state.resetFlags(Intel8080Flags::CF);
		//			carryBit = (_state.a() >> 7);
		_state.a() = (_state.carryFlag() ? 1 : 0) | (_state.a() << 1);
		cycle = 4;
		break;
	case 0x09: /* DAD BC */
		dad(_state.bc());
		cycle = 10;
		break;
	case 0x0A: /* LDAX B */
		_state.a() = _handlerRead(_state.bc());
		cycle = 7;
		break;
	case 0x0B: /* DCX B */
		_state.bc() = _state.bc() - 1;
		cycle = 5;
		break;
	case 0x0C: /* INR C */
		_state.c() = inr(_state.c());
		cycle = 5;
		break;
	case 0x0D: /* DCR C */
		_state.c() = dcr(_state.c());
		cycle = 5;
		break;
	case 0x0E: /* MVI C */
		_state.c() = readArgument8();
		cycle = 7;
		break;
	case 0x0F: /* RRC */
		if (_state.a() & 1)
			_state.setFlags(Intel8080Flags::CF);
		else
			_state.resetFlags(Intel8080Flags::CF);
		//			carryBit = (_state.a() & 1);
		_state.a() = (_state.carryFlag() ? 0x80 : 0) | (_state.a() >> 1);
		cycle = 4;
		break;

	case 0x11: /* LXI D */
		_state.e() = readArgument8();
		_state.d() = readArgument8();
		cycle = 10;
		break;
	case 0x12: /* STAX D */
		_handlerWrite(_state.de(), _state.a());
		cycle = 7;
		break;
	case 0x13: /* INX D */
		_state.e()++;
		if (_state.e() == 0)
			_state.d()++;
		cycle = 5;
		break;
	case 0x14: /* INR D */
		_state.d() = inr(_state.d());
		cycle = 5;
		break;
	case 0x15: /* DCR D */
		_state.d() = dcr(_state.d());
		cycle = 5;
		break;
	case 0x16: /* MVI D */
		_state.d() = readArgument8();
		cycle = 7;
		break;
	case 0x17: /* RAL */
	{
		uint8_t flag = (_state.carryFlag()) ? 1 : 0;
		if (_state.a() >> 7)
			_state.setFlags(Intel8080Flags::CF);
		else
			_state.resetFlags(Intel8080Flags::CF);

		//			carryBit = _state.a() >> 7;
		_state.a() = (_state.a() << 1) | (flag);
	}
	cycle = 4;
	break;
	case 0x19: /* DAD D */
		dad(_state.de());
		cycle = 10;
		break;
	case 0x1A: /* LDAX D */
		_state.a() = _handlerRead(_state.de());
		cycle = 7;
		break;
	case 0x1B: /* DCX D */
		_state.de() = _state.de() - 1;
		cycle = 5;
		break;
	case 0x1C: /* INR E */
		_state.e() = inr(_state.e());
		cycle = 5;
		break;
	case 0x1D: /* DCR E */
		_state.e() = dcr(_state.e());
		cycle = 5;
		break;
	case 0x1E: /* MVI E */
		_state.e() = readArgument8();
		cycle = 7;
		break;
	case 0x1F: /* RAR */
	{
		uint8_t flag = (_state.carryFlag()) ? 1 : 0;
		if (_state.a() & 1)
			_state.setFlags(Intel8080Flags::CF);
		else
			_state.resetFlags(Intel8080Flags::CF);
		//			carryBit = _state.a() & 0x01;
		_state.a() = (_state.a() >> 1) | (flag << 7);
	}
	cycle = 4;
	break;
	case 0x21: /* LXI H */
		_state.l() = readArgument8();
		_state.h() = readArgument8();
		cycle = 10;
		break;
	case 0x22: /* SHLD */
		tmp16 = readArgument16();
		_handlerWrite(tmp16, _state.l());
		_handlerWrite(tmp16 + 1, _state.h());
		cycle = 16;
		break;
	case 0x23: /* INX H */
		_state.l()++;
		if (_state.l() == 0)
			_state.h()++;
		cycle = 5;
		break;
	case 0x24: /* INR H */
		_state.h() = inr(_state.h());
		cycle = 5;
		break;
	case 0x25: /* DCR H */
		_state.h() = dcr(_state.h());
		cycle = 5;
		break;
	case 0x26: /* MVI H */
		_state.h() = readArgument8();
		cycle = 7;
		break;
	case 0x27: /* DAA */
		daa();
		cycle = 4;
		break;
	case 0x29: /* DAD H */
		dad(_state.hl());
		cycle = 10;
		break;
	case 0x2A: /* LHLD */
		tmp16 = readArgument16();
		_state.l() = _handlerRead(tmp16);
		_state.h() = _handlerRead(tmp16 + 1);
		cycle = 16;
		break;
	case 0x2B: /* DCX H */
		tmp16 = _state.hl() - 1;
		_state.h() = tmp16 >> 8;
		_state.l() = tmp16 & 0xff;
		cycle = 5;
		break;
	case 0x2C: /* INR L */
		_state.l() = inr(_state.l());
		cycle = 5;
		break;
	case 0x2D: /* DCR L */
		_state.l() = dcr(_state.l());
		cycle = 5;
		break;
	case 0x2E: /* MVI L */
		_state.l() = readArgument8();
		cycle = 7;
		break;
	case 0x2F: /* CMA */
		_state.a() = ~_state.a();
		cycle = 4;
		break;

	case 0x31: /* LXI SP */
		_state.sp() = readArgument16();
		cycle = 10;
		break;
	case 0x32: /* STA */
		_handlerWrite(readArgument16(), _state.a());
		cycle = 13;
		break;
	case 0x33: /* INX SP */
		_state.sp() = _state.sp() + 1;
		cycle = 5;
		break;
	case 0x34: /* INR M */
		_handlerWrite(_state.hl(), inr(get_m()));
		cycle = 10;
		break;
	case 0x35: /* DCR M */
		_handlerWrite(_state.hl(), dcr(get_m()));
		cycle = 10;
		break;
	case 0x36: /* MVI M */
		_handlerWrite(_state.hl(), readArgument8());
		cycle = 10;
		break;
	case 0x37: /* STC */
		_state.setFlags(Intel8080Flags::CF);
		//			carryBit = 1;
		cycle = 4;
		break;
	case 0x39: /* DAD SP */
		dad(_state.sp());
		cycle = 10;
		break;
	case 0x3A: /* LDA */
		_state.a() = _handlerRead(readArgument16());
		cycle = 13;
		break;
	case 0x3B: /* DCX SP */
		_state.sp() = _state.sp() - 1;
		cycle = 5;
		break;
	case 0x3C: /* INR A */
		_state.a() = inr(_state.a());
		cycle = 5;
		break;
	case 0x3D: /* DCR A */
		_state.a() = dcr(_state.a());
		cycle = 5;
		break;
	case 0x3E: /* MVI A */
		_state.a() = readArgument8();
		cycle = 7;
		break;
	case 0x3F: /* CMC */
		if (_state.carryFlag())
			_state.resetFlags(Intel8080Flags::CF);
		else
			_state.setFlags(Intel8080Flags::CF);

		//			carryBit = 1 - carryBit;
		cycle = 4;
		break;
	//case 0x40: /* MOV B,B */
	//	_state.b() = _state.b();
	//	cycle = 5;
	//	break;
	//case 0x41: /* MOV B,C */
	//	_state.b() = _state.c();
	//	cycle = 5;
	//	break;
	//case 0x42: /* MOV B,D */
	//	_state.b() = _state.d();
	//	cycle = 5;
	//	break;
	//case 0x43: /* MOV B,E */
	//	_state.b() = _state.e();
	//	cycle = 5;
	//	break;
	//case 0x44: /* MOV B,H */
	//	_state.b() = _state.h();
	//	cycle = 5;
	//	break;
	//case 0x45: /* MOV B,L */
	//	_state.b() = _state.l();
	//	cycle = 5;
	//	break;
	case 0x46: /* MOV B,M */
		_state.b() = get_m();
		cycle = 7;
		break;
	//case 0x47: /* MOV B,A */
	//	_state.b() = _state.a();
	//	cycle = 5;
	//	break;
	//case 0x48: /* MOV C,B */
	//	_state.c() = _state.b();
	//	cycle = 5;
	//	break;
	//case 0x49: /* MOV C,C */
	//	_state.c() = _state.c();
	//	cycle = 5;
	//	break;
	//case 0x4A: /* MOV C,D */
	//	_state.c() = _state.d();
	//	cycle = 5;
	//	break;
	//case 0x4B: /* MOV C,E */
	//	_state.c() = _state.e();
	//	cycle = 5;
	//	break;
	//case 0x4C: /* MOV C,H */
	//	_state.c() = _state.h();
	//	cycle = 5;
	//	break;
	//case 0x4D: /* MOV C,L */
	//	_state.c() = _state.l();
	//	cycle = 5;
	//	break;
	case 0x4E: /* MOV C,M */
		_state.c() = get_m();
		cycle = 7;
		break;
	//case 0x4F: /* MOV C,A */
	//	_state.c() = _state.a();
	//	cycle = 5;
	//	break;

	//case 0x50: /* MOV D,B */
	//	_state.d() = _state.b();
	//	cycle = 5;
	//	break;
	//case 0x51: /* MOV D,C */
	//	_state.d() = _state.c();
	//	cycle = 5;
	//	break;
	//case 0x52: /* MOV D,D */
	//	_state.d() = _state.d();
	//	cycle = 5;
	//	break;
	//case 0x53: /* MOV D,E */
	//	_state.d() = _state.e();
	//	cycle = 5;
	//	break;
	//case 0x54: /* MOV D,H */
	//	_state.d() = _state.h();
	//	cycle = 5;
	//	break;
	//case 0x55: /* MOV D,L */
	//	_state.d() = _state.l();
	//	cycle = 5;
	//	break;
	case 0x56: /* MOV D,M */
		_state.d() = get_m();
		cycle = 7;
		break;
	//case 0x57: /* MOV D,A */
	//	_state.d() = _state.a();
	//	cycle = 5;
	//	break;
	//case 0x58: /* MOV E,B */
	//	_state.e() = _state.b();
	//	cycle = 5;
	//	break;
	//case 0x59: /* MOV E,C */
	//	_state.e() = _state.c();
	//	cycle = 5;
	//	break;
	//case 0x5A: /* MOV E,D */
	//	_state.e() = _state.d();
	//	cycle = 5;
	//	break;
	//case 0x5B: /* MOV E,E */
	//	_state.e() = _state.e();
	//	cycle = 5;
	//	break;
	//case 0x5C: /* MOV E,H */
	//	_state.e() = _state.h();
	//	cycle = 5;
	//	break;
	//case 0x5D: /* MOV E,L */
	//	_state.e() = _state.l();
	//	cycle = 5;
	//	break;
	case 0x5E: /* MOV E,M */
		_state.e() = get_m();
		cycle = 7;
		break;
	//case 0x5F: /* MOV E,A */
	//	_state.e() = _state.a();
	//	cycle = 5;
	//	break;

	//case 0x60: /* MOV H,B */
	//	_state.h() = _state.b();
	//	cycle = 5;
	//	break;
	//case 0x61: /* MOV H,C */
	//	_state.h() = _state.c();
	//	cycle = 5;
	//	break;
	//case 0x62: /* MOV H,D */
	//	_state.h() = _state.d();
	//	cycle = 5;
	//	break;
	//case 0x63: /* MOV H,E */
	//	_state.h() = _state.e();
	//	cycle = 5;
	//	break;
	//case 0x64: /* MOV H,H */
	//	_state.h() = _state.h();
	//	cycle = 5;
	//	break;
	//case 0x65: /* MOV H,L */
	//	_state.h() = _state.l();
	//	cycle = 5;
	//	break;
	case 0x66: /* MOV H,M */
		_state.h() = get_m();
		cycle = 7;
		break;
	//case 0x67: /* MOV H,A */
	//	_state.h() = _state.a();
	//	cycle = 5;
	//	break;
	//case 0x68: /* MOV L,B */
	//	_state.l() = _state.b();
	//	cycle = 5;
	//	break;
	//case 0x69: /* MOV L,C */
	//	_state.l() = _state.c();
	//	cycle = 5;
	//	break;
	//case 0x6A: /* MOV L,D */
	//	_state.l() = _state.d();
	//	cycle = 5;
	//	break;
	//case 0x6B: /* MOV L,E */
	//	_state.l() = _state.e();
	//	cycle = 5;
	//	break;
	//case 0x6C: /* MOV L,H */
	//	_state.l() = _state.h();
	//	cycle = 5;
	//	break;
	//case 0x6D: /* MOV L,L */
	//	_state.l() = _state.l();
	//	cycle = 5;
	//	break;
	case 0x6E: /* MOV L,M */
		_state.l() = get_m();
		cycle = 7;
		break;
	//case 0x6F: /* MOV L,A */
	//	_state.l() = _state.a();
	//	cycle = 5;
	//	break;

	case 0x70: /* MOV M,B */
		_handlerWrite(_state.hl(), _state.b());
		cycle = 7;
		break;
	case 0x71: /* MOV M,C */
		_handlerWrite(_state.hl(), _state.c());
		cycle = 7;
		break;
	case 0x72: /* MOV M,D */
		_handlerWrite(_state.hl(), _state.d());
		cycle = 7;
		break;
	case 0x73: /* MOV M,E */
		_handlerWrite(_state.hl(), _state.e());
		cycle = 7;
		break;
	case 0x74: /* MOV M,H */
		_handlerWrite(_state.hl(), _state.h());
		cycle = 7;
		break;
	case 0x75: /* MOV M,L */
		_handlerWrite(_state.hl(), _state.l());
		cycle = 7;
		break;
	case 0x77: /* MOV M,A */
		_handlerWrite(_state.hl(), _state.a());
		cycle = 7;
		break;
	//case 0x78: /* MOV A,B */
	//	_state.a() = _state.b();
	//	cycle = 5;
	//	break;
	//case 0x79: /* MOV A,C */
	//	_state.a() = _state.c();
	//	cycle = 5;
	//	break;
	//case 0x7A: /* MOV A,D */
	//	_state.a() = _state.d();
	//	cycle = 5;
	//	break;
	//case 0x7B: /* MOV A,E */
	//	_state.a() = _state.e();
	//	cycle = 5;
	//	break;
	//case 0x7C: /* MOV A,H */
	//	_state.a() = _state.h();
	//	cycle = 5;
	//	break;
	//case 0x7D: /* MOV A,L */
	//	_state.a() = _state.l();
	//	cycle = 5;
	//	break;
	case 0x7E: /* MOV A,M */
		_state.a() = get_m();
		cycle = 7;
		break;
	//case 0x7F: /* MOV A,A */
	//	_state.a() = _state.a();
	//	cycle = 5;
	//	break;

	case 0x80: /* ADD B */
		add(_state.b());
		cycle = 4;
		break;
	case 0x81: /* ADD C */
		add(_state.c());
		cycle = 4;
		break;
	case 0x82: /* ADD D */
		add(_state.d());
		cycle = 4;
		break;
	case 0x83: /* ADD E */
		add(_state.e());
		cycle = 4;
		break;
	case 0x84: /* ADD H */
		add(_state.h());
		cycle = 4;
		break;
	case 0x85: /* ADD L */
		add(_state.l());
		cycle = 4;
		break;
	case 0x86: /* ADD M */
		add(get_m());
		cycle = 7;
		break;
	case 0x87: /* ADD A */
		add(_state.a());
		cycle = 4;
		break;
	case 0x88: /* ADC B */
		adc(_state.b());
		cycle = 4;
		break;
	case 0x89: /* ADC C */
		adc(_state.c());
		cycle = 4;
		break;
	case 0x8A: /* ADC D */
		adc(_state.d());
		cycle = 4;
		break;
	case 0x8B: /* ADC E */
		adc(_state.e());
		cycle = 4;
		break;
	case 0x8C: /* ADC H */
		adc(_state.h());
		cycle = 4;
		break;
	case 0x8D: /* ADC L */
		adc(_state.l());
		cycle = 4;
		break;
	case 0x8E: /* ADC M */
		adc(get_m());
		cycle = 7;
		break;
	case 0x8F: /* ADC A */
		adc(_state.a());
		cycle = 4;
		break;

	case 0x90: /* SUB B */
		sub(_state.b());
		cycle = 4;
		break;
	case 0x91: /* SUB C */
		sub(_state.c());
		cycle = 4;
		break;
	case 0x92: /* SUB D */
		sub(_state.d());
		cycle = 4;
		break;
	case 0x93: /* SUB E */
		sub(_state.e());
		cycle = 4;
		break;
	case 0x94: /* SUB H */
		sub(_state.h());
		cycle = 4;
		break;
	case 0x95: /* SUB L */
		sub(_state.l());
		cycle = 4;
		break;
	case 0x96: /* SUB M */
		sub(get_m());
		cycle = 7;
		break;
	case 0x97: /* SUB A */
		sub(_state.a());
		cycle = 4;
		break;
	case 0x98: /* SBB B */
		sbb(_state.b());
		cycle = 4;
		break;
	case 0x99: /* SBB C */
		sbb(_state.c());
		cycle = 4;
		break;
	case 0x9A: /* SBB D */
		sbb(_state.d());
		cycle = 4;
		break;
	case 0x9B: /* SBB E */
		sbb(_state.e());
		cycle = 4;
		break;
	case 0x9C: /* SBB H */
		sbb(_state.h());
		cycle = 4;
		break;
	case 0x9D: /* SBB L */
		sbb(_state.l());
		cycle = 4;
		break;
	case 0x9E: /* SBB M */
		sbb(get_m());
		cycle = 7;
		break;
	case 0x9F: /* SBB A */
		sbb(_state.a());
		cycle = 4;
		break;

	case 0xA0: /* ANA B */
		ana(_state.b());
		cycle = 4;
		break;
	case 0xA1: /* ANA C */
		ana(_state.c());
		cycle = 4;
		break;
	case 0xA2: /* ANA D */
		ana(_state.d());
		cycle = 4;
		break;
	case 0xA3: /* ANA E */
		ana(_state.e());
		cycle = 4;
		break;
	case 0xA4: /* ANA H */
		ana(_state.h());
		cycle = 4;
		break;
	case 0xA5: /* ANA L */
		ana(_state.l());
		cycle = 4;
		break;
	case 0xA6: /* ANA M */
		ana(get_m());
		cycle = 7;
		break;
	case 0xA7: /* ANA A */
		ana(_state.a());
		cycle = 4;
		break;
	case 0xA8: /* XRA B */
		xra(_state.b());
		cycle = 4;
		break;
	case 0xA9: /* XRA C */
		xra(_state.c());
		cycle = 4;
		break;
	case 0xAA: /* XRA D */
		xra(_state.d());
		cycle = 4;
		break;
	case 0xAB: /* XRA E */
		xra(_state.e());
		cycle = 4;
		break;
	case 0xAC: /* XRA H */
		xra(_state.h());
		cycle = 4;
		break;
	case 0xAD: /* XRA L */
		xra(_state.l());
		cycle = 4;
		break;
	case 0xAE: /* XRA M */
		xra(get_m());
		cycle = 7;
		break;
	case 0xAF: /* XRA A */
		xra(_state.a());
		cycle = 4;
		break;

	case 0xB0: /* ORA B */
		ora(_state.b());
		cycle = 4;
		break;
	case 0xB1: /* ORA C */
		ora(_state.c());
		cycle = 4;
		break;
	case 0xB2: /* ORA D */
		ora(_state.d());
		cycle = 4;
		break;
	case 0xB3: /* ORA E */
		ora(_state.e());
		cycle = 4;
		break;
	case 0xB4: /* ORA H */
		ora(_state.h());
		cycle = 4;
		break;
	case 0xB5: /* ORA L */
		ora(_state.l());
		cycle = 4;
		break;
	case 0xB6: /* ORA M */
		ora(get_m());
		cycle = 7;
		break;
	case 0xB7: /* ORA A */
		ora(_state.a());
		cycle = 4;
		break;
	case 0xB8: /* CMP B */
		cmp(_state.b());
		cycle = 4;
		break;
	case 0xB9: /* CMP C */
		cmp(_state.c());
		cycle = 4;
		break;
	case 0xBA: /* CMP D */
		cmp(_state.d());
		cycle = 4;
		break;
	case 0xBB: /* CMP E */
		cmp(_state.e());
		cycle = 4;
		break;
	case 0xBC: /* CMP H */
		cmp(_state.h());
		cycle = 4;
		break;
	case 0xBD: /* CMP L */
		cmp(_state.l());
		cycle = 4;
		break;
	case 0xBE: /* CMP M */
		cmp(get_m());
		cycle = 7;
		break;
	case 0xBF: /* CMP A */
		cmp(_state.a());
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
	case 0xC1: /* POP BC */ { uint16_t t = popOfStack(); _state.b() = t >> 8; _state.c() = t & 0xff; } cycle = 10; break;
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
	case 0xC5: /* PUSH BC */ pushToStack((_state.b() << 8) | _state.c()); cycle = 11; break;
	case 0xC6: /* ADI */
		add(readArgument8());
		cycle = 7;
		break;
	case 0xC7: /* RST 0 */
		pushToStack(pc);
		pc = 0;
		break;
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
	case 0xCE: /* ACI */
		adc(readArgument8());
		cycle = 7;
		break;
		//		case 0xCF: /* RST 1 */

	case 0xD0: /* RNC */
		if (!_state.carryFlag()) {
			pc = popOfStack();
			cycle = 11;
		}
		else {
			cycle = 5;
		}
		break;
	case 0xD1: /* POP DE */
		_state.de() = popOfStack();
		cycle = 10;
		break;
	case 0XD2: /* JNC */
		tmp16 = readArgument16();
		if (!_state.carryFlag())
			pc = tmp16;
		cycle = 10;
		break;
	case 0xD3: /* OUT */
		_handlerOut(readArgument8(), _state.a());
		cycle = 10;
		break;
	case 0xD4: /* CNC */
		tmp16 = readArgument16();
		if (!_state.carryFlag()) {
			pushToStack(pc);
			pc = tmp16;
			cycle = 17;
		}
		else {
			cycle = 11;
		}
		break;
	case 0xD5: /* PUSH DE */
		pushToStack(_state.de());
		cycle = 11;
		break;
	case 0XD6: /* SUI */
		sub(readArgument8());
		cycle = 7;
		break;
		//		case 0xD7: /* RST 2 */
	case 0XD8: /* RC */
		if (_state.carryFlag()) {
			pc = popOfStack();
			cycle = 11;
		}
		else {
			cycle = 5;
		}
		break;
	case 0xDA: /* JC */
		tmp16 = readArgument16();
		if (_state.carryFlag())
			pc = tmp16;
		cycle = 10;
		break;
	case 0XDB: /* IN */
		_state.a() = _handlerIn(readArgument8());
		cycle = 10;
		break;
	case 0xDC: /* CC */
		tmp16 = readArgument16();
		if (_state.carryFlag()) {
			pushToStack(pc);
			pc = tmp16;
			cycle = 17;
		}
		else {
			cycle = 11;
		}
		break;
	case 0xDE: /* SBI */
		sbb(readArgument8());
		cycle = 7;
		break;
		//		case 0xDF: /* RST 3 */

	case 0xE0: /* RPO */
		if (!parityBit) {
			pc = popOfStack();
			cycle = 11;
		}
		else {
			cycle = 5;
		}
		break;
	case 0xE1: /* POP HL */
		_state.hl() = popOfStack();
		cycle = 10;
		break;
	case 0xE2: /* JPO */
		tmp16 = readArgument16();
		if (!parityBit)
			pc = tmp16;
		cycle = 10;
		break;
	case 0xE3: /* XTHL */
		tmp16 = _handlerRead(_state.sp()) | (_handlerRead(_state.sp() + 1) << 8);
		_handlerWrite(_state.sp(), _state.l());
		_handlerWrite(_state.sp() + 1, _state.h());
		_state.hl() = tmp16;
		cycle = 18;
		break;
	case 0xE4: /* CPO */
		tmp16 = readArgument16();
		if (!parityBit) {
			pushToStack(pc);
			pc = tmp16;
			cycle = 17;
		}
		else {
			cycle = 11;
		}
		break;
	case 0xE5: /* PUSH HL */ pushToStack((_state.h() << 8) | _state.l()); cycle = 11; break;
	case 0xE6: /* ANI */
		ana(readArgument8());
		cycle = 7;
		break;
		//		case 0XE7: /* RST 4 */
	case 0xE8: /* RPE */
		if (parityBit) {
			pc = popOfStack();
			cycle = 11;
		}
		else {
			cycle = 5;
		}
		break;
	case 0xE9: /* PCHL */
		pc = _state.hl();
		cycle = 5;
		break;
	case 0xEA: /* JPE */
		tmp16 = readArgument16();
		if (parityBit)
			pc = tmp16;
		cycle = 10;
		break;
	case 0xEB: /* XCHG */ { uint8_t tmp = _state.d(); _state.d() = _state.h(); _state.h() = tmp; tmp = _state.e(); _state.e() = _state.l(); _state.l() = tmp; } cycle = 4; break;
	case 0xEC: /* CPE */
		tmp16 = readArgument16();
		if (parityBit) {
			pushToStack(pc);
			pc = tmp16;
			cycle = 17;
		}
		else {
			cycle = 11;
		}
		break;
	case 0xEE: /* XRI */
		xra(readArgument8());
		cycle = 7;
		break;
		//		case 0xEF: /* RST 5 */

	case 0xF0: /* RP */
		if (!signBit) {
			pc = popOfStack();
			cycle = 11;
		}
		else {
			cycle = 5;
		}
		break;
	case 0xF1: /* POP PSW */ { uint16_t t = popOfStack(); _state.a() = t >> 8; parityBit = (t >> 2) & 1; zeroBit = (t >> 6) & 1; signBit = (t >> 7) & 1; _state.f() = t; auxCarryBit = (t >> 4) & 1; } cycle = 10; break;
	case 0xF2: /* JP */
		tmp16 = readArgument16();
		if (!signBit)
			pc = tmp16;
		cycle = 10;
		break;
	case 0xF3: /* DI */
		interrupt_enabled = 0;
		cycle = 4;
		break;
	case 0xF4: /* CP */
		tmp16 = readArgument16();
		if (!signBit) {
			pushToStack(pc);
			pc = tmp16;
			cycle = 17;
		}
		else {
			cycle = 11;
		}
		break;
	case 0xF5: /* PUSH PSW */ pushToStack((_state.a() << 8) | (signBit << 7) | (zeroBit << 6) | (auxCarryBit << 4) | (parityBit << 2) | 2 | (_state.f() & 1)); cycle = 11; break;
	case 0xF6: /* ORI */
		ora(readArgument8());
		cycle = 7;
		break;
		//		case 0xF7: /* RST 6 */
	case 0xF8: /* RM */
		if (signBit) {
			pc = popOfStack();
			cycle = 11;
		}
		else {
			cycle = 5;
		}
		break;
	case 0xF9: /* SPHL */
		_state.sp() = _state.hl();
		cycle = 5;
		break;
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
}
