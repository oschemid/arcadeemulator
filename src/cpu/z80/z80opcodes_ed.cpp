#include "../z80.h"
#include "types.h"
#include <array>


enum class opcode {
	UNIMPLEMENTED,
	ADC,
	SBC,
	CPD,
	CPDR,
	CPI,
	CPIR,
	LD_SS_NNNN,
	LD_NNNN_SS,
	LDD,
	LDDR,
	LDI,
	LDIR,
	NEG,
	RRD,
	RLD,
	IM0,
	IM1,
	IM2,
	LD_A_I,
	LD_A_R,
	LD_I_A,
	LD_R_A,
};


/*********************************************************************************************************************/
// opcodes table
constexpr auto opcodes{ []() constexpr {
	std::array<opcode,256> result{opcode::UNIMPLEMENTED};
	for (int i = 0; i < 256; ++i) {
		if ((i & 0b11001111) == 0b01001010)
			result[i] = opcode::ADC;
		if ((i & 0b11001111) == 0b01000010)
			result[i] = opcode::SBC;
		if ((i & 0b11001111) == 0b01001011)
			result[i] = opcode::LD_SS_NNNN;
		if ((i & 0b11001111) == 0b01000011)
			result[i] = opcode::LD_NNNN_SS;
		switch (i) {
		case 0X44:
			result[i] = opcode::NEG;
			break;
		case 0x46:
			result[i] = opcode::IM0;
			break;
		case 0x47:
			result[i] = opcode::LD_I_A;
			break;
		case 0x4F:
			result[i] = opcode::LD_R_A;
			break;
		case 0x56:
			result[i] = opcode::IM1;
			break;
		case 0x57:
			result[i] = opcode::LD_A_I;
			break;
		case 0x5E:
			result[i] = opcode::IM2;
			break;
		case 0x5F:
			result[i] = opcode::LD_A_R;
			break;
		case 0x67:
			result[i] = opcode::RRD;
			break;
		case 0x6F:
			result[i] = opcode::RLD;
			break;
		case 0xA9:
			result[i] = opcode::CPD;
			break;
		case 0xB9:
			result[i] = opcode::CPDR;
			break;
		case 0xA0:
			result[i] = opcode::LDI;
			break;
		case 0xA1:
			result[i] = opcode::CPI;
			break;
		case 0xA8:
			result[i] = opcode::LDD;
			break;
		case 0xB0:
			result[i] = opcode::LDIR;
			break;
		case 0xB1:
			result[i] = opcode::CPIR;
			break;
		case 0xB8:
			result[i] = opcode::LDDR;
			break;
		}
	}
	return result;
}()
};

using namespace ae::cpu;

/*********************************************************************************************************************/
void Z80::decode_opcode_ed() {
	const opcode_t opcode = readOpcode();
	if (opcodes[opcode] != opcode::UNIMPLEMENTED) {

		switch (opcodes[opcode]) {
		case opcode::IM0:
			im = interrupt_mode::mode_0;
			break;
		case opcode::IM1:
			im = interrupt_mode::mode_1;
			break;
		case opcode::IM2:
			im = interrupt_mode::mode_2;
			break;
		case opcode::LD_I_A:
			_state.i() = _state.a();
			_elapsed_cycles++;
			break;
		case opcode::LD_R_A:
			_state.r() = _state.a();
			_elapsed_cycles++;
			break;
		case opcode::LD_A_I:
			_state.a() = _state.i();
			_state.resetFlags(Z80State::NF | Z80State::HF | Z80State::PF);
			_state.setSZXY(_state.a());
			if (iff2)
				_state.setFlags(Z80State::PF);
			_elapsed_cycles++;
			break;
		case opcode::LD_A_R:
			_state.a() = _state.r();
			_state.resetFlags(Z80State::NF | Z80State::HF | Z80State::PF);
			_state.setSZXY(_state.a());
			if (iff2)
				_state.setFlags(Z80State::PF);
			_elapsed_cycles++;
			break;
		case opcode::ADC:
			adc_ss(opcode);
			break;
		case opcode::SBC:
			sbc_ss(opcode);
			break;
		case opcode::CPD:
			cpd();
			_elapsed_cycles += 4;
			break;
		case opcode::CPDR:
			cpd();
			if ((_state.f() & (flags::parityFlag | flags::zeroFlag)) == 0) {

				//		_elapsed_cycles += 4;
				//		if ((!_state.zeroFlag()) && (_state.bc() = !0)) {
				pc -= 2;
				//			_elapsed_cycles += 5;
			}
			break;
		case opcode::CPI:
			cpi();
			_elapsed_cycles += 4;
			break;
		case opcode::CPIR:
			cpi();
			if ((_state.f() & (flags::parityFlag | flags::zeroFlag)) == 0) {
				//		_elapsed_cycles += 4;
				//		if ((!_state.zeroFlag()) && (_state.bc() = !0)) {
				pc -= 2;
				//			_elapsed_cycles += 5;
			}
			break;
		case opcode::LDD:
			ldd();
			_elapsed_cycles++;
			break;
		case opcode::LDDR:
			ldd();
			_elapsed_cycles++;
			if (_state.bc() != 0) {
				pc -= 2;
				_elapsed_cycles += 5;
			}
			break;
		case opcode::LDI:
			ldi();
			_elapsed_cycles++;
			break;
		case opcode::LDIR:
			ldi();
			_elapsed_cycles++;
			if (_state.bc() != 0) {
				pc -= 2;
				_elapsed_cycles += 5;
			}
			break;
		case opcode::NEG:
			neg();
			break;
		case opcode::RLD:
			rld();
			break;
		case opcode::RRD:
			rrd();
			break;
		case opcode::LD_SS_NNNN:
			decode16(opcode) = read16(readArgument16());
			break;
		case opcode::LD_NNNN_SS:
			write16(readArgument16(), decode16(opcode));
			break;
		default:
			unimplemented();
			break;
		}
		return;
	}
}

void Z80::rrd() {
	uint8_t tmp = _state.a() & 0x0f;
	uint8_t mem = read8(_state.hl());
	_state.a() = (_state.a() & 0xf0) | (mem & 0x0f);
	write8(_state.hl(), static_cast<uint8_t>((mem >> 4) | (tmp << 4)));
	_elapsed_cycles += 3; // write on HL bis

	_state.f() &= Z80State::CF;
	_state.setSZXY(_state.a());
	_state.setP(_state.a());
}

void Z80::rld() {
	uint8_t tmp = _state.a() & 0x0f;
	uint8_t mem = read8(_state.hl());
	_state.a() = (_state.a() & 0xf0) | (mem >> 4);
	write8(_state.hl(), static_cast<uint8_t>((mem << 4) | tmp));
	_elapsed_cycles += 3; // write on HL bis

	_state.f() &= Z80State::CF;
	_state.setSZXY(_state.a());
	_state.setP(_state.a());
}

void Z80::adc_ss(const opcode_t opcode) {
	const uint16_t b = decode16(opcode);
	const uint32_t s = _state.hl() + b + (_state.carryFlag() ? 1 : 0);
	const uint32_t c = s ^ _state.hl() ^ b;
	_state.hl() = static_cast<uint16_t>(s);
	_state.resetFlags(Z80State::NF | Z80State::CF | Z80State::HF | Z80State::PF);
	if (s > 0xffff)
		_state.setFlags(Z80State::CF);
	if (c & 0x1000)
		_state.setFlags(Z80State::HF);
	if (((c >> 15) & 0x1) ^ ((c >> 16) & 0x1))
		_state.setFlags(Z80State::PF);
	_state.setSZXY(_state.hl());
	_elapsed_cycles += 7;
}
void Z80::sbc_ss(const opcode_t opcode) {
	const uint16_t b = decode16(opcode);
	const uint32_t s = _state.hl() - b - (_state.carryFlag() ? 1 : 0);
	const uint32_t c = (s ^ _state.hl() ^ b) & 0x18000;
	_state.hl() = static_cast<uint16_t>(s);
	_state.resetFlags(Z80State::CF | Z80State::HF | Z80State::PF);
	_state.setFlags(Z80State::NF);
	if (s > 0xffff)
		_state.setFlags(Z80State::CF);
	if (c & 0x1000)
		_state.setFlags(Z80State::HF);
	if (((c >> 15) & 0x1) ^ ((c >> 16) & 0x1))
		_state.setFlags(Z80State::PF);
	_state.setSZXY(_state.hl());
	_elapsed_cycles += 7;
}
void Z80::neg() {
	const uint8_t ap = _state.a();
	_state.a() = -_state.a();

	_state.setSZXY(_state.a());
	_state.resetFlags(Z80State::PF | Z80State::CF | Z80State::HF);
	if (ap != 0)
		_state.setFlags(Z80State::CF);
	if (ap == 0x80)
		_state.setFlags(Z80State::PF);
	_state.setFlags(Z80State::NF);
	if ((_state.a() ^ ap) & 0x10)
		_state.setFlags(Z80State::HF);
}
void Z80::cpd() {
	uint8_t m = read8(_state.hl());

	//	const uint8_t value = read8(_state.hl());
	//	const uint16_t sum = _state.a() - value;
	//	const uint16_t carryIns = (sum ^ _state.a() ^ value);

	_state.setFlags(Z80State::NF);
	_state.resetFlags(Z80State::SF | Z80State::ZF | Z80State::HF | Z80State::PF);
	if (_state.a() < m)
		_state.setFlags(Z80State::SF);
	if (_state.a() == m)

		//	if ((sum & 0xff) == 0)
		_state.setFlags(Z80State::ZF);
	//	else
	//		_state.setFlags(sum & Z80State::SF);
	//	if (carryIns & 0x10)
	//		_state.setFlags(Z80State::HF);
	uint16_t carryIns = ((_state.a() - m) ^ _state.a() ^ m);
	if ((carryIns >> 4) & 0x1)
		_state.setFlags(Z80State::HF);

	--_state.hl();
	--_state.bc();
	//	if (_state.bc() != 0)
	if (_state.bc() == 0)
		_state.setFlags(Z80State::PF);
}
void Z80::cpi() {
	uint8_t m = read8(_state.hl());

	_state.setFlags(Z80State::NF);
	_state.resetFlags(Z80State::SF | Z80State::ZF | Z80State::HF | Z80State::PF);
	if (_state.a() < m)
		_state.setFlags(Z80State::SF);
	if (_state.a() == m)
		_state.setFlags(Z80State::ZF);
	uint16_t carryIns = ((_state.a() - m) ^ _state.a() ^ m);
	if ((carryIns >> 4) & 0x1)
		_state.setFlags(Z80State::HF);

	++_state.hl();
	--_state.bc();
	if (_state.bc() == 0)
		//	if (_state.bc() != 0)
		_state.setFlags(Z80State::PF);
}
void Z80::ldd() {
	write8(de(), read8(_state.hl()));
	--_state.hl();
	--_state.bc();
	--_state.de();
	_state.resetFlags(Z80State::HF | Z80State::NF | Z80State::PF);
	if (_state.bc() != 0)
		_state.setFlags(Z80State::PF);
}
void Z80::ldi() {
	write8(de(), read8(_state.hl()));
	++_state.hl();
	--_state.bc();
	++_state.de();
	_state.resetFlags(Z80State::HF | Z80State::NF | Z80State::PF);
	if (_state.bc() != 0)
		_state.setFlags(Z80State::PF);
}
