#pragma once
#include "types.h"


namespace aos::namco
{
	std::function<void(uint8_t*, const size_t)> getDecoder(const string&);
	std::function<uint8_t(const uint8_t)> getInterruptDecoder(const string&);
}
