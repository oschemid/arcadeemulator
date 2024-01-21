#pragma once
#include "emulator.h"

#include <forward_list>


namespace aos::namco
{
	class Mmu
	{
	public:
		class mapping
		{
		public:
			mapping(const uint16_t start, const uint16_t end) :
				_start(start), _end(end)
			{}
			~mapping()
			{
				if (_memory)
					delete[] _memory;
			}
			mapping& rom();
			mapping& ram();
			mapping& mirror(const uint16_t mirror) { _mirroring = mirror; return *this; }
			mapping& name(const string name) { _name = name; return *this; }
			mapping& bank(const uint8_t bank) { _bank = bank; return *this; }
			mapping& readfn(std::function<uint8_t(const uint16_t)>);
			mapping& writefn(std::function<void(const uint16_t, const uint8_t)>);
			mapping& decodefn(std::function<void(uint8_t*, const size_t)>);

			void init(const vector<aos::mmu::RomMapping>&);
			bool is_mapped(const uint16_t address, const uint8_t bank=0) const
			{
				if ((_bank > 0) && (bank != _bank))
					return false;
				const uint16_t address_mirror = address & _mirroring;
				return ((address_mirror >= _start) && (address_mirror <= _end)) ? true : false;
			}
			bool is_readable() const { return _readfn != nullptr; }
			bool is_writable() const { return _writefn != nullptr; }

			uint8_t read(const uint16_t address) { if (_readfn) { const uint16_t address_mirror = address & _mirroring; return _readfn(address_mirror - _start); } }
			void write(const uint16_t address, const uint8_t value) { if (_writefn) { const uint16_t address_mirror = address & _mirroring; _writefn(address_mirror - _start, value); } }

			void patch(const uint16_t address, const uint8_t value) { if (_memory) _memory[address-_start] = value; }
		protected:
			string _name;
			uint16_t _start;
			uint16_t _end;
			uint16_t _mirroring{ 0xffff };
			uint8_t _bank{ 0 };
			bool _internalmemory{ true };
			uint8_t* _memory{ nullptr };
			std::function<void(uint8_t*, const size_t)> _decodingfn;
			std::function<uint8_t(const uint16_t)> _readfn;
			std::function<void(const uint16_t, const uint8_t)> _writefn;
		};
		Mmu()
		{
		};

		mapping& map(const uint16_t, const uint16_t);
		void init(const vector<aos::mmu::RomMapping>& roms);
		void before_fn(std::function<void(const uint16_t)> fn) { _beforefn = fn; }
		void selectBank(const uint8_t bank) { _bank_selected = bank; }
		uint8_t read(const uint16_t);
		void write(const uint16_t, const uint8_t);
		void patch(const uint16_t, const uint8_t);

	protected:
		std::forward_list<mapping> _mapping;
		uint8_t _bank_selected{ 0 };
		std::function<void(const uint16_t)> _beforefn;
	};
}