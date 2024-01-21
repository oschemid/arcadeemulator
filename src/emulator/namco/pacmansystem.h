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
	using decodingfn = std::function<void(const string, uint8_t*, const size_t)>;
	using portdecodingfn = std::function<uint8_t(const uint8_t, const uint8_t)>;
	using interruptdecodingfn = std::function<uint8_t(const uint8_t)>;

	class PacmanPort : public aos::io::Port
	{
	public:
		PacmanPort() : aos::io::Port(0xff) {}

		PacmanPort& joystick1(const bool active=true) {
			set(0, "_JOY1_UP", active);
			set(1, "_JOY1_LEFT", active);
			set(2, "_JOY1_RIGHT", active);
			set(3, "_JOY1_DOWN", active);
			return *this;
		}
		PacmanPort& joystick2(const bool active = true) {
			set(0, "_JOY2_UP", active);
			set(1, "_JOY2_LEFT", active);
			set(2, "_JOY2_RIGHT", active);
			set(3, "_JOY2_DOWN", active);
			return *this;
		}
		PacmanPort& fire2(const uint8_t bit = 4, const bool active = true) {
			set(bit, "_JOY2_FIRE", active);
			return *this;
		}
		PacmanPort& fire1(const uint8_t bit = 4, const bool active = true) {
			set(bit, "_JOY1_FIRE", active);
			return *this;
		}
		PacmanPort& coin(const uint8_t bit = 5, const bool active=true) {
			set(bit, "_COIN", active);
			return *this;
		}
		PacmanPort& starts(const bool active = true) {
			set(5, "_START1", active);
			set(6, "_START2", active);
			return *this;
		}
	};

	// Namco Pacman System
	template <class Gpu>class PacmanSystem : public aos::emulator::Emulator
	{
	public:
		PacmanSystem(
			const aos::mmu::RomMappings&,
			const aos::emulator::GameConfiguration&,
			Gpu::Ptr);
		virtual ~PacmanSystem() {}

		// Configuration Methods
		PacmanSystem& romDecodingFn(decodingfn decoder) {
			_romDecoder = decoder;
			_gpu->romDecodingFn(decoder);
			return *this;
		}
		PacmanSystem& interruptDecodingFn(interruptdecodingfn decoder) {
			_interruptDecoder = decoder;
			return *this;
		}
		PacmanSystem& portDecodingFn(portdecodingfn decoder) {
			_portDecoder = decoder;
			return *this;
		}

		aos::emulator::SystemInfo getSystemInfo() const override;

		void init(aos::display::RasterDisplay*) override;
		uint8_t tick() override;

		std::map<string, aos::tilemap::Tiles> getTiles() const override {
			return { {"tiles", _gpu->getTiles()}, {"sprites", _gpu->getSprites()}};
		}
		std::vector<palette_t> getPalettes() const override {
			return _gpu->getPalettes();
		}
		void enableInterrupts(const bool flag) { _interrupt_enabled = flag; }

		PacmanPort& port0() { return _port0; }
		PacmanPort& port1() { return _port1; }

		void rotateDisplay() { _gpu->rotate(); }

	protected:
		xprocessors::Cpu::Ptr _cpu{ nullptr };
		Gpu::Ptr _gpu{ nullptr };
		wsg _wsg;
		Mmu _mmu;
		ae::controller::ArcadeController::Ptr _controller;

		Gpu::Configuration _gpuconfig{ };

		virtual void mapping() = 0;

		aos::mmu::RomMappings _roms;
		decodingfn _romDecoder;
		portdecodingfn _portDecoder;
		PacmanPort _port0;
		PacmanPort _port1;
		aos::io::Port _port2{ 0 };
	public:
		aos::io::Port _port3{ 0 };
	protected:
		interruptdecodingfn _interruptDecoder;
		uint8_t _interrupt_vector{ 0 };

	private:
		bool _interrupt_enabled{ false };
	};

	template<class Gpu> PacmanSystem<Gpu>::PacmanSystem(
		const aos::mmu::RomMappings& roms,
		const aos::emulator::GameConfiguration& game,
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
		_cpu->out([this](const uint8_t p, const uint8_t v) { if (p == 0) { if (_interruptDecoder) _interrupt_vector = _interruptDecoder(v); else _interrupt_vector = v; } });

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
