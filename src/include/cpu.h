#pragma once

#include "types.h"
#include <functional>

#include "memory.h"


namespace ae
{
	class ICpu {
	public:
		typedef std::function<void(const uint8_t, const uint8_t)> outfn;
		typedef std::function<const uint8_t(const uint8_t)> infn;
		typedef std::function<void(const uint16_t, const uint8_t)> writefn;
		typedef std::function<const uint8_t(const uint16_t)> readfn;

	public:
		virtual bool reset(const uint16_t = 0) = 0;
		virtual const string disassemble() = 0;
		virtual const uint8_t executeOne() = 0;
		virtual bool interrupt(const uint8_t) = 0;
		virtual bool in(const infn) = 0;
		virtual bool out(const outfn) = 0;
		virtual bool read(const readfn) = 0;
		virtual bool write(const writefn) = 0;

	public:
		static ICpu* create(const string&);
	};
}