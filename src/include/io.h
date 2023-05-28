#pragma once
#include "types.h"
#include "emulator.h"
#include "../controller/arcadecontroller.h"
#include <vector>
#include <bitset>


namespace ae::io
{
	class Port
	{
	public:
		Port(const uint8_t);
		void set(uint8_t, const string&, bool=false);
		void reset();

		void init(const aos::emulator::GameConfiguration&);
		void tick(const ae::controller::ArcadeController&);
		uint8_t get() const;
	protected:
		using definition = std::tuple<uint8_t, string, bool>;

		std::bitset<8> _port;
		std::vector<definition> _definition;
	};
}