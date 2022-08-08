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
			ae::ICpu* cpu;
			ae::IMemory* memory;
			bool finished;

			void out(const uint8_t, const uint8_t);
			bool runTest(const string&);

		public:
			i8080_tests();

			bool run();
		};
	}
}
