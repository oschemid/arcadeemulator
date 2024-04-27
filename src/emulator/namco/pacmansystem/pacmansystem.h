#pragma once
#include "core.h"

#include "../../../libraries/xos-processors/src/experimental/z80.h"
#include "../gpu/pacmangpu.h"
#include "../mmu.h"
#include "../wsg.h"
#include "device/arcadecontroller.h"

//********************************************************************************
// Pacman System
//********************************************************************************
// This core emulates the following games:
//   * Abscam
//   * Ali Baba and the 40 Thieves
//   * Eggor
//   - Eyes
//   * Gorkans
//   - Jumpshot
//   * Mr TNT
//   - Ms Pac Man
//   - Naughty Mouse
//   * Pac Man (Puck Man)
//   - Pac Man Plus
//   - Piranha
//   * Titan
//   - Woodpecker
//********************************************************************************
// Configuration
// controller
//   * fire - joystick1 has a fire button
// invertedcoin - coin is inverted
// interruptdecoder - function called to translates interrupt vector
// hardware
//   * alibaba - specific hardware for Alibaba games
// roms
//   * extendedMemoryType
//     * NO no extended memory
//     * ROMS ROM between 0x8000 and 0xc000
//     * RAMROMS ROM between 0x8000 and 0x9000, RAM between 0x9000 and 0xa000
//               and ROM between 0xa000 and 0xc000
//   * cpudecoder - function to decode ROMs
//   * videodecoder - function to decode video ROMs
//   * files - mappings for ROM files (cpu,cpu2,cpu3 and video,palette and sound)
//********************************************************************************
namespace aos::namco
{
	class PacmanSystem2 : public Core
	{
	public:
		struct Configuration
		{
			struct
			{
				bool fire{ false };
			} controller;
			bool invertedcoin{ false };

			string interruptdecoder{ "" };

			struct Hardware
			{
				bool alibaba{ false };
			} hardware;

			struct Roms
			{
				enum MemoryType { NO, ROM, RAMROM, BANK2 } extendedMemoryType{ NO };
				string cpudecoder{ "" };
				string videodecoder{ "" };
				aos::mmu::RomMappings files;
			} roms;
		};
		PacmanSystem2(const Configuration);
		virtual ~PacmanSystem2() {}

		virtual DisplayDevice::DisplayRequirements getRequirements() const override;
		virtual void init(map<string, Device::SharedPtr>) override;
		virtual void run() override;

		void settings(const string&, const uint8_t);

	protected:
		Configuration _configuration;

		xprocessors::cpu::Z80 _cpu;
		namco::PacmanGpu::Ptr _gpu{ nullptr };
		namco::wsg _wsg;
		device::ArcadeController* _controller{ nullptr };
		Mmu _mmu;

		uint8_t _port0{ 0xff };
		uint8_t _port1{ 0xff };
		uint8_t _dsw1{ 0xff };
		uint16_t _mystery{ 0 };

		tools::Clock _clock{ 3072 };
		uint8_t _interrupt_vector{ 0 };
		std::function<uint8_t(const uint8_t)> _interrupt_decoder{ nullptr };
		bool _enableInterrupt{ false };

		void createRoms();
		void patchBank2();
		void beforeRW(const uint16_t);
		void updatePorts();
	};
}
