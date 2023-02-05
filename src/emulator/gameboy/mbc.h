#pragma once
#include "types.h"


namespace ae {
	namespace gameboy {
		class BootRom {
		protected:
			uint8_t _rom[0x100];

		public:
			BootRom(const string&);
			uint8_t read(const uint16_t address) const { return (address < 0x100) ? _rom[address] : 0; }
		};

		class Mbc {
		public:
			struct header {
				string title;
			};

		protected:
			uint8_t* _rom;
			uint16_t _rom_size;
			uint8_t* _ram;
			uint16_t _ram_size;
			header _header;

			void decode_header();
		public:
			Mbc(const uint16_t, const uint16_t);
			~Mbc();

			uint8_t read(const uint16_t) const;
			bool write(const uint16_t, const uint8_t);

			const header& get_header() const { return _header; }

			static Mbc* create(const string&);
		};
	}
}