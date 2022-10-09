#include "i8080_tests.h"
#include "../src/cpu/i8080.h"
#include <iostream>


tests_ae::cpu::i8080_tests::i8080_tests() {
	cpu = ae::Cpu::create("i8080");
	cpu->read([this](const uint16_t p) { return memory->read(p); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return memory->write(p, v); });
}

void tests_ae::cpu::i8080_tests::out(const uint8_t p, const uint8_t v) {
	if (p == 0) {
		finished = true;
		return;
	}
	ae::cpu::Intel8080* cpuc = (ae::cpu::Intel8080*)cpu;
	if (p == 1) {
		uint8_t operation = cpuc->c();

		if (operation == 2) { // print a character stored in E
			std::cout << (char)(cpuc->e());
		}
		else if (operation == 9) { // print from memory at (DE) until '$' char
			uint16_t addr = cpuc->de();
			do {
				std::cout << (char)(memory->read(addr++));
			} while (memory->read(addr) != '$');
		}

	}
}
bool tests_ae::cpu::i8080_tests::runTest(const string& filename, const uint64_t cycles_expected) {
	uint64_t cycles = 0;
	memory = ae::newMemory(0);
	memory->map(0, 0xffff, ae::IMemory::type::RAM);
	memory->load(0x100, filename);
	memory->write(0, 0xD3);
	memory->write(1, 0x00);
	memory->write(5, 0xD3);
	memory->write(6, 0x01);
	memory->write(7, 0xC9);
	cpu->read([this](const uint16_t p) { return memory->read(p); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return memory->write(p, v); });
	cpu->reset(0x100);
	cpu->in([](const uint8_t) { return 0; });
	cpu->out([this](const uint8_t p, const uint8_t v) { out(p, v); });

	finished = false;
	while (!finished) {
		cycles += cpu->executeOne();
	}
	if (cycles_expected > 0)
		std::cout << std::endl << "Expected cycles : " << cycles_expected << " - Cycles : " << cycles;
	return true;
}

bool tests_ae::cpu::i8080_tests::run() {
	std::cout << "TST8080.COM" << std::endl;
	runTest("data/cpu/Intel8080/TST8080.COM", 4924);
	std::cout << std::endl;
	std::cout << "8080PRE.COM" << std::endl;
	runTest("data/cpu/Intel8080/8080PRE.COM", 7817);
	std::cout << std::endl;
	std::cout << "8080EXM.COM" << std::endl;
	runTest("data/cpu/Intel8080/8080EXM.COM", 23803381171);
	std::cout << std::endl;
	std::cout << "CPUTEST.COM" << std::endl;
	runTest("data/cpu/Intel8080/CPUTEST.COM", 255653383);
	std::cout << std::endl;
	return true;
}