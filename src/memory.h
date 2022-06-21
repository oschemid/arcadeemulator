#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <stdexcept>

using std::uint8_t;
using std::uint16_t;
using std::string;


namespace ae
{
	typedef std::function<const uint8_t(const uint16_t)> reader;
	typedef std::function<void(const uint16_t, const uint8_t)> writer;

	class memory
	{
	protected:
		uint8_t data[0x4000];

	public:
		memory();

		bool load(const string&, const uint16_t);
		const uint8_t read(const uint16_t address) const {
			if (address < 0x4000)
				return data[address];
			if (address >= 0x6000)
				return 0;
			return data[address - 0x4000];
		}
		void write(const uint16_t address,
				   const uint8_t value) {
			if (address < 0x2000)
				throw std::runtime_error("Write into ROM");
			if (address < 0x4000) {
				data[address] = value;
				return;
			}
			if (address < 0x6000)
				data[address - 0x2000] = value;
		}
	};
}