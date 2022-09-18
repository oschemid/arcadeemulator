#pragma once

#include "cpu.h"

using ae::string;
using ae::uint8_t;


namespace tests_ae
{
	namespace cpu
	{
		class i8080_tests {
		protected:
			ae::Cpu* cpu;
			ae::IMemory* memory;
			bool finished;

			void out(const uint8_t, const uint8_t);
			bool runTest(const string&, const uint64_t = 0);

		public:
			i8080_tests();

			bool run();
		};
	}
}
