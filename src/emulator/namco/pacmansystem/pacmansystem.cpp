#include "pacmansystem.h"
#include "pacmansystemdecoder.h"


namespace aos::namco
{
	PacmanSystem2::PacmanSystem2(const Configuration configuration) :
		_configuration{ configuration }
	{
		_gpu = namco::PacmanGpu::create({
				.orientation = geometry_t::rotation_t::ROT90,
				.tileModel = PacmanGpu::Configuration::TileModel::PACMAN,
				.spriteAddress = static_cast<uint16_t>((configuration.hardware.alibaba) ? 0xef0 : 0xff0) });
	}

	void PacmanSystem2::settings(const string& parameter, const uint8_t value)
	{
		if (parameter == "dsw1")
			_dsw1 = value;
	}

	DisplayDevice::DisplayRequirements PacmanSystem2::getRequirements() const
	{
		return {
			.geometry = {.width = 288, .height = 224, .rotation = geometry_t::rotation_t::ROT90 }
		};
	}

	void PacmanSystem2::createRoms()
	{
		using MemoryType = Configuration::Roms::MemoryType;
		switch (_configuration.roms.extendedMemoryType)
		{
		case MemoryType::NO:
			if (_configuration.roms.cpudecoder != "")
			{
				_mmu.map(0, 0x3fff).mirror(0x7fff).name("cpu").rom().decodefn(getDecoder(_configuration.roms.cpudecoder));
			}
			else
			{
				_mmu.map(0, 0x3fff).mirror(0x7fff).name("cpu").rom();
			}
			break;
		case MemoryType::ROM:
			if (_configuration.roms.cpudecoder != "")
			{
				_mmu.map(0, 0x3fff).name("cpu").rom().decodefn(getDecoder(_configuration.roms.cpudecoder));
				_mmu.map(0x8000, 0xbfff).name("cpu2").rom().decodefn(getDecoder(_configuration.roms.cpudecoder));
			}
			else
			{
				_mmu.map(0, 0x3fff).name("cpu").rom();
				_mmu.map(0x8000, 0xbfff).name("cpu2").rom();
			}
			break;
		case MemoryType::RAMROM:
			_mmu.map(0, 0x3fff).name("cpu").rom();
			_mmu.map(0x8000, 0x8fff).name("cpu2").rom();
			_mmu.map(0x9000, 0x9fff).ram();
			_mmu.map(0xa000, 0xa7ff).name("cpu3").rom();
			break;
		case MemoryType::BANK2:
			_mmu.map(0, 0x3fff).mirror(0x7fff).bank(1).name("cpu").rom();
			_mmu.map(0, 0x3fff).bank(2).name("cpu2").rom().decodefn(getDecoder("mspacmanU7"));
			_mmu.map(0x8000, 0xbfff).bank(2).name("cpu2b").rom().decodefn(getDecoder("mspacmanU56"));
			break;
		}
	}
	void PacmanSystem2::patchBank2()
	{
		_mmu.selectBank(2);

		// Patch
		static const uint16_t tab[] = {
	0x0410, 0x8008, 0x08E0, 0x81D8, 0x0A30, 0x8118, 0x0BD0, 0x80D8,
	0x0C20, 0x8120, 0x0E58, 0x8168, 0x0EA8, 0x8198, 0x1000, 0x8020,
	0x1008, 0x8010, 0x1288, 0x8098, 0x1348, 0x8048, 0x1688, 0x8088,
	0x16B0, 0x8188, 0x16D8, 0x80C8, 0x16F8, 0x81C8, 0x19A8, 0x80A8,
	0x19B8, 0x81A8, 0x2060, 0x8148, 0x2108, 0x8018, 0x21A0, 0x81A0,
	0x2298, 0x80A0, 0x23E0, 0x80E8, 0x2418, 0x8000, 0x2448, 0x8058,
	0x2470, 0x8140, 0x2488, 0x8080, 0x24B0, 0x8180, 0x24D8, 0x80C0,
	0x24F8, 0x81C0, 0x2748, 0x8050, 0x2780, 0x8090, 0x27B8, 0x8190,
	0x2800, 0x8028, 0x2B20, 0x8100, 0x2B30, 0x8110, 0x2BF0, 0x81D0,
	0x2CC0, 0x80D0, 0x2CD8, 0x80E0, 0x2CF0, 0x81E0, 0x2D60, 0x8160
		};
		for (size_t i = 0; i < 80; i += 2)
		{
			for (uint8_t j = 0; j < 8; ++j)
			{
				uint8_t a = _mmu.read(tab[i + 1] + j);
				_mmu.patch(tab[i] + j, _mmu.read(tab[i + 1] + j));
			}
		}
		_mmu.before_fn([this](const uint16_t address) { beforeRW(address); });
	}
	void PacmanSystem2::beforeRW(const uint16_t address)
	{
		if ((address >= 0x0038 && address <= 0x003f) ||
			(address >= 0x03b0 && address <= 0x03b7) ||
			(address >= 0x1600 && address <= 0x1607) ||
			(address >= 0x2120 && address <= 0x2127) ||
			(address >= 0x3ff0 && address <= 0x3ff7) ||
			(address >= 0x8000 && address <= 0x8007) ||
			(address >= 0x97f0 && address <= 0x97f7))
		{
			_mmu.selectBank(1);
		}
		if (address >= 0x3ff8 && address <= 0x3fff)
		{
			_mmu.selectBank(2);
		}
	}

	void PacmanSystem2::init(map<string, Device::SharedPtr> devices)
	{
		if (_configuration.interruptdecoder != "")
		{
			_interrupt_decoder = getInterruptDecoder(_configuration.interruptdecoder);
		}

		createRoms();

		_mmu.map(0x4000, 0x4fff).mirror(0x5fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
		_mmu.map(0x5000, 0x503f).mirror(0x7fff).readfn([this](const uint16_t) { return _port0; });
		_mmu.map(0x5040, 0x507f).mirror(0x7fff).readfn([this](const uint16_t) { return _port1; });
		_mmu.map(0x5080, 0x50bf).mirror(0x7fff).readfn([this](const uint16_t) { return _dsw1; });

		if (_configuration.hardware.alibaba)
		{
			_mmu.map(0x50c0, 0x50c0).readfn([](const uint16_t) { return rand() & 0x0f; });
			_mmu.map(0x50c1, 0x50c1).readfn([this](const uint16_t) { return (_mystery++ >> 10) & 1; });

			_mmu.map(0x50c2, 0x50c2).writefn([this](const uint16_t, const uint8_t value) { _enableInterrupt = (((value & 1) == 1) ? true : false); });
			_mmu.map(0x50c1, 0x50c1).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5040, 0X504f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address & 0xff, value); });
			_mmu.map(0x5050, 0x505f).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
			_mmu.map(0x5060, 0X506f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(0x10 | address, value); });
		}
		else
		{
			_mmu.map(0x5000, 0x5000).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _enableInterrupt = (((value & 1) == 1) ? true : false); });
			_mmu.map(0x5003, 0x5003).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5040, 0X505f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address & 0xff, value); });
			_mmu.map(0x5060, 0x506f).mirror(0x5fff).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
		}

		auto raster = static_cast<aos::device::RasterDisplay*>(devices["display"].get());
		if (_configuration.roms.videodecoder!="")
			_gpu->romDecodingFn(getDecoder(_configuration.roms.videodecoder));
		_gpu->init(raster, _configuration.roms.files);
		_mmu.init(_configuration.roms.files);
		_wsg.init(_configuration.roms.files);

		_controller = static_cast<aos::device::ArcadeController*>(devices["controller"].get());

		if (_configuration.roms.extendedMemoryType = Configuration::Roms::BANK2)
			patchBank2();
	}

	void PacmanSystem2::updatePorts()
	{
		_port0 = 0xff;
		if (_controller->joystick1(device::ArcadeController::up))
			_port0 &= ~1;
		if (_controller->joystick1(device::ArcadeController::left))
			_port0 &= ~2;
		if (_controller->joystick1(device::ArcadeController::right))
			_port0 &= ~4;
		if (_controller->joystick1(device::ArcadeController::down))
			_port0 &= ~8;
		if (_controller->coin(device::ArcadeController::coin1))
			_port0 &= (_configuration.invertedcoin)? ~0x40 : ~0x20;

		_port1 = 0xff;
		if (_controller->button(device::ArcadeController::start1))
			_port1 &= ~0x20;
		if ((_configuration.controller.fire) && (_controller->joystick1(device::ArcadeController::fire)))
			_port1 &= ~0x10;
	}
	void PacmanSystem2::run()
	{
		_clock.reset();
		int tick = 0;
		while (0 == 0) {
			if (_clock.tickable())
			{
				tick++;
				if (tick > 51200)
				{
					tick -= 51200;
					_gpu->draw();

					updatePorts();

					if (_enableInterrupt)
						_cpu.interrupt();
				}
				_cpu.tick();

				auto pins = _cpu.getControlPins();
				if (pins & (xprocessors::cpu::Z80::PIN_MREQ))
				{
					if (pins & (xprocessors::cpu::Z80::PIN_RD))
					{
						const uint16_t address = _cpu.getAddressBus();
						_cpu.setDataBus(_mmu.read(address));
					}
					if (pins & (xprocessors::cpu::Z80::PIN_WR))
					{
						const uint16_t address = _cpu.getAddressBus();
						_mmu.write(address, _cpu.getDataBus());
					}
				}
				if (pins & (xprocessors::cpu::Z80::PIN_IORQ))
				{
					const uint16_t address = _cpu.getAddressBus();
					if (pins & (xprocessors::cpu::Z80::PIN_WR))
					{
						if ((address & 0x00ff) == 0)
						{
							if (_interrupt_decoder)
								_interrupt_vector = _interrupt_decoder(_cpu.getDataBus());
							else
								_interrupt_vector = _cpu.getDataBus();
						}
					}
					if (pins & (xprocessors::cpu::Z80::PIN_RD)) {
						_cpu.setDataBus(_interrupt_vector);
					}
				}

				if (tick % 32 == 0) {
					_wsg.tick();
				}
			}
		}
	}
}
