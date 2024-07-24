#include "database.h"
#include "emulator.h"
#include "xprocessors.h"
#include "registry.h"
#include "tools.h"
#include "z80.h"
#include "../../libraries/xos-processors/src/chips/am40010.h"
#include "../../libraries/xos-processors/src/chips/mc6845.h"
#include "../../libraries/xos-processors/src/chips/i8255.h"
#include "../../libraries/xos-processors/src/chips/ay38912.h"
#include "amstradmmu.h"
#include "device/amstradkeyboard.h"
#include "fdc765.h"
#include "disk/loader_dsk.h"
#include "device/rasterdisplay.h"

#include "core.h"
#include "tools.h"


namespace aos::amstrad
{
	bool debugging = false;

	//class AmstradDebugger : public aos::emulator::DebuggerTools
	//{
	//public:
	//	std::atomic_flag pause;
	//};

	class AmstradCPC : public Core
	{
	public:
		AmstradCPC(const aos::mmu::RomMappings&);
		virtual ~AmstradCPC() {}

		json getRequirements() const override;
		void init(map<string, Device::SharedPtr>) override;
		void run() override;

	protected:
		tools::Clock _clock{ 16000 };
		xprocessors::cpu::Z80* _cpu{ nullptr };
		xprocessors::am40010 _gatearray;
		xprocessors::mc6845 _crtc;
		xprocessors::i8255 _ppi;
		xprocessors::ay38912 _psg;
		FDC765 _fdc;
		disk::Disk* _disk{ nullptr };
		uint8_t _scanline{ 0 };
		aos::mmu::RomMappings _roms;
		Memory _memory;

		aos::device::AmstradKeyboard* _keyboard;
		aos::device::RasterDisplay* _raster;

//		xprocessors::cpu::Z80debugging _debug;
		uint64_t  _tick{ 0 };

		uint16_t x = 0;
		uint16_t y = 0;
		bool is_hsync = false;

		void tick();
	};

	AmstradCPC::AmstradCPC(
		const aos::mmu::RomMappings& roms) :
		_roms(roms)
	{
		_cpu = new xprocessors::cpu::Z80();
	}

	json AmstradCPC::getRequirements() const
	{
		return json{
			{"display", {
				{"type", "display"},
				{"width", 384},
				{"height", 272},
				{"rotation", "NONE"} }
			},
			{"keyboard", {
				{"type", "amstradkbd"} }
			}
		};
	}

	void AmstradCPC::init(map<string, Device::SharedPtr> devices)
	{
		_memory.init(_roms);
		_gatearray.init(&_crtc, _memory.getVRAM());
		_raster = static_cast<aos::device::RasterDisplay*>(devices["display"].get());
		_keyboard = static_cast<aos::device::AmstradKeyboard*>(devices["keyboard"].get());
		_gatearray.memory_notifier([this](bool r1, bool r2) { _memory.enableRom1(r1); _memory.enableRom2(r2); });
		_ppi.porta_handlers([this]() { return _psg.getDatabus(); }, [this](const uint8_t d) { _psg.setDatabus(d); });
		_ppi.portb_handlers([this]() { return (_crtc.vsync()) ? 0xff : 0xfe; }, nullptr);
		_ppi.portc_handlers(nullptr, [this](const uint8_t d) { _scanline = d & 0x0f; _psg.setControl(d >> 6); });
		_psg.handler([this]() { return _keyboard->getLine(_scanline); });

//		_debug.activateDisassembly();
		//_disk = disk::LoaderDsk().load("roms\\amstrad\\XMas-Snow_Fun__(Amstrad_Computer_User)__ENGLISH.dsk");
		_disk = disk::LoaderDsk().load("roms\\amstrad\\Arkanoid (UK) (1987) [Original].dsk");
		//		_disk = disk::LoaderDsk().load("roms\\amstrad\\Miami Vice (UK) (1986) [Original].dsk");
		//		_disk = disk::LoaderDsk().load("roms\\amstrad\\Commando (1985)(Elite Systems).dsk");
		//		_disk = disk::LoaderDsk().load("roms\\amstrad\\Boulder Dash (1984)(Prism Leisure)[cr FMR].dsk");

		_fdc.insert(_disk);
	}

	void AmstradCPC::run()
	{
		_clock.reset();
		while (0 == 0)
		{
			if (_clock.tickable())
				tick();
		}
	}
	void AmstradCPC::tick()
	{
		//#ifdef _DEBUG
		//		_debugger.pause.wait(true);
		//#endif
		const uint8_t* vram = _memory.getVRAM();

		_tick++;

		if ((_tick & 0x01) == 0)
		{
			_gatearray.tick();
			auto beam = _gatearray.get_beam();
			if (beam.vsync)
			{
				if (y > 1)
					_raster->refresh();
				y = 0;
			}
			else
			{
				if (_crtc.get_vma() == 0xf800)
					x = x;
				if (((x < 0x5e * 4)) && (y < 0x110))
				{
					_raster->set(x, y, beam.color);
				}
			}
			if (!beam.hsync)
			{
				x++;
				is_hsync = false;
			}
			else
			{
				if (!is_hsync)
				{
					x = 0;
					y++;
					is_hsync = true;
				}
			}
		}

		if ((_tick & 0x0f) == 0)
		{
			_crtc.tick();
		}

		if ((_tick & 0x03) == 0)
		{
			//			if (_cpu->pc == 0xc93d)
			//				debugging = true;
	//		if (debugging)
	//			_debug.debug(_cpu, [this](const uint16_t a) { return _memory.read(a); });
			_cpu->tick();
			auto pins = _cpu->getControlPins();
			if (pins & (xprocessors::cpu::Z80::PIN_MREQ))
			{
				if (pins & (xprocessors::cpu::Z80::PIN_RD))
				{
					const uint16_t address = _cpu->getAddressBus();
					_cpu->setDataBus(_memory.read(address));
				}
				if (pins & (xprocessors::cpu::Z80::PIN_WR))
				{
					const uint16_t address = _cpu->getAddressBus();
					_memory.write(address, _cpu->getDataBus());
				}
			}
			if (pins & (xprocessors::cpu::Z80::PIN_IORQ))
			{
				const uint16_t address = _cpu->getAddressBus();
				if ((address & 0xc000) == 0x4000)
					_gatearray.tickio(_cpu->getDataBus());
				if ((address & 0x4000) == 0x0000)
					if (pins & (xprocessors::cpu::Z80::PIN_RD | xprocessors::cpu::Z80::PIN_WR))
						_crtc.tickio(address >> 8, _cpu->getDataBus());
				if ((address & 0x0800) == 0x0000)
				{
					if (pins & (xprocessors::cpu::Z80::PIN_WR))
					{
						_ppi.write((address & 0x0300) >> 8, _cpu->getDataBus());
					}
					if (pins & (xprocessors::cpu::Z80::PIN_RD))
					{
						_cpu->setDataBus(_ppi.read((address & 0x0300) >> 8));
					}
				}
				if ((address & 0x0580) == 0x0000)
				{
					if (pins & (xprocessors::cpu::Z80::PIN_WR))
						_fdc.motor(_cpu->getDataBus() & 0x0f);
				}
				if ((address & 0x0580) == 0x0100)
				{
					if (pins & (xprocessors::cpu::Z80::PIN_RD))
						_cpu->setDataBus(_fdc.read(address & 0xff));
					if (pins & (xprocessors::cpu::Z80::PIN_WR))
						_fdc.write(address & 0xff, _cpu->getDataBus());
				}
				if ((address & 0x2000) == 0x0000)
				{
					// ROM selection
					if (pins & (xprocessors::cpu::Z80::PIN_WR))
					{
						_memory.selectRom(_cpu->getDataBus());
					}
				}
			}
		}
		if ((_tick & 0x1fff) == 0)
		{
			_cpu->interrupt();
		}
	}

	//class AmstradCPCMachine : public Machine
	//{
	//public:
	//	AmstradCPCMachine();
	//	~AmstradCPCMachine() = default;
	//protected:
	//	virtual DisplayDevice::DisplayRequirements getRequirements() const override { return _core->getRequirements(); }
	//	virtual void init(map<string, Device::SharedPtr> devices) override { _core->init(devices); }
	//};

	//AmstradCPCMachine::AmstradCPCMachine()
	//{
	//	_core = std::make_unique<aos::amstrad::AmstradCPC>(aos::mmu::RomMappings({{"os", 0x4000, 0x874fd0c1},
	//		{"basic", 0x4000, 0x6f8d661e},
	//		{"amsdos", 0x4000, 0x1fe22ecd}}));
	//}

	static tools::RomManager& romcache()
	{
		static tools::RomManager instance("roms/amstrad");
		return instance;
	}
	static bool cache_loaded = false;

	Core::Ptr createAmstradCPC(const json& settings)
	{
		if (!cache_loaded)
		{
			romcache().preload();
			cache_loaded = true;
		}

		mmu::RomMappings mappings;
		for (auto& [zone, roms] : settings["roms"].items())
		{
			for (auto& rom : roms)
			{
				mmu::RomMapping mapping{ zone, {"",""},{0,0} };
				if (rom.contains("file"))
				{
					mapping.rom = romcache().get(rom["file"]);
					mappings.push_back(mapping);
				}
			}
		}
		return std::make_unique<aos::amstrad::AmstradCPC>(mappings);
	}

	static bool registering = CoreFactory::registerCore("amstrad", createAmstradCPC);
}