#pragma once
#include "types.h"
#include "tools.h"


using aos::uint8_t;
using aos::string;
using aos::vector;
using aos::map;


namespace aos::mmu
{
	// This structure defines a mapping between a ROM file and a memory bank
	// Parameters:
	//   - region, name of the memory bank
	//   - rom, file containing ROM dump
	//   - mapping, optional parameters for mapping
	//       - start, offset for reading file
	//       - size, size of the reading file
	struct RomMapping
	{
		string region;
		aos::tools::Rom rom;
		struct {
			uint16_t start;
			uint16_t size;
		} mapping;
	};
	using RomMappings = vector<RomMapping>;

	// This class defines a memory bank
	// Parameters:
	//    - size, size of the memory bank
	//    - readfn, read access function
	//    - writefn, write access function
	//    - memory, if managed memory bank, memory block
	class MemoryBank
	{
	public:
		MemoryBank(const uint16_t size = 0) :
			_size(size) {}
		~MemoryBank();

		// Configuration
		MemoryBank& readfn(readfn_t fn) { _readfn = fn; return *this; }
		MemoryBank& writefn(writefn_t fn) { _writefn = fn; return *this; }
		MemoryBank& managed(const bool m) { _managed = m; return *this; }

		// Helpers for standard memory bank
		MemoryBank& rom();
		MemoryBank& ram();

		// Loading
		void init(const RomMappings&);

		// Access functions
		uint8_t read(const uint16_t address) {
			if (_readfn)
				return _readfn(address);
			return 0xff;
		}
		void write(const uint16_t address, const uint8_t value) {
			if (_writefn)
				_writefn(address, value);
		}
		uint8_t* memory() const { return _memory; }

	protected:
		uint16_t _size{ 0 };
		bool _managed{ false };
		uint8_t* _memory{ nullptr };
		readfn_t _readfn{ nullptr };
		writefn_t _writefn{ nullptr };

		void load(const RomMappings&);
	};
		//mapping& mirror(const uint16_t mirror) { _mirroring = mirror; return *this; }
			//mapping& name(const string name) { _name = name; return *this; }
			//mapping& bank(const uint8_t bank) { _bank = bank; return *this; }
			//mapping& decodefn(std::function<void(uint8_t*, const size_t)>);

			//void init(const vector<aos::mmu::RomMapping>&);
			//bool is_readable() const { return _readfn != nullptr; }
			//bool is_writable() const { return _writefn != nullptr; }

			//void patch(const uint16_t address, const uint8_t value) { if (_memory) _memory[address - _start] = value; }

	class MemoryMap
	{
	public:
		MemoryMap(const uint16_t start, const uint16_t end) :
			_start{start}, _end{end}
		{}
		MemoryMap& mirror(const uint16_t mirroring) { _mirroring = mirroring; return *this; }

		bool is_mapped(const uint16_t address) const
		{
			const uint16_t address_mirror = address & _mirroring;
			return ((address_mirror >= _start) && (address_mirror <= _end)) ? true : false;
		}
		uint16_t map_address(const uint16_t address) const { return (address & _mirroring) - _start; }

	protected:
		uint16_t _start;
		uint16_t _end;
		uint16_t _mirroring{ 0xffff };
	};

	class Mmu
	{
	public:
		Mmu();
		~Mmu();

		// Configuration functions
		MemoryBank& bank(const string, const uint16_t);
		MemoryBank& bank(const string);

		MemoryMap& map(const uint16_t, const uint16_t, const string);
		void init(const RomMappings&);

		// Access functions
		uint8_t read(const uint16_t);
		void write(const uint16_t, const uint8_t);


		//tempo
		uint8_t* getvram() { return _banks["vram"].memory(); }
	protected:
		aos::map<string, MemoryBank> _banks;
		vector<std::pair<MemoryMap, string>> _mappings;
	};
}
