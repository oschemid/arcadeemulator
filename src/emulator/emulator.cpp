#include "emulator.h"
#include "registry.h"
#include <chrono>


using namespace ae::emulator;


UEmulator Emulator::create(const string& id)
{
	return Registry<UEmulator>::instance().create(id);
}

uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start) {
	auto diff = std::chrono::high_resolution_clock::now() - *start;
	return duration_cast<std::chrono::nanoseconds>(diff).count();
}