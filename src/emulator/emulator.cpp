#include "emulator.h"
#include "registry.h"
#include <chrono>


using namespace ae::emulator;


Emulator::Ptr ae::emulator::create(const string& name, const Game& game)
{
	return ae::Registry<Emulator::Ptr,Emulator::creator_fn>::instance().create(name)(game);
}

uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start) {
	auto diff = std::chrono::high_resolution_clock::now() - *start;
	return duration_cast<std::chrono::nanoseconds>(diff).count();
}