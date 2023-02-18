#pragma once
#include "types.h"
#include "memorymap.h"
#include "mbc.h"
#include <functional>
#include <vector>


namespace ae::gameboy {
	namespace io {
		const uint8_t div = 0x04;
		const uint8_t nr10 = 0x10;
		const uint8_t nr11 = 0x11;
		const uint8_t nr12 = 0x12;
		const uint8_t nr13 = 0x13;
		const uint8_t nr14 = 0x14;
		const uint8_t nr21 = 0x16;
		const uint8_t nr22 = 0x17;
		const uint8_t nr23 = 0x18;
		const uint8_t nr24 = 0x19;
		const uint8_t nr30 = 0x1a;
		const uint8_t nr31 = 0x1b;
		const uint8_t nr32 = 0x1c;
		const uint8_t nr33 = 0x1d;
		const uint8_t nr34 = 0x1e;
		const uint8_t nr41 = 0x20;
		const uint8_t nr42 = 0x21;
		const uint8_t nr43 = 0x22;
		const uint8_t nr44 = 0x23;
		const uint8_t nr50 = 0x24;
		const uint8_t nr52 = 0x26;
		const uint8_t stat = 0x41;
		const uint8_t ly = 0x44;
		const uint8_t ff50 = 0x50;
	}

	class Mmu {
	public:
		using read_fn = std::function<uint8_t(const uint16_t)>;
		using write_fn = std::function<void(const uint16_t, const uint8_t)>;

		enum class origin {
			cpu,
			ppu,
			apu
		};
		void registerIoCallback(std::function<void(const uint8_t, const uint8_t)> cb) {
			_io_callbacks.push_back(cb);
		}
		void map(MemoryMap, read_fn, write_fn);

	protected:
		std::vector< std::function<void(const uint8_t, const uint8_t)>> _io_callbacks;
		read_fn _handlerReadVRAM;
		write_fn _handlerWriteVRAM;
		read_fn _handlerReadOAM;
		write_fn _handlerWriteOAM;
		std::array<read_fn, 0x80> _handlerReadIO;
		std::array<write_fn, 0x80> _handlerWriteIO;

		void notify(const uint8_t, const uint8_t) const;
		std::shared_ptr<BootRom> _bootrom;
		std::shared_ptr<Mbc> _cartridge;
		uint8_t* _rams;
		uint16_t _div;

	public:
		Mmu(std::shared_ptr<BootRom>&, std::shared_ptr<Mbc>&);
		~Mmu();

		void tick();

		uint8_t read(const uint16_t, const origin) const;
		bool write(const uint16_t, const uint8_t, const origin);
		uint8_t in(const uint8_t, const origin) const;
		bool out(const uint8_t, const uint8_t, const origin);
	};
}