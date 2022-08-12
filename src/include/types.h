#pragma once

#include <string>
#include <cstdint>

namespace ae {
	using std::string;
	using std::uint8_t;
	using std::uint16_t;
	using std::size_t;

	struct rgb_t {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};
	struct rect_t {
		uint16_t left;
		uint16_t top;
		uint16_t right;
		uint16_t bottom;
	};
};