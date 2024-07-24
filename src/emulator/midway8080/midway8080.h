#pragma once
#include "core.h"
#include "types.h"
#include "emulator.h"
#include "i8080.h"
#include "memory.h"
#include "device/arcadecontroller.h"
//#include "io.h"
#include "device/rasterdisplay.h"
#include "mb14241.h"


namespace aos::midway8080
{
	// Console Taito/Midway8080
	// ------------------------
	// Cpu : Intel 8080 (1.997MHz)

	class Midway8080 : public Core
	{
	public:
		struct Configuration
		{
			struct Display
			{
				bool rotated{ false };
				enum
				{
					WHITE,
					GREEN,
					SPACEINVADERS,
					COSMICMONSTERS,
					SPACECHASER
				} colormap;
			} display;
			struct Controllers
			{
				struct Joystick
				{
					enum JoystickType { NO, JOYSTICK4WAY, JOYSTICK2WAYFIRE, JOYSTICK4WAYFIRE } type;
					string mapping;
				} joysticks;
				string buttons;
			} controllers;
			struct Roms
			{
				bool extendedRoms{ false };
				aos::mmu::RomMappings files;
			} roms;
		};
		Midway8080(const Configuration&);
		virtual ~Midway8080();

		virtual json getRequirements() const override;
		virtual void init(map<string, Device::SharedPtr>) override;
		virtual void run() override;

		void setPorts(const uint8_t port1, const uint8_t port2) { _port1_dsw = port1; _port2_dsw = port2; }
	protected:
		Configuration _configuration;
		xprocessors::cpu::i8080dbg _cpu;
		mmu::Mmu _mmu;
		aos::device::RasterDisplay* _raster;
		tools::Clock _clock{ 1997 };
		xprocessors::MB14241::Ptr _shifter{ nullptr };
		uint8_t* _colorram{ nullptr };
		uint8_t _port0{ 1 };
		uint8_t _port1{ 0 };
		uint8_t _port1_dsw{ 0 };
		uint8_t _port2{ 0 };
		uint8_t _port2_dsw{ 0 };
		device::ArcadeController* _controller1{ nullptr };
		device::ArcadeController* _controller2{ nullptr };

		std::function<rgb_t(const uint8_t, const uint8_t, const bool)>	_colorfn;
		uint8_t in(const uint8_t);
		void out(const uint8_t, const uint8_t);

		void updateDisplay();
		void updatePorts();
	};
}