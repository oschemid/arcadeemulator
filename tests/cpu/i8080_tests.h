#pragma once

#include "cpu.h"


namespace tests_ae
{
	namespace cpu
	{
		class i8080_tests {
		protected:
			ae::ICpu* cpu;
			ae::memory memory;
			bool finished;

			void out(const uint8_t, const uint8_t);
			bool runTest(const string&);

		public:
			i8080_tests();

			bool run();
		};
	}
}
