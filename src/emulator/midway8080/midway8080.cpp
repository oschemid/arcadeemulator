#include "midway8080.h"


namespace aos::midway8080
{
	aos::rgb_t spaceinvader_gels(const uint8_t x, const uint8_t y, const bool b)
	{
		if (!b)
			return { 0,0,0 };
		if ((x <= 223) && (x > 194))
			return { 255, 32,32 };
		if ((x <= 72) && (x >= 16))
			return { 32, 255, 32 };
		if ((x < 16) && (y >= 16) && (y < 134))
			return  { 32, 255,32 };
		return { 255, 255, 255 };
	}

	aos::rgb_t cosmicmo_gels(const uint8_t x, const uint8_t y, const bool b)
	{
		if (!b)
			return { 0,0,0 };
		if (x > 192)
			return { 38, 255, 38 };
		if (x < 73)
			return { 255,255,38 };
		return { 255,128,255 };
	}

	aos::rgb_t green_gels(const uint8_t x, const uint8_t y, const bool b)
	{
		return (b) ? aos::rgb_t{ 51, 255, 127 } : aos::rgb_t{ 0,0,0 };
	}

	Midway8080::Midway8080(const Configuration& configuration) :
		_raster { nullptr },
		_configuration{ configuration }
	{
	}

	Midway8080::~Midway8080()
	{
		if (_colorram)
			delete[] _colorram;
	}

	void Midway8080::init(map<string, Device::SharedPtr> devices)
	{
		_raster = static_cast<aos::device::RasterDisplay*>(devices["display"].get());

		_shifter = xprocessors::MB14241::create();
		_controller1 = static_cast<aos::device::ArcadeController*>(devices["controller1"].get());
		_controller2 = static_cast<aos::device::ArcadeController*>(devices["controller2"].get());

		_mmu.bank("cpu", 0x2000).rom(); // 8k ROM
		_mmu.bank("ram", 0x400).ram(); // 1k RAM
		_mmu.bank("vram", 0x1c00).ram(); // 7k Video RAM
		if (_configuration.roms.extendedRoms)
		{
			_mmu.bank("cpu2", 0x2000).rom();
			_mmu.map(0x0000, 0x1fff, "cpu").mirror(0x7fff);
			_mmu.map(0x4000, 0x5fff, "cpu2").mirror(0x7fff);
			if (_configuration.display.colormap != Configuration::Display::SPACECHASER)
			{
				_mmu.map(0x2000, 0x23ff, "ram").mirror(0x7fff);
				_mmu.map(0x2400, 0x3fff, "vram").mirror(0x7fff);
			}
			else
			{
				_mmu.map(0x2000, 0x23ff, "ram");
				_mmu.map(0x2400, 0x3fff, "vram");
			}
		}
		else
		{
			_mmu.map(0x0000, 0x1fff, "cpu").mirror(0x3fff);
			_mmu.map(0x2000, 0x23ff, "ram").mirror(0x3fff);
			_mmu.map(0x2400, 0x3fff, "vram").mirror(0x3fff);
		}

		_mmu.init(_configuration.roms.files);

		switch (_configuration.display.colormap)
		{
		case Configuration::Display::GREEN:
			_colorfn = green_gels;
			break;
		case Configuration::Display::SPACEINVADERS:
			_colorfn = spaceinvader_gels;
			break;
		case Configuration::Display::COSMICMONSTERS:
			_colorfn = cosmicmo_gels;
			break;
		case Configuration::Display::SPACECHASER:
			_colorram = new uint8_t[0x2000];
			_mmu.bank("cram", 0x2000).readfn([this](const uint16_t p) { return _colorram[(p & 0x1f) | ((p & 0x1f80) >> 2)]; }).writefn([this](const uint16_t p, const uint8_t v) { _colorram[(p & 0x1f) | ((p & 0x1f80) >> 2)] = v; });
			_mmu.map(0xc400, 0xffff, "cram");
			_colorfn = nullptr;
			break;
		default:
			_colorfn = nullptr;
			break;
		}
	}

	json Midway8080::getRequirements() const
	{
		json requirements{
			{"display", {
				{"type", "display"},
				{"width", 256},
				{"height", 224},
				{"rotation", (_configuration.display.rotated)? "NONE" : "ROT270"} }
			}
		};
		requirements["controller1"] = { {"type", "joystick"}, {"fire", true }, {"secundary", false} };
		requirements["controller2"] = { {"type", "joystick"}, {"fire", true }, {"secundary", true} };

		return requirements;
	}

	void Midway8080::updateDisplay()
	{
		uint8_t* _memory = _mmu.getvram();
		for (int y = 0; y < 224; y++)
		{
			for (int x = 0; x < 256; x += 8)
			{
				uint16_t offset = (y << 5) + (x >> 3);
				uint8_t VRAMByte = _memory[offset];
				uint8_t colorid = 7;
				if (_colorram)
				{
					colorid = _colorram[((offset & 0x1f) | ((offset & 0x1f80) >> 2))] & 0x07;
				}
				for (int bit = 0; bit < 8; bit++)
				{
					uint8_t CoordX;
					uint8_t CoordY;

					CoordX = x + bit;
					CoordY = y;

					aos::rgb_t color = { 0,0,0 };
					if (_colorfn)
						color = _colorfn(CoordX, CoordY, (VRAMByte >> bit) & 1);
					else
					{
						if ((VRAMByte >> bit) & 1)
						{
							switch (colorid)
							{
							case 0:
								color = { 128, 128, 255 };
								break;
							case 1:
								color = { 255, 0, 0 };
								break;
							case 2:
								color = { 0,0,255 };
								break;
							case 3:
								color = { 255,0,255 };
								break;
							case 4:
								color = { 0,255,0 };
								break;
							case 5:
								color = { 255, 255, 0 };
								break;
							case 6:
								color = { 0,255,255 };
								break;
							case 7:
								color = { 255, 255, 255 };
								break;
							default:
								break;
							}
						}
					}
					_raster->set(CoordX, CoordY, color);
				}
			}
		}
	}

	void Midway8080::run()
	{
		_clock.reset();
		int tick = 0;
		bool ioexpected = false;
		bool intexpected = false;
		int draw = 0;
		_cpu.setReady();

//		_cpu.pause();
		_cpu.setMemoryAccessor([this](uint16_t a) { return _mmu.read(a); });
//		std::ofstream stream("log.log");

		while (0 == 0)
		{
//			if (!_cpu.isrunning())
//			{
//				stream << _cpu.opcode() << std::endl;
//				_cpu.runStep();
//			}
			if (_clock.tickable())
			{
				tick++;
				if (tick > 16641)
				{
					tick -= 16641;
					_cpu.sendInterrupt();
					if (draw)
					{
						updateDisplay();
						_raster->refresh();
	//						_cpu.interrupt(2);
					}
	//					else
	//						_cpu.interrupt(1);
					draw = 1 - draw;

					updatePorts();
				}
				_cpu.tick();

				const uint8_t pins = _cpu.getControlPins();
				if (pins & xprocessors::cpu::i8080::PIN_SYNC)
				{
					ioexpected = (_cpu.getDataBus() & (xprocessors::cpu::i8080::STATUS_OUT|xprocessors::cpu::i8080::STATUS_INP));
					intexpected = (_cpu.getDataBus() & xprocessors::cpu::i8080::STATUS_INTA);
					if (intexpected)
						_cpu.ackInterrupt();
				}
				if (pins & xprocessors::cpu::i8080::PIN_DBIN)
				{
					const uint16_t address = _cpu.getAddressBus();
					if (ioexpected)
					{
						_cpu.setDataBus(in(address & 0x00ff));
					}
					else if (intexpected)
					{
						if (draw)
						{
							_cpu.setDataBus(0xcf);
						}
						else
						{
							_cpu.setDataBus(0xd7);
						}
					}
					else
					{
						_cpu.setDataBus(_mmu.read(address));
					}
				}
				if ((pins & xprocessors::cpu::i8080::PIN_WR))
				{
					const uint16_t address = _cpu.getAddressBus();
					if (ioexpected)
					{
						out(address & 0x00ff, _cpu.getDataBus());
					}
					else
					{
						_mmu.write(address, _cpu.getDataBus());
					}
				}
			}
		}
	}

	void Midway8080::updatePorts()
	{
		_port0 = 0;
		if (_configuration.controllers.joysticks.mapping == "0")
		{
			if (_controller1->joystick(device::ArcadeController::left))
				_port0 |= 1;
			if (_controller1->joystick(device::ArcadeController::right))
				_port0 |= 2;
			if (_controller1->joystick(device::ArcadeController::down))
				_port0 |= 4;
			if (_controller1->joystick(device::ArcadeController::up))
				_port0 |= 8;
			if (_controller2->joystick(device::ArcadeController::left))
				_port0 |= 16;
			if (_controller2->joystick(device::ArcadeController::right))
				_port0 |= 32;
			if (_controller2->joystick(device::ArcadeController::down))
				_port0 |= 64;
			if (_controller2->joystick(device::ArcadeController::up))
				_port0 |= 128;
		}
		else
		{
			_port0 = 1;
		}

		_port1 = _port1_dsw;
		if (_configuration.controllers.joysticks.mapping == "12")
		{
			if (_controller1->joystick(device::ArcadeController::fire))
				_port1 |= 16;
			if (_controller1->joystick(device::ArcadeController::left))
				_port1 |= 32;
			if (_controller1->joystick(device::ArcadeController::right))
				_port1 |= 64;
			if (_configuration.controllers.joysticks.type = Configuration::Controllers::Joystick::JOYSTICK4WAYFIRE)
			{
				if (_controller1->joystick(device::ArcadeController::up))
					_port1 |= 128;
				if (_controller1->joystick(device::ArcadeController::down))
					_port1 |= 8;
			}
		}
		if (_configuration.controllers.buttons == "C21")
		{
			if (_controller1->coin(device::ArcadeController::coin1))
				_port1 |= 1;
			if (_controller1->button(device::ArcadeController::start2))
				_port1 |= 2;
			if (_controller1->button(device::ArcadeController::start1))
				_port1 |= 4;
		}
		if (_configuration.controllers.buttons == "12.C")
		{
			if (_controller1->button(device::ArcadeController::start1))
				_port1 |= 1;
			if (_controller1->button(device::ArcadeController::start2))
				_port1 |= 2;
			if (_controller1->coin(device::ArcadeController::coin1))
				_port1 |= 8;
		}

		_port2 = 0x80 | _port2_dsw;
		if (_configuration.controllers.joysticks.mapping == "12")
		{
			if (_controller2->joystick(device::ArcadeController::fire))
				_port2 |= 16;
			if (_controller2->joystick(device::ArcadeController::left))
				_port2 |= 32;
			if (_controller2->joystick(device::ArcadeController::right))
				_port2 |= 64;
			if (_configuration.controllers.joysticks.type = Configuration::Controllers::Joystick::JOYSTICK4WAYFIRE)
			{
				if (_controller2->joystick(device::ArcadeController::up))
					_port2 |= 4;
				if (_controller2->joystick(device::ArcadeController::down))
					_port2 |= 2;
			}
		}
	}
	uint8_t Midway8080::in(const uint8_t port)
	{
		uint8_t result{ 0 };

		switch (port)
		{
		case 0:
			return _port0;
		case 1:
			return _port1;
		case 2:
			return _port2;
		case 3:
			return _shifter->readValue();
		default:
			return result;
		}
	}

	void Midway8080::out(const uint8_t port, const uint8_t value)
	{
		switch (port)
		{
		case 2:
			_shifter->writeOffset(value);
			break;
		case 4:
			_shifter->writeValue(value);
			break;
		}
	}
}
