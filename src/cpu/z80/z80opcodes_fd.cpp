#include "../z80.h"
#include "types.h"
#include <array>


using namespace ae::cpu;


/*********************************************************************************************************************/
// decode
uint16_t Z80::decode_fd_opcode(const uint8_t opcode) {
	uint16_t cycle = 0;

	uint16_t tmp16;
	uint8_t tmp8;

	switch (opcode) {
	case 0x77: /* LD (IY+d), A */
		_handlerWrite(_state.iy() + static_cast<signed char>(readArgument8()), _state.a());
		cycle = 19;
		break;
	case 0xE1: /* POP IY */
		_state.iy() = popOfStack();
		cycle = 14;
		break;
	case 0xE5: /* PUSH IY */
		pushToStack(_state.iy());
		cycle = 11;
		break;
	case 0xE9: /* JP (IY) */
		pc = _state.iy();
		cycle = 8;
		break;
	default: unimplemented(); break;
	}
	return cycle;
}
