#include "../z80.h"
#include "types.h"
#include <array>


enum class opcode {
	UNIMPLEMENTED,
	NOP,
	HALT,
	DI,
	EI,
	DECODE_CB,
	DECODE_DD,
	DECODE_ED,
	DECODE_FD,

	ADD_HL_SS,
	ADD_N,
	ADD_R,
	ADD_HL,
	ADC_N,
	ADC_R,
	ADC_HL,
	SUB_N,
	SUB_R,
	SUB_HL,
	SBC_N,
	SBC_R,
	SBC_HL,
	AND_N,
	AND_R,
	AND_HL,
	XOR_N,
	XOR_R,
	XOR_HL,
	OR_N,
	OR_R,
	OR_HL,
	CP_N,
	CP_R,
	CP_HL,
	INC_R,
	INC_HL,
	INC_SS,
	DEC_R,
	DEC_HL,
	DEC_SS,
	DAA,
	CPL,
	SCF,
	CCF,
	LD_HL_NNNN,
	LD_NNNN_HL,
	LD_SS_NN,
	LD_A_DE,
	LD_DE_A,
	LD_A_BC,
	LD_BC_A,
	LD_R_N,
	LD_HL_N,
	LD_R_HL,
	LD_R_R,
	LD_HL_R,
	LD_A_NNNN,
	LD_NNNN_A,
	RLCA,
	RRCA,
	RLA,
	RRA,
	EX_DE,
	EX_AF,
	EXX,
	EX_SP,
	LD_SP_HL,
	JP,
	JP_CC,
	JP_HL,
	JR,
	JR_C,
	DJNZ,
	PUSH,
	POP,
};


/*********************************************************************************************************************/
// OPCODES TABLES
constexpr auto opcodes{ []() constexpr {
	std::array<opcode,256> result{opcode::UNIMPLEMENTED};
	for (int i = 0; i < 256; ++i) {
		if ((i & 0b11000000) == 0b01000000) {
			if ((i & 0b11000111) == 0b01000110)
				result[i] = (i == 0b01110110) ? opcode::HALT : opcode::LD_R_HL;
			else if ((i & 0b11111000) == 0b01110000)
				result[i] = opcode::LD_HL_R;
			else
				result[i] = opcode::LD_R_R;
		}
		if ((i & 0b11000111) == 0b00000110)
			result[i] = (i == 0b00110110) ? opcode::LD_HL_N : opcode::LD_R_N;
		if ((i & 0b11001111) == 0b00000001)
			result[i] = opcode::LD_SS_NN;
		if ((i & 0b11001111) == 0b00001001)
			result[i] = opcode::ADD_HL_SS;
		if ((i & 0b11000111) == 0b00000100)
			result[i] = (i == 0b00110100) ? opcode::INC_HL : opcode::INC_R;
		if ((i & 0b11001111) == 0b00000011)
			result[i] = opcode::INC_SS;
		if ((i & 0b11000111) == 0b00000101)
			result[i] = (i == 0b00110101) ? opcode::DEC_HL : opcode::DEC_R;
		if ((i & 0b11001111) == 0b00001011)
			result[i] = opcode::DEC_SS;
		if ((i & 0b11111000) == 0b10000000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::ADD_HL : opcode::ADD_R;
		if ((i & 0b11111000) == 0b10001000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::ADC_HL : opcode::ADC_R;
		if ((i & 0b11111000) == 0b10010000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::SUB_HL : opcode::SUB_R;
		if ((i & 0b11111000) == 0b10011000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::SBC_HL : opcode::SBC_R;
		if ((i & 0b11111000) == 0b10100000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::AND_HL : opcode::AND_R;
		if ((i & 0b11111000) == 0b10101000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::XOR_HL : opcode::XOR_R;
		if ((i & 0b11111000) == 0b10110000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::OR_HL : opcode::OR_R;
		if ((i & 0b11111000) == 0b10111000)
			result[i] = ((i & 0x07) == 0x06) ? opcode::CP_HL : opcode::CP_R;
		if ((i & 0b11000111) == 0b11000010)
			result[i] = opcode::JP_CC;
		if ((i & 0b11100111) == 0b00100000)
			result[i] = opcode::JR_C;
		if ((i & 0b11001111) == 0b11000101)
			result[i] = opcode::PUSH;
		if ((i & 0b11001111) == 0b11000001)
			result[i] = opcode::POP;
				switch (i) {
				case 0x00:
					result[i] = opcode::NOP;
					break;
				case 0x02:
					result[i] = opcode::LD_BC_A;
					break;
				case 0x07:
					result[i] = opcode::RLCA;
					break;
									case 0x08:
										result[i] = opcode::EX_AF;
										break;
									case 0x0A:
										result[i] = opcode::LD_A_BC;
										break;
									case 0x0F:
										result[i] = opcode::RRCA;
										break;
										//		case 0x10:
										//			result[i] = opcode::DJNZ;
										//			break;
							case 0x12:
								result[i] = opcode::LD_DE_A;
								break;
							case 0x17:
								result[i] = opcode::RLA;
								break;
										case 0x18:
											result[i] = opcode::JR;
											break;
										case 0x1A:
											result[i] = opcode::LD_A_DE;
											break;
										case 0x1F:
											result[i] = opcode::RRA;
											break;
										case 0x22:
											result[i] = opcode::LD_NNNN_HL;
											break;
										case 0x27:
											result[i] = opcode::DAA;
											break;
										case 0x2A:
											result[i] = opcode::LD_HL_NNNN;
											break;
										case 0x2F:
											result[i] = opcode::CPL;
											break;
										case 0x32:
											result[i] = opcode::LD_NNNN_A;
											break;
										case 0x37:
											result[i] = opcode::SCF;
											break;
										case 0x3A:
											result[i] = opcode::LD_A_NNNN;
											break;
										case 0x3F:
											result[i] = opcode::CCF;
											break;
													case 0xC3:
														result[i] = opcode::JP;
														break;
													case 0xC6:
														result[i] = opcode::ADD_N;
														break;
													case 0xCB:
														result[i] = opcode::DECODE_CB;
														break;
													case 0xCE:
														result[i] = opcode::ADC_N;
														break;
													case 0xD6:
														result[i] = opcode::SUB_N;
														break;
																case 0xD9:
																	result[i] = opcode::EXX;
																	break;
																case 0xDD:
																	result[i] = opcode::DECODE_DD;
																	break;
																case 0xDE:
																	result[i] = opcode::SBC_N;
																	break;
																			case 0xE3:
																				result[i] = opcode::EX_SP;
																				break;
																			case 0xE6:
																				result[i] = opcode::AND_N;
																				break;
																				//		case 0xE9:
																				//			result[i] = opcode::JP_HL;
																				//			break;
																						case 0xEB:
																							result[i] = opcode::EX_DE;
																							break;
																						case 0xED:
																							result[i] = opcode::DECODE_ED;
																							break;
																						case 0xEE:
																							result[i] = opcode::XOR_N;
																							break;
																						case 0xF3:
																							result[i] = opcode::DI;
																							break;
																						case 0xF6:
																							result[i] = opcode::OR_N;
																							break;
																							//		case 0xF9:
																							//			result[i] = opcode::LD_SP_HL;
																							//			break;
																									case 0xFB:
																										result[i] = opcode::EI;
																										break;
																									case 0xFD:
																										result[i] = opcode::DECODE_FD;
																										break;
																									case 0xFE:
																										result[i] = opcode::CP_N;
																										break;
																									}
																									}
																									return result;
																									}()
};


using namespace ae::cpu;

/*********************************************************************************************************************/
uint16_t Z80::decode_opcode(const uint8_t opcode,
							const prefix p) {
	uint16_t cycle = 0;
	uint16_t tmp16;

	if (opcodes[opcode] != opcode::UNIMPLEMENTED) {
		cycle += 4;
		switch (opcodes[opcode]) {
		case opcode::NOP:
			break;
		case opcode::HALT:
			halted = true;
			break;
		case opcode::DI:
			iff1 = false;
			iff2 = false;
			break;
		case opcode::EI:
			iff1 = true;
			iff2 = true;
			iff1_waiting = true;
			break;
		case opcode::DECODE_CB:
			decode_opcode_cb(p);
			break;
		case opcode::DECODE_DD:
			if (p != NO)
				unimplemented();
			else {
				uint8_t opcode2 = readOpcode();
				if (((opcode2 & 0b11001111) == 0b00001001) ||
					((opcode2 & 0b11111000) == 0b10000000) ||
					((opcode2 & 0b11111000) == 0b10001000) ||
					((opcode2 & 0b11111000) == 0b10010000) ||
					((opcode2 & 0b11111000) == 0b10011000) ||
					((opcode2 & 0b11111000) == 0b10100000) ||
					((opcode2 & 0b11111000) == 0b10101000) ||
					((opcode2 & 0b11111000) == 0b10110000) ||
					((opcode2 & 0b11111000) == 0b10111000) ||
					((opcode2 & 0b11001111) == 0b00000011) ||
					((opcode2 & 0b11001111) == 0b00000001) ||
					((opcode2 & 0b11001111) == 0b00001011) ||
					((opcode2 & 0b11000111) == 0b00000101) ||
					((opcode2 & 0b11000111) == 0b00000100) ||
					((opcode2 & 0b11000111) == 0b01000110) ||
					((opcode2 & 0b11000111) == 0b00000110) ||
					((opcode2 & 0b11000000) == 0b01000000) ||
					(opcode2 == 0x22) ||
					(opcode2 == 0x2A) ||
					(opcode2 == 0xcb) //||
					//					(opcode2 == 0xf9)
					) {
					cycle += decode_opcode(opcode2, DD);
				}
				else {
					cycle += decode_dd_opcode(opcode2);
				}
			}
			break;
		case opcode::DECODE_ED:
			decode_opcode_ed();
			break;
		case opcode::DECODE_FD:
			if (p != NO)
				unimplemented();
			else {
				uint8_t opcode2 = readOpcode();
				if (((opcode2 & 0b11001111) == 0b00001001) ||
					((opcode2 & 0b11111000) == 0b10000000) ||
					((opcode2 & 0b11111000) == 0b10001000) ||
					((opcode2 & 0b11111000) == 0b10010000) ||
					((opcode2 & 0b11111000) == 0b10011000) ||
					((opcode2 & 0b11111000) == 0b10100000) ||
					((opcode2 & 0b11111000) == 0b10101000) ||
					((opcode2 & 0b11111000) == 0b10110000) ||
					((opcode2 & 0b11111000) == 0b10111000) ||
					((opcode2 & 0b11001111) == 0b00000011) ||
					((opcode2 & 0b11001111) == 0b00000001) ||
					((opcode2 & 0b11001111) == 0b00001011) ||
					((opcode2 & 0b11000111) == 0b00000101) ||
					((opcode2 & 0b11000111) == 0b00000100) ||
					((opcode2 & 0b11000111) == 0b01000110) ||
					((opcode2 & 0b11000111) == 0b00000110) ||
					((opcode2 & 0b11000000) == 0b01000000) ||
					(opcode2 == 0x22) ||
					(opcode2 == 0x2A) ||
					(opcode2 == 0xcb)
					) {
					cycle += decode_opcode(opcode2, FD);
				}
				else {
					cycle += decode_fd_opcode(opcode2);
				}
			}
			break;
		case opcode::LD_R_R:
			decode8(opcode >> 3, p) = decode8(opcode, p);
			break;
		case opcode::LD_HL_R:
			apply_hl([this, opcode](const uint8_t) { return decode8(opcode); }, p, (p != NO) ? readArgument8() : 0);
			if (p == NO)
				_elapsed_cycles -= 4;
			break;
		case opcode::LD_SP_HL:
			if (p == NO)
				_state.sp() = _state.hl();
			else if (p == DD)
				_state.sp() = _state.ix();
			else
				_state.sp() = _state.iy();
			_elapsed_cycles += 2;
			break;
		case opcode::LD_R_N:
			decode8(opcode >> 3, p) = readArgument8();
			_elapsed_cycles--;
			break;
		case opcode::LD_R_HL:
			if (p == NO) {
				decode8(opcode >> 3) = read8(_state.hl());
				_elapsed_cycles--;
			}
			else if (p == DD) {
				decode8(opcode >> 3) = read8(_state.ix() + static_cast<int8_t>(readArgument8()));
				_elapsed_cycles += 3;
			}
			else if (p == FD) {
				decode8(opcode >> 3) = read8(_state.iy() + static_cast<int8_t>(readArgument8()));
				_elapsed_cycles += 3;
			}
			break;
		case opcode::LD_HL_N:
		{
			int8_t delta = (p != NO) ? readArgument8() : 0;
			apply_hl([this](const uint8_t) { return readArgument8(); }, p, delta);
			_elapsed_cycles -= 4;
			if (p == NO)
				_elapsed_cycles--;
		}
		break;
		case opcode::LD_A_BC:
			_state.a() = read8(_state.bc());
			_elapsed_cycles--;
			break;
		case opcode::LD_A_DE:
			_state.a() = read8(_state.de());
			_elapsed_cycles--;
			break;
		case opcode::LD_BC_A:
			write8(_state.bc(), _state.a());
			break;
		case opcode::LD_DE_A:
			write8(_state.de(), _state.a());
			break;
		case opcode::LD_SS_NN:
			decode16(opcode, p) = readArgument16();
			break;
		case opcode::LD_A_NNNN:
			_state.a() = read8(readArgument16());
			_elapsed_cycles--;
			break;
		case opcode::LD_NNNN_A:
			write8(readArgument16(), _state.a());
			break;
		case opcode::LD_HL_NNNN:
			tmp16 = readArgument16();
			if (p == NO)
				_state.hl() = read16(tmp16);
			else if (p == DD)
				_state.ix() = read16(tmp16);
			else if (p == FD)
				_state.iy() = read16(tmp16);
			break;

		case opcode::LD_NNNN_HL:
			write16(readArgument16(), decode16(opcode, p));
			break;

		case opcode::ADD_HL_SS:
			add_ss(opcode, p);
			break;
		case opcode::ADD_N:
			add(readArgument8());
			_elapsed_cycles--;
			break;
		case opcode::ADD_R:
			add(decode8(opcode, p));
			break;
		case opcode::ADD_HL:
			if (p == DD) {
				add(read8(_state.ix() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else if (p == FD) {
				add(read8(_state.iy() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else {
				add(read8(_state.hl()));
				_elapsed_cycles--;
			}
			break;
		case opcode::ADC_N:
			add(readArgument8(), (_state.carryFlag()) ? 1 : 0);
			_elapsed_cycles--;
			break;
		case opcode::ADC_R:
			add(decode8(opcode, p), (_state.carryFlag()) ? 1 : 0);
			break;
		case opcode::ADC_HL:
			if (p == DD) {
				add(read8(_state.ix() + static_cast<signed char>(readArgument8())), (_state.carryFlag()) ? 1 : 0);
				_elapsed_cycles += 3;
			}
			else if (p == FD) {
				add(read8(_state.iy() + static_cast<signed char>(readArgument8())), (_state.carryFlag()) ? 1 : 0);
				_elapsed_cycles += 3;
			}
			else {
				add(read8(_state.hl()), (_state.carryFlag()) ? 1 : 0);
				_elapsed_cycles--;
			}
			break;
		case opcode::SUB_N:
			sub(readArgument8());
			_elapsed_cycles--;
			break;
		case opcode::SUB_R:
			sub(decode8(opcode, p));
			break;
		case opcode::SUB_HL:
			if (p == DD) {
				sub(read8(_state.ix() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else if (p == FD) {
				sub(read8(_state.iy() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else {
				sub(read8(_state.hl()));
				_elapsed_cycles--;
			}
			break;
		case opcode::SBC_N:
			sub(readArgument8(), (_state.carryFlag()) ? 1 : 0);
			_elapsed_cycles--;
			break;
		case opcode::SBC_R:
			sub(decode8(opcode, p), (_state.carryFlag()) ? 1 : 0);
			break;
		case opcode::SBC_HL:
			if (p == DD) {
				sub(read8(_state.ix() + static_cast<signed char>(readArgument8())), (_state.carryFlag()) ? 1 : 0);
				_elapsed_cycles += 3;
			}
			else if (p == FD) {
				sub(read8(_state.iy() + static_cast<signed char>(readArgument8())), (_state.carryFlag()) ? 1 : 0);
				_elapsed_cycles += 3;
			}
			else {
				sub(read8(_state.hl()), (_state.carryFlag()) ? 1 : 0);
				_elapsed_cycles--;
			}
			break;
		case opcode::AND_N:
			ana(readArgument8());
			_elapsed_cycles--;
			break;
		case opcode::AND_R:
			ana(decode8(opcode, p));
			break;
		case opcode::AND_HL:
			if (p == DD) {
				ana(read8(_state.ix() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else if (p == FD) {
				ana(read8(_state.iy() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else {
				ana(read8(_state.hl()));
				_elapsed_cycles--;
			}
			break;
		case opcode::XOR_N:
			xra(readArgument8());
			_elapsed_cycles--;
			break;
		case opcode::XOR_R:
			xra(decode8(opcode, p));
			break;
		case opcode::XOR_HL:
			if (p == DD) {
				xra(read8(_state.ix() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else if (p == FD) {
				xra(read8(_state.iy() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else {
				xra(read8(_state.hl()));
				_elapsed_cycles--;
			}
			break;
		case opcode::OR_N:
			ora(readArgument8());
			_elapsed_cycles--;
			break;
		case opcode::OR_R:
			ora(decode8(opcode, p));
			break;
		case opcode::OR_HL:
			if (p == DD) {
				ora(read8(_state.ix() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else if (p == FD) {
				ora(read8(_state.iy() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else {
				ora(read8(_state.hl()));
				_elapsed_cycles--;
			}
			break;
		case opcode::CP_N:
			cp(readArgument8());
			_elapsed_cycles--;
			break;
		case opcode::CP_R:
			cp(decode8(opcode, p));
			break;
		case opcode::CP_HL:
			if (p == DD) {
				cp(read8(_state.ix() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else if (p == 2) {
				cp(read8(_state.iy() + static_cast<signed char>(readArgument8())));
				_elapsed_cycles += 3;
			}
			else {
				cp(read8(_state.hl()));
				_elapsed_cycles--;
			}
			break;
		case opcode::DAA:
			daa();
			break;
		case opcode::CPL:
			_state.a() = ~_state.a();
			_state.setFlags(Z80State::HF | Z80State::NF);
			break;
		case opcode::SCF:
			_state.setFlags(Z80State::CF);
			_state.resetFlags(Z80State::NF | Z80State::XF | Z80State::YF | Z80State::HF);
			_state.f() |= _state.a() & (Z80State::XF | Z80State::YF);
			break;
		case opcode::CCF:
			if (_state.carryFlag()) {
				_state.resetFlags(Z80State::NF | Z80State::CF | Z80State::XF | Z80State::YF);
				_state.setFlags(Z80State::HF);
			}
			else {
				_state.resetFlags(Z80State::NF | Z80State::HF | Z80State::XF | Z80State::YF);
				_state.setFlags(Z80State::CF);
			}
			_state.f() |= _state.a() & (Z80State::XF | Z80State::YF);
			break;
		case opcode::RLCA:
			rla(true);
			break;
		case opcode::RRCA:
			rra(true);
			break;
		case opcode::RLA:
			rla(false);
			break;
		case opcode::RRA:
			rra(false);
			break;
		case opcode::INC_R:
			apply_r([this](const uint8_t r) { return inc(r); }, opcode >> 3, p);
			break;
		case opcode::INC_HL:
			apply_hl([this](const uint8_t r) { return inc(r); }, p, (p == NO) ? 0 : static_cast<int8_t>(readArgument8()));
			if (p != NO)
				_elapsed_cycles += 4;
			break;
		case opcode::DEC_R:
			apply_r([this](const uint8_t r) { return dec(r); }, opcode >> 3, p);
			break;
		case opcode::DEC_HL:
			apply_hl([this](const uint8_t r) { return dec(r); }, p, (p == NO) ? 0 : static_cast<int8_t>(readArgument8()));
			if (p != NO)
				_elapsed_cycles += 4;
			break;
		case opcode::INC_SS:
			++decode16(opcode, p);
			_elapsed_cycles += 2;
			break;
		case opcode::DEC_SS:
			--decode16(opcode, p);
			_elapsed_cycles += 2;
			break;
		case opcode::EX_AF:
			_state.exchange(Z80State::AF);
			break;
		case opcode::EXX:
			_state.exchange(Z80State::BC);
			_state.exchange(Z80State::DE);
			_state.exchange(Z80State::HL);
			break;
		case opcode::EX_DE:
			exchange_de_hl();
			break;
		case opcode::EX_SP:
			if (p == NO)
				_state.hl() = exchange_sp(_state.hl());
			else if (p == DD)
				_state.ix() = exchange_sp(_state.ix());
			else
				_state.iy() = exchange_sp(_state.iy());
			_elapsed_cycles += 3;
			break;
		case opcode::JP:
			pc = readArgument16();
			break;
		case opcode::JP_CC:
			tmp16 = readArgument16();
			if (checkCondition3(opcode)) {
				pc = tmp16;
			}
			break;
		case opcode::JP_HL:
			pc = decode16(0x20, p);
			break;
		case opcode::JR:
			pc = pc + static_cast<signed char>(readArgument8());
			_elapsed_cycles += 4;
			break;
		case opcode::JR_C:
			if (checkCondition2(opcode)) {
				pc = pc + static_cast<signed char>(readArgument8());
				_elapsed_cycles += 4;
			}
			else {
				readArgument8();
				_elapsed_cycles--;
			}
			break;
		case opcode::DJNZ:
			if (--_state.b() == 0) {
				readArgument8();
			}
			else {
				pc = pc + static_cast<signed char>(readArgument8());
				_elapsed_cycles += 5;
			}
			break;
		case opcode::PUSH:
			pushToStack(decode16(opcode, p, true));
			break;
		case opcode::POP:
			decode16(opcode, p, true) = popOfStack();
			break;
		default:
			unimplemented();
			break;
		}
		return cycle;
	}

	uint8_t tmp8 = 0;

	uint8_t opcode1 = opcode >> 6;
	uint8_t opcode2 = (opcode >> 3) & 0x07;
	uint8_t opcode3 = opcode & 0x07;

	switch (opcode1)
	{
	case 0b11:
		switch (opcode3) {
		case 0b000: /* RET cc */
			cycle += 5;
			if (checkCondition3(opcode)) {
				pc = popOfStack();
				cycle += 6;
			}
			break;
		case 0b010: /* JP cc, nn */
			tmp16 = readArgument16();
			cycle += 10;
			if (checkCondition3(opcode)) {
				pc = tmp16;
			}
			break;
		case 0b100: /* CALL cc, nn */
			tmp16 = readArgument16();
			cycle += 10;
			if (checkCondition3(opcode)) {
				pushToStack(pc);
				pc = tmp16;
				cycle += 7;
			}
			break;
		default:
			break;
		}
		break;
	}
	if (cycle == 0) {
		switch (opcode) {
		case 0x08: /* EX AF, AF' */
			_state.exchange(Z80State::AF);
			cycle = 4;
			break;
		case 0x10: /* DJNZ e */
		{
			uint8_t e = readArgument8();
			if (--_state.b() == 0) {
				cycle = 8;
			}
			else {
				pc = pc + static_cast<signed char>(e);
				cycle = 13;
			}}
		break;
		case 0x18: /* JR, e */
			tmp8 = readArgument8();
			pc = pc + static_cast<signed char>(tmp8);
			cycle = 12;
			break;
		case 0x20: /* JR NZ, e */
		{
			uint8_t e = readArgument8();
			if (_state.f() & flags::zeroFlag) {
				cycle = 7;
			}
			else {
				pc = pc + static_cast<signed char>(e);
				cycle = 12;
			}
			break;
		}
		case 0x28: /* JR z, e */
		{
			uint8_t e = readArgument8();
			if (!(_state.f() & flags::zeroFlag)) {
				cycle = 7;
			}
			else {
				pc = pc + static_cast<signed char>(e);
				cycle = 12;
			}
			break;
		}
		case 0x30: /* JR NC, e */
		{
			uint8_t e = readArgument8();
			if (_state.f() & flags::carryFlag) {
				cycle = 7;
			}
			else {
				pc = pc + static_cast<signed char>(e);
				cycle = 12;
			}
			break;
		}
		case 0x38: /* JR c, e */
		{
			uint8_t e = readArgument8();
			if (!(_state.f() & flags::carryFlag)) {
				cycle = 7;
			}
			else {
				pc = pc + static_cast<signed char>(e);
				cycle = 12;
			}
			break;
		}
		case 0xC1: /* POP BE */
			_state.bc() = popOfStack();
			cycle = 10;
			break;
		case 0xC3: /* JMP */
			pc = readArgument16();
			cycle = 10;
			break;
		case 0xC5: /* PUSH BC */
			pushToStack(_state.bc());
			cycle = 11;
			break;
		case 0xC9: /* RET */
			pc = popOfStack();
			cycle = 10;
			break;
		case 0xCD: /* CALL */
			tmp16 = readArgument16();
			pushToStack(pc);
			pc = tmp16;
			cycle = 17;
			break;
		case 0xCF: /* RST 08 */
			pushToStack(pc);
			pc = 0x08;
			cycle = 11;
			break;
		case 0xD1: /* POP DE */
			_state.de() = popOfStack();
			cycle = 10;
			break;
		case 0xD3: /* OUT (N), A */
			_handlerOut(readArgument8(), _state.a());
			cycle = 11;
			break;
		case 0xD5: /* PUSH DE */
			pushToStack(_state.de());
			cycle = 11;
			break;
		case 0xD7: /* RST 10 */
			pushToStack(pc);
			pc = 0x10;
			cycle = 11;
			break;
		case 0xD9: /* EXX */
			_state.exchange(Z80State::BC);
			_state.exchange(Z80State::DE);
			_state.exchange(Z80State::HL);
			cycle = 4;
			break;
		case 0xDF: /* RST 18 */
			pushToStack(pc);
			pc = 0x18;
			cycle = 11;
			break;
		case 0xE1: /* POP HL */
			_state.hl() = popOfStack();
			cycle = 10;
			break;
		case 0xE5: /* PUSH HL */
			pushToStack(_state.hl());
			cycle = 11;
			break;

		case 0xE7: /* RST 20 */
			pushToStack(pc);
			pc = 0x20;
			cycle = 11;
			break;
		case 0xE9: /* JP (HL) */
			pc = _state.hl();
			cycle = 4;
			break;
		case 0XEB: /* EX DE, HL */
			tmp8 = _state.d();
			_state.d() = _state.h();
			_state.h() = tmp8;
			tmp8 = _state.e();
			_state.e() = _state.l();
			_state.l() = tmp8;
			cycle = 4;
			break;
		case 0xEF: /* RST 28 */
			pushToStack(pc);
			pc = 0x28;
			cycle = 11;
			break;
		case 0xF1: /* POP AF */
			tmp16 = popOfStack();
			_state.a() = tmp16 >> 8;
			_state.f() = tmp16 & 0xFF;
			cycle = 10;
			break;
		case 0xF5: /* PUSH AF */
			pushToStack(_state.af());
			cycle = 11;
			break;
		case 0xF7: /* RST 30 */
			pushToStack(pc);
			pc = 0x30;
			cycle = 11;
			break;
		case 0xF9: /* LD SP, HL */
			_state.sp() = _state.hl();
			cycle = 6;
			break;
		default: unimplemented(); break;
		}
	}
	return cycle;
}

/**********************************************************************************************************************/
void Z80::add_ss(const opcode_t opcode, const prefix p) {
	const uint16_t a = decode16(0x20, p);
	const uint16_t b = decode16(opcode, p);
	const uint16_t s = a + b;

	decode16(0x20, p) = s;
	_state.resetFlags(Z80State::NF | Z80State::CF | Z80State::HF);
	if (a > 0xffff - b)
		_state.setFlags(Z80State::CF);
	if (((s ^ a ^ b) >> 11) & 0x1)
		_state.setFlags(Z80State::HF);
	_elapsed_cycles += 7;
}
void Z80::rla(const bool c) {
	const bool carry = (c) ? _state.a() & 0x80 : _state.carryFlag();

	_state.resetFlags(Z80State::NF | Z80State::HF | Z80State::CF);
	if (_state.a() & 0x80)
		_state.setFlags(Z80State::CF);
	_state.a() <<= 1;
	_state.a() |= (carry) ? 1 : 0;
}
void Z80::rra(const bool c) {
	const bool carry = (c) ? _state.a() & 0x01 : _state.carryFlag();

	_state.resetFlags(Z80State::NF | Z80State::HF | Z80State::CF);
	if (_state.a() & 0x01)
		_state.setFlags(Z80State::CF);
	_state.a() >>= 1;
	_state.a() |= (carry) ? 0x80 : 0;
}
void Z80::daa() {
	uint8_t carry = 0;
	uint8_t diff = 0;
	if ((_state.carryFlag()) || (_state.a() > 0x99)) {
		diff += 0x60;
		carry = Z80State::CF;
	}
	if ((_state.halfCarryFlag()) || ((_state.a() & 0xf) > 9))
		diff += 6;

	if (_state.addSubFlag()) {
		if ((_state.a() & 0xf) > 5)
			_state.resetFlags(Z80State::HF);
		_state.a() -= diff;
	}
	else {
		if ((_state.a() & 0xf) > 9)
			_state.setFlags(Z80State::HF);
		else
			_state.resetFlags(Z80State::HF);
		_state.a() += diff;
	}
	_state.setSZXY(_state.a());
	_state.setP(_state.a());
	_state.f() |= carry;
}
void Z80::add(const uint8_t value, const uint8_t flag)
{
	const uint16_t sum = _state.a() + value + flag;
	const uint16_t carryIns = sum ^ _state.a() ^ value;

	_state.a() = sum & 0xff;

	_state.resetFlags(Z80State::NF | Z80State::HF | Z80State::PF | Z80State::CF);
	_state.setSZXY(_state.a());
	if (sum > 0xff)
		_state.setFlags(Z80State::CF);
	if (carryIns & 0x10)
		_state.setFlags(Z80State::HF);
	if (((carryIns >> 7) & 0x1) ^ ((carryIns >> 8) & 0x1))
		_state.setFlags(Z80State::PF);
}
void Z80::sub(const uint8_t value, const uint8_t flag)
{
	const uint16_t sum = _state.a() - value - flag;
	const uint16_t carryIns = (sum ^ _state.a() ^ value);

	_state.a() = sum & 0xff;

	_state.resetFlags(Z80State::HF | Z80State::PF | Z80State::CF);
	_state.setFlags(Z80State::NF);
	_state.setSZXY(_state.a());
	if (carryIns & 0x10)
		_state.setFlags(Z80State::HF);

	if (((carryIns >> 7) & 0x1) ^ ((carryIns >> 8) & 0x1))
		_state.setFlags(Z80State::PF);
	if ((carryIns >> 8) & 0x1)
		_state.setFlags(Z80State::CF);
}
void Z80::cp(const uint8_t value)
{
	const uint16_t sum = _state.a() - value;
	const uint16_t carryIns = (sum ^ _state.a() ^ value);

	_state.f() = Z80State::NF;
	if (sum == 0)
		_state.setFlags(Z80State::ZF);
	else
		_state.setFlags(sum & Z80State::SF);
	_state.setFlags(value & (Z80State::XF | Z80State::YF));

	if (carryIns & 0x10)
		_state.setFlags(Z80State::HF);
	if (((carryIns >> 7) & 0x1) ^ ((carryIns >> 8) & 0x1))
		_state.setFlags(Z80State::PF);
	if ((carryIns >> 8) & 0x1)
		_state.setFlags(Z80State::CF);
}
void Z80::ana(const uint8_t value) {
	_state.a() &= value;

	_state.f() = 0;
	_state.setSZXY(_state.a());
	_state.setP(_state.a());
	_state.setFlags(Z80State::HF);
}
void Z80::ora(const uint8_t value) {
	_state.a() |= value;

	_state.f() = 0;
	_state.setSZXY(_state.a());
	_state.setP(_state.a());
}
void Z80::xra(const uint8_t value) {
	_state.a() ^= value;

	_state.f() = 0;
	_state.setSZXY(_state.a());
	_state.setP(_state.a());
}

uint8_t Z80::inc(const uint8_t value) {
	uint8_t result = value + 1;
	_state.resetFlags(Z80State::NF | Z80State::HF | Z80State::VF);
	_state.setSZXY(result);
	if (result == 0x80)
		_state.setFlags(Z80State::VF);
	if ((result & 0x0f) == 0x00)
		_state.setFlags(Z80State::HF);
	return result;
}
uint8_t Z80::dec(const uint8_t value) {
	uint8_t result = value - 1;
	_state.resetFlags(Z80State::HF | Z80State::VF);
	_state.setFlags(Z80State::NF);
	_state.setSZXY(result);
	if (result == 0x7f)
		_state.setFlags(Z80State::VF);
	if ((result & 0x0f) == 0x0f)
		_state.setFlags(Z80State::HF);
	return result;
}

void Z80::exchange_de_hl() {
	const uint16_t t = _state.de();
	_state.de() = _state.hl();
	_state.hl() = t;
}
uint16_t Z80::exchange_sp(const uint16_t hl) {
	const uint16_t sp = read16(_state.sp());
	write16(_state.sp(), hl);
	return sp;
}