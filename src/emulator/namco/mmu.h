#pragma once
#include "types.h"
#include "emulator.h"


namespace aos::namco
{
	class Mmu
	{
	protected:
		struct memorymap {
			uint16_t start;
			uint16_t end;
			uint16_t mirroring;
		};

		struct memory {
			bool delegated;
			size_t size;
			uint8_t* mymemory;
			std::function<uint8_t(const uint16_t)> myreadfn;
			std::function<void(const uint16_t, const uint8_t)> mywritefn;
		};
		using memoryPtr = std::shared_ptr<memory>;

	public:
		class mapping
		{
		public:
			mapping(memoryPtr map) : _map(map) {}
			void rom();
			void ram();
			void shared(std::function<uint8_t(const uint16_t)>, std::function<void(const uint16_t, const uint8_t)>);
			mapping& readfn(std::function<uint8_t(const uint16_t)>);
			mapping& writefn(std::function<void(const uint16_t, const uint8_t)>);

		protected:
			memoryPtr _map;
		};
		Mmu()
		{
		};
		~Mmu();

		mapping map(const uint16_t, const uint16_t, const uint16_t, const string);
		mapping map(const uint16_t, const uint16_t, const string);
		mapping map(const uint16_t, const uint16_t, const uint16_t);
		mapping map(const uint16_t, const uint16_t);
		void init(const vector<aos::emulator::RomConfiguration>&);

		uint8_t read(const uint16_t);
		void write(const uint16_t, const uint8_t);

	protected:
		std::map<string, size_t> _naming;
		std::vector<std::pair<memorymap, memoryPtr>> _mapping;
	};

}