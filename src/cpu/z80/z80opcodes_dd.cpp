#include "../z80.h"
#include "types.h"
#include <array>


using namespace ae::cpu;


/*********************************************************************************************************************/
// decode
uint16_t Z80::decode_dd_opcode(const uint8_t opcode) {
	uint16_t cycle = 0;

	uint16_t tmp16;
	uint8_t tmp8;

	switch (opcode) {
	case 0x77: /* LD (IX+d), A */
		_handlerWrite(_state.ix() + static_cast<signed char>(readArgument8()), _state.a());
		cycle = 19;
		break;
	case 0x86: /* ADD A, (IX+d) */
		tmp8 = readArgument8();
		add(_handlerRead(_state.ix() + static_cast<signed char>(tmp8)));
		cycle = 19;
		break;
	case 0xE1: /* POP IX */
		_state.ix() = popOfStack();
		cycle = 14;
		break;
	default: unimplemented(); break;
	}
	return cycle;
}
