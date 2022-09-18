#pragma once

#include "types.h"
#include <functional>

#include "memory.h"


namespace ae
{
	class Cpu {
	public:
		typedef std::function<void(const uint8_t, const uint8_t)> outfn;
		typedef std::function<const uint8_t(const uint8_t)> infn;
		typedef std::function<void(const uint16_t, const uint8_t)> writefn;
		typedef std::function<const uint8_t(const uint16_t)> readfn;

		typedef uint8_t opcode_t;

	protected:
		uint64_t _elapsed_cycles;

		outfn _handlerOut;
		infn _handlerIn;
		readfn _handlerRead;
		writefn _handlerWrite;

		Cpu();

	public:
		const uint64_t elapsed_cycles() const { return _elapsed_cycles; }

		virtual bool reset(const uint16_t = 0) = 0;
		virtual const string disassemble() = 0;
		virtual const uint8_t executeOne() = 0;
		virtual bool interrupt(const uint8_t) = 0;
		bool in(const infn);
		bool out(const outfn);
		bool read(const readfn);
		bool write(const writefn);

	public:
		static Cpu* create(const string&);

		static inline constexpr bool sign(const uint8_t v) { return (v & 0x80) != 0; }
		static inline constexpr bool zero(const uint8_t v) { return v == 0; }
		static inline constexpr bool parity(const uint8_t v) {
			uint8_t t = v;
			t ^= t >> 4;
			t ^= t >> 2;
			return ((t ^= t >> 1) & 1) ? false : true;
		}
	};
}