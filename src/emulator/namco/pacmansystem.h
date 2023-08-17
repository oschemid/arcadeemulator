#pragma once
#include "emulator.h"
#include "tilemap.h"
#include "xprocessors.h"
#include "io.h"
#include "../../controller/arcadecontroller.h"
#include "wsg.h"
#include "mmu.h"


namespace aos::namco
{
	// Namco Pacman System
	template <class Gpu>class PacmanSystem : public aos::emulator::Emulator
	{
	public:
		PacmanSystem(const vector<aos::emulator::RomConfiguration>&,
			const aos::emulator::GameConfiguration&,
			Gpu::Ptr);
		virtual ~PacmanSystem() {}

		aos::emulator::SystemInfo getSystemInfo() const override;

		void init(aos::display::RasterDisplay*) override;
		uint8_t tick() override;

		std::map<string, aos::tilemap::Tiles> getTiles() const override {
			return { {"tiles", _gpu->getTiles()}, {"sprites", _gpu->getSprites()}};
		}
		std::vector<palette_t> getPalettes() const override {
			return _gpu->getPalettes();
		}

	protected:
		xprocessors::Cpu::Ptr _cpu{ nullptr };
		Gpu::Ptr _gpu{ nullptr };
		wsg _wsg;
		Mmu _mmu;
		ae::controller::ArcadeController::Ptr _controller;

		Gpu::Configuration _gpuconfig{ };

		virtual void mapping() = 0;

		vector<aos::emulator::RomConfiguration> _roms;
		aos::io::Port _port0{ 0 };
		aos::io::Port _port1{ 0 };
		aos::io::Port _port2{ 0 };
		aos::io::Port _port3{ 0 };

		bool _interrupt_enabled{ false };
		uint8_t _interrupt_vector{ 0 };
	};

	template<class Gpu> PacmanSystem<Gpu>::PacmanSystem(const vector<aos::emulator::RomConfiguration>& roms, const aos::emulator::GameConfiguration& game,
		Gpu::Ptr gpu) :
		_roms(roms)
	{
		_clockPerMs = 3072;

		_cpu = xprocessors::Cpu::create("Z80");
		_gpu = std::move(gpu);
	}

	template<class Gpu> aos::emulator::SystemInfo PacmanSystem<Gpu>::getSystemInfo() const
	{
		return aos::emulator::SystemInfo{
			.geometry = _gpu->getGeometry()
		};
	}


	template<class Gpu> void PacmanSystem<Gpu>::init(aos::display::RasterDisplay* raster)
	{
		mapping();
		_mmu.init(_roms);

		_controller = ae::controller::ArcadeController::create();
		_wsg.init(_roms);

		_cpu->read([this](const uint16_t p) { return _mmu.read(p); });
		_cpu->write([this](const uint16_t p, const uint8_t v) { _mmu.write(p, v); });
		_cpu->in([this](const uint8_t) { return 0; });
		_cpu->out([this](const uint8_t p, const uint8_t v) { if (p == 0) _interrupt_vector = v; });

		_gpu->init(raster, _roms);
	}

	template<class Gpu> uint8_t PacmanSystem<Gpu>::tick()
	{
		static uint64_t deltaDisplay{ 0 };
		static uint64_t deltaSound{ 0 };
		const uint64_t clockDisplay = 1000 / 60 * _clockPerMs;

		uint64_t previous = _cpu->elapsed_cycles();
		_cpu->executeOne();
		uint64_t deltaClock = _cpu->elapsed_cycles() - previous;

		if (deltaDisplay > clockDisplay) { // 60 Hz
			_gpu->draw();
			if (_interrupt_enabled)
				_cpu->interrupt(_interrupt_vector);
			deltaDisplay -= clockDisplay;
			_controller->tick(); _port0.tick(*_controller); _port1.tick(*_controller);
		}
		deltaDisplay += deltaClock;

		if (deltaSound > 32) {
			_wsg.tick();
			deltaSound -= 32;
		}
		deltaSound += deltaClock;
		return static_cast<uint8_t>(deltaClock);
	}
}