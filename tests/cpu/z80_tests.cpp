#include "z80_tests.h"
#include "../../src/cpu/z80.h"
#include <iostream>


tests_ae::cpu::z80_tests::z80_tests() :
	finished(false),
	memory(nullptr) {
	cpu = ae::Cpu::create("Z80");
	cpu->read([this](const uint16_t p) { return memory->read(p); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return memory->write(p, v); });
}

bool tests_ae::cpu::z80_tests::runTestCycles() {
	const std::pair<uint64_t, uint64_t> opcodes[] = {
		// Group 4 - 8-Bit Arithmetic and Logical Group
		{ 0x80000000, 4 }, { 0x86000000, 7 }, { 0xdd800000, 8 }, {0xfd800000, 8 }, {0xc6000000, 7 }, { 0xdd860000, 19 }, { 0xfd860000, 19 }, // ADD
		{ 0x88000000, 4 }, { 0x8e000000, 7 }, { 0xdd880000, 8 }, {0xfd880000, 8 }, {0xce000000, 7 }, { 0xdd8e0000, 19 }, { 0xfd8e0000, 19 }, // ADC
		{ 0x90000000, 4 }, { 0x96000000, 7 }, { 0xdd900000, 8 }, {0xfd900000, 8 }, {0xd6000000, 7 }, { 0xdd960000, 19 }, { 0xfd960000, 19 }, // SUB
		{ 0x98000000, 4 }, { 0x9e000000, 7 }, { 0xdd980000, 8 }, {0xfd980000, 8 }, {0xde000000, 7 }, { 0xdd9e0000, 19 }, { 0xfd9e0000, 19 }, // SBC
		{ 0xa0000000, 4 }, { 0xa6000000, 7 }, { 0xdda00000, 8 }, {0xfda00000, 8 }, {0xe6000000, 7 }, { 0xdda60000, 19 }, { 0xfda60000, 19 }, // AND
		{ 0xb0000000, 4 }, { 0xb6000000, 7 }, { 0xddb00000, 8 }, {0xfdb00000, 8 }, {0xf6000000, 7 }, { 0xddb60000, 19 }, { 0xfdb60000, 19 }, // OR
		{ 0xa8000000, 4 }, { 0xae000000, 7 }, { 0xdda80000, 8 }, {0xfda80000, 8 }, {0xee000000, 7 }, { 0xddae0000, 19 }, { 0xfdae0000, 19 }, // XOR
		{ 0xb8000000, 4 }, { 0xbe000000, 7 }, { 0xddb80000, 8 }, {0xfdb80000, 8 }, {0xfe000000, 7 }, { 0xddbe0000, 19 }, { 0xfdbe0000, 19 }, // CP
		{ 0x04000000, 4 }, { 0xdd040000, 8 }, { 0xfd040000, 8 }, {0x34000000, 11 }, {0xdd340000, 23 }, { 0xfd340000, 23 }, // INC
		{ 0x05000000, 4 }, { 0xdd050000, 8 }, { 0xfd050000, 8 }, {0x35000000, 11 }, {0xdd350000, 23 }, { 0xfd350000, 23 }, // DEC

		// Group 5 - General-Purpose Arithmetic and CPU Control Group
		{ 0x27000000, 4 }, { 0xed440000, 8 }, // DAA, NEG
		{ 0x2f000000, 4 }, { 0x37000000, 4 }, { 0x3f000000, 4 }, // CPL, SCF, CCF
		{ 0x00000000, 4 }, { 0x76000000, 4 }, { 0xf3000000, 4 }, { 0xfb000000, 4 }, // NOP, HALT, DI, EI
		{ 0xed460000, 8 }, { 0xed560000, 8 }, { 0xed5e0000, 8 }, // IM
		// Group 6 - 16-Bit Arithmetic Group
		{ 0x09000000, 11 }, { 0xdd190000, 15 }, { 0xfd290000, 15 }, // ADD
		{ 0xed4a0000, 15 }, { 0xed420000, 15 }, // ADC, SBC
		{ 0x03000000, 6 }, { 0xdd230000, 10 }, {0xfd230000, 10 }, // INC
		{ 0x0b000000, 6 }, { 0xdd2b0000, 10 }, {0xfd2b0000, 10 }, // DEC

		// Group 7 - Rotate and Shift Group
		{ 0x07000000, 4}, { 0x0f000000, 4 }, { 0x17000000, 4 }, { 0x1f000000, 4 }, // RLCA, RLA, RRCA, RRA
		{ 0xcb000000, 8 }, { 0xcb060000, 15 }, { 0xddcb0006, 23 }, { 0xfdcb0006, 23 }, { 0xddcb0001, 23 }, { 0xfdcb0002, 23 }, // RLC
		{ 0xcb100000, 8 }, { 0xcb160000, 15 }, { 0xddcb0016, 23 }, { 0xfdcb0016, 23 }, { 0xddcb0011, 23 }, { 0xfdcb0012, 23 }, // RL
		{ 0xcb080000, 8 }, { 0xcb0e0000, 15 }, { 0xddcb000e, 23 }, { 0xfdcb000e, 23 }, { 0xddcb0009, 23 }, { 0xfdcb000a, 23 }, // RRC
		{ 0xcb180000, 8 }, { 0xcb1e0000, 15 }, { 0xddcb001e, 23 }, { 0xfdcb001e, 23 }, { 0xddcb0019, 23 }, { 0xfdcb001a, 23 }, // RR
		{ 0xcb200000, 8 }, { 0xcb260000, 15 }, { 0xddcb0026, 23 }, { 0xfdcb0026, 23 }, { 0xddcb0021, 23 }, { 0xfdcb0022, 23 }, // SLA
		{ 0xcb300000, 8 }, { 0xcb360000, 15 }, { 0xddcb0036, 23 }, { 0xfdcb0036, 23 }, { 0xddcb0031, 23 }, { 0xfdcb0032, 23 }, // SLL
		{ 0xcb280000, 8 }, { 0xcb2e0000, 15 }, { 0xddcb002e, 23 }, { 0xfdcb002e, 23 }, { 0xddcb0029, 23 }, { 0xfdcb002a, 23 }, // SRA
		{ 0xcb380000, 8 }, { 0xcb3e0000, 15 }, { 0xddcb003e, 23 }, { 0xfdcb003e, 23 }, { 0xddcb0039, 23 }, { 0xfdcb003a, 23 }, // SRL
		{ 0xed670000, 18 }, { 0xed6f0000, 18 }, // RRD, RLD

		// Group 8 - Bit Set, Reset and Test Group
		{ 0xcb400000, 8 }, { 0xcb460000, 12 }, { 0xddcb0046, 20 }, { 0xfdcb0046, 20 }, // BIT
		{ 0xcbc00000, 8 }, { 0xcbc60000, 15 }, { 0xddcb00c6, 23 }, { 0xfdcb00c6, 23 }, { 0xddcb00c1, 23 }, { 0xfdcb00c2, 23 }, // SET
		{ 0xcb800000, 8 }, { 0xcb860000, 15 }, { 0xddcb0086, 23 }, { 0xfdcb0086, 23 }, { 0xddcb0081, 23 }, { 0xfdcb0082, 23 }, // RES
	};

	memory = ae::newMemory(0xff);
	memory->map(0, 0xff, ae::IMemory::type::RAM);
	cpu->read([this](const uint16_t p) { return memory->read(p); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return true; });
	ae::cpu::Z80* cpuc = (ae::cpu::Z80*)cpu;

	for (auto pair : opcodes) {
		auto mem = pair.first;
		cpu->reset();
		memory->write(0, (mem >> 24) & 0xff);
		memory->write(1, (mem >> 16) & 0xff);
		memory->write(2, (mem >> 8) & 0xff);
		memory->write(3, mem & 0xff);

		cpu->executeOne();
		if (cpuc->elapsed_cycles() != pair.second) {
			std::cout << std::hex << mem << std::dec << " - Expected cycles : " << pair.second << " - Cycles : " << cpuc->elapsed_cycles() << std::endl;
		}
	}
	return true;
}

void tests_ae::cpu::z80_tests::out(const uint8_t p, const uint8_t v) {
	if (p == 0) {
		finished = true;
		return;
	}
	ae::cpu::Z80* cpuc = (ae::cpu::Z80*)cpu;
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
bool tests_ae::cpu::z80_tests::runTest(const string& filename, const uint64_t cycles_expected) {
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

	/* PATCH */
//	memory->write(0x120, 0x4a + 70);
	finished = false;
	ae::cpu::Z80* cpuc = (ae::cpu::Z80*)cpu;
	while (!finished) {
		cycles += cpu->executeOne();
	}
	if (cycles_expected > 0)
		std::cout << std::endl << "Expected cycles : " << cycles_expected << " - Cycles : " << cpuc->elapsed_cycles();
	return true;
}

bool tests_ae::cpu::z80_tests::run() {
	std::cout << "check cycles" << std::endl;
	runTestCycles();
	std::cout << std::endl;
	std::cout << "prelim.com" << std::endl;
	runTest("data/cpu/Z80/prelim.com", 8721Ui64);
	std::cout << std::endl;
	std::cout << "zexdoc.com" << std::endl;
	runTest("data/cpu/Z80/zexdoc.com", 8721Ui64);
	std::cout << std::endl;
	//	std::cout << "zexall.com" << std::endl;
	//	runTest("data/cpu/Z80/zexall.com", 8721Ui64);
	//	std::cout << std::endl;
	return true;
}