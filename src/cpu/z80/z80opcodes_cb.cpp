#include "../z80.h"
#include "types.h"
#include <array>


enum class opcode {
	UNIMPLEMENTED,
	RLC_R,
	RLC_HL,
	RL_R,
	RL_HL,
	RRC_R,
	RRC_HL,
	RR_R,
	RR_HL,
	BIT_R,
	BIT_HL,
	SET_R,
	SET_HL,
	RES_R,
	RES_HL,
	SRL_R,
	SRL_HL,
	SRA_R,
	SRA_HL,
	SLA_R,
	SLA_HL,
	SLL_R,
	SLL_HL
};


/*********************************************************************************************************************/
// opcodes table
constexpr auto opcodes{ []() constexpr {
	std::array<opcode,256> result{opcode::UNIMPLEMENTED};
	for (int i = 0; i < 256; ++i) {
		if ((i & 0b11111000) == 0b00000000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::RLC_HL : opcode::RLC_R;
		if ((i & 0b11111000) == 0b00010000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::RL_HL : opcode::RL_R;
		if ((i & 0b11111000) == 0b00001000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::RRC_HL : opcode::RRC_R;
		if ((i & 0b11111000) == 0b00011000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::RR_HL : opcode::RR_R;
		if ((i & 0b11111000) == 0b00100000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::SLA_HL : opcode::SLA_R;
		if ((i & 0b11111000) == 0b00101000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::SRA_HL : opcode::SRA_R;
		if ((i & 0b11111000) == 0b00110000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::SLL_HL : opcode::SLL_R;
		if ((i & 0b11111000) == 0b00111000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::SRL_HL : opcode::SRL_R;
		if ((i & 0b11000000) == 0b01000000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::BIT_HL : opcode::BIT_R;
		if ((i & 0b11000000) == 0b11000000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::SET_HL : opcode::SET_R;
		if ((i & 0b11000000) == 0b10000000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::RES_HL : opcode::RES_R;
	}
	return result;
}()
};

using namespace ae::cpu;

/*********************************************************************************************************************/
void Z80::decode_opcode_cb(const prefix p) {
	const int8_t delta = (p != NO) ? static_cast<int8_t>(readArgument8()) : 0;
	const opcode_t opcode = readOpcode();

	switch (opcodes[opcode]) {
	case opcode::RLC_R:
		apply_ixy_r([this](const uint8_t r) { return rlc(r); }, opcode, p, delta);
		break;
	case opcode::RLC_HL:
		apply_hl([this](const uint8_t r) { return rlc(r); }, p, delta);
		break;
	case opcode::RL_R:
		apply_ixy_r([this](const uint8_t r) { return rl(r); }, opcode, p, delta);
		break;
	case opcode::RL_HL:
		apply_hl([this](const uint8_t r) { return rl(r); }, p, delta);
		break;
	case opcode::RR_R:
		apply_ixy_r([this](const uint8_t r) { return rr(r); }, opcode, p, delta);
		break;
	case opcode::RR_HL:
		apply_hl([this](const uint8_t r) { return rr(r); }, p, delta);
		break;
	case opcode::RRC_R:
		apply_ixy_r([this](const uint8_t r) { return rrc(r); }, opcode, p, delta);
		break;
	case opcode::RRC_HL:
		apply_hl([this](const uint8_t r) { return rrc(r); }, p, delta);
		break;
	case opcode::SRL_R:
		apply_ixy_r([this](const uint8_t r) { return srl(r); }, opcode, p, delta);
		break;
	case opcode::SRL_HL:
		apply_hl([this](const uint8_t r) { return srl(r); }, p, delta);
		break;
	case opcode::SRA_R:
		apply_ixy_r([this](const uint8_t r) { return sra(r); }, opcode, p, delta);
		break;
	case opcode::SRA_HL:
		apply_hl([this](const uint8_t r) { return sra(r); }, p, delta);
		break;
	case opcode::SLA_R:
		apply_ixy_r([this](const uint8_t r) { return sla(r); }, opcode, p, delta);
		break;
	case opcode::SLA_HL:
		apply_hl([this](const uint8_t r) { return sla(r); }, p, delta);
		break;
	case opcode::SLL_R:
		apply_ixy_r([this](const uint8_t r) { return sll(r); }, opcode, p, delta);
		break;
	case opcode::SLL_HL:
		apply_hl([this](const uint8_t r) { return sll(r); }, p, delta);
		break;
	case opcode::BIT_R:
		bit(decode8(opcode), (opcode & 0b00111000) >> 3);
		break;
	case opcode::BIT_HL:
		if (p == DD)
			bit(read(_state.ix() + delta), (opcode & 0b00111000) >> 3);
		else if (p == FD)
			bit(read(_state.ix() + delta), (opcode & 0b00111000) >> 3);
		else
			bit(read(_state.hl()), (opcode & 0b00111000) >> 3);
		break;
	case opcode::SET_R:
		apply_ixy_r([this, opcode](const uint8_t r) { return r | (1 << ((opcode & 0b00111000) >> 3)); }, opcode, p, delta);
		break;
	case opcode::SET_HL:
		apply_hl([this, opcode](const uint8_t r) { return r | (1 << ((opcode & 0b00111000) >> 3)); }, p, delta);
		break;
	case opcode::RES_R:
		apply_ixy_r([this, opcode](const uint8_t r) { return r & ~(1 << ((opcode & 0b00111000) >> 3)); }, opcode, p, delta);
		break;
	case opcode::RES_HL:
		apply_hl([this, opcode](const uint8_t r) { return r & ~(1 << ((opcode & 0b00111000) >> 3)); }, p, delta);
		break;
	default:
		throw std::runtime_error("Unexpected opcode in decode_cb : " + opcode);
	}
}
/*********************************************************************************************************************/

uint8_t Z80::rlc(const uint8_t value) {
	const uint8_t result = (value << 1) | (value >> 7);
	_state.f() = (value & 0x80) ? Z80State::CF : 0;
	_state.setSZXY(result);
	_state.setP(result);
	return result;
}
uint8_t Z80::rrc(const uint8_t value) {
	const uint8_t result = (value >> 1) | (value << 7);
	_state.f() = (value & 0x01) ? Z80State::CF : 0;
	_state.setSZXY(result);
	_state.setP(result);
	return result;
}
uint8_t Z80::rr(const uint8_t value) {
	const uint8_t result = (value >> 1) | ((_state.carryFlag()) ? 0x80 : 0);
	_state.f() = (value & 0x01) ? Z80State::CF : 0;
	_state.setSZXY(result);
	_state.setP(result);
	return result;
}
uint8_t Z80::rl(const uint8_t value) {
	const uint8_t result = (value << 1) | ((_state.carryFlag()) ? 0x01 : 0);
	_state.f() = (value & 0x80) ? Z80State::CF : 0;
	_state.setSZXY(result);
	_state.setP(result);
	return result;
}
uint8_t Z80::srl(const uint8_t value) {
	const uint8_t result = value >> 1;
	_state.f() = (value & 0x01) ? Z80State::CF : 0;
	_state.setSZXY(result);
	_state.setP(result);
	return result;
}
uint8_t Z80::sra(const uint8_t value) {
	const uint8_t result = value >> 1 | (value & 0x80);
	_state.f() = (value & 0x01) ? Z80State::CF : 0;
	_state.setSZXY(result);
	_state.setP(result);
	return result;
}
uint8_t Z80::sla(const uint8_t value) {
	const uint8_t result = value << 1;
	_state.f() = (value & 0x80) ? Z80State::CF : 0;
	_state.setSZXY(result);
	_state.setP(result);
	return result;
}
uint8_t Z80::sll(const uint8_t value) {
	const uint8_t result = (value << 1) | 1;
	_state.f() = (value & 0x80) ? Z80State::CF : 0;
	_state.setSZXY(result);
	_state.setP(result);
	return result;
}
void Z80::bit(const uint8_t value, const uint8_t b) {
	_state.f() &= Z80State::CF;
	_state.f() |= Z80State::HF;
	_state.f() |= (value & (1 << b)) ? Z80State::SF : Z80State::ZF | Z80State::VF;
}