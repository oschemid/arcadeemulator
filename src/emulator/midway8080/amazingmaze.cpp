#include "registry.h"
#include "tools.h"
#include "types.h"
#include "midway8080.h"
#include "emulator.h"
#include "../../controller/arcadecontroller.h"
#include "io.h"


namespace aos::midway8080
{
	class AmazingMaze : public Midway8080
	{
	public:
		AmazingMaze(const vector<aos::mmu::RomMapping>&,
			const emulator::GameConfiguration&);
		virtual ~AmazingMaze();

		void init(aos::display::RasterDisplay*, aos::controller::Controller*) override;

	protected:
		uint8_t in(const uint8_t);
		void out(const uint8_t, const uint8_t);
	};
}


using namespace aos::midway8080;


AmazingMaze::AmazingMaze(const vector<aos::mmu::RomMapping>& roms, const aos::emulator::GameConfiguration& config) :
	aos::midway8080::Midway8080{ roms }
{
	_port0.set(0, "_JOY1_LEFT");
	_port0.set(1, "_JOY1_RIGHT");
	_port0.set(2, "_JOY1_DOWN");
	_port0.set(3, "_JOY1_UP");
	_port0.set(4, "_JOY2_LEFT");
	_port0.set(5, "_JOY2_RIGHT");
	_port0.set(6, "_JOY2_DOWN");
	_port0.set(7, "_JOY2_UP");
	_port0.init(config);

	_port1.set(4, "coinage");
	_port1.set(6, "gametime");
	_port1.set(3, "_COIN");
	_port1.set(0, "_START1");
	_port1.set(1, "_START2");
	_port1.set(7, "service");
	_port1.init(config);
}

void AmazingMaze::init(aos::display::RasterDisplay* raster, aos::controller::Controller*)
{
	Midway8080::init(raster, nullptr);

	_controller = ae::controller::ArcadeController::create();

	_mmu.bank("cpu", 0x2000).rom(); // 8k ROM
	_mmu.bank("ram", 0x400).ram(); // 1k RAM
	_mmu.bank("vram", 0x1c00).ram(); // 7k Video RAM

	_mmu.map(0x0000, 0x1fff, "cpu").mirror(0x3fff);
	_mmu.map(0x2000, 0x23ff, "ram").mirror(0x3fff);
	_mmu.map(0x2400, 0x3fff, "vram").mirror(0x3fff);
	_mmu.init(_roms);
}

AmazingMaze::~AmazingMaze()
{
}

void AmazingMaze::out(const uint8_t port, const uint8_t value) 
{
}

uint8_t AmazingMaze::in(const uint8_t port) {
	uint8_t result{ 0 };
	switch (port) {
	case 0:
		return _port0.get();
	case 1:
		return _port1.get();
	default:
		return result;
	}
}

static aos::RegistryHandler<aos::emulator::GameDriver> amazingmaze("amazingmaze", {
	.name = "Amazing Maze",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& rom) { 
		auto emul = std::make_unique<aos::midway8080::AmazingMaze>(rom, config);
		emul->rotateDisplay();
		emul->colorfn([](const uint8_t, const uint8_t, const bool b) { return (b) ? aos::rgb_t{ 51, 255, 127 } : aos::rgb_t{ 0,0,0 }; });
		return emul;
	},
	.roms = {
			{ "cpu", 0x800,0xf2860cff },
			{ "cpu", 0x800,0x65fad839 }
			},
	.configuration = {
		.switches = {{ "coinage", 0, "Coin", {"One coin One credit", "Two coins One credit", "One coin Two credits", "Two coins Two redits"} },
					 { "gametime", 0, "Two player time", {"6 mins", "4 mins"} },
					 { "service", 0, "Service", {"Off", "On"} }
					  }
	}
});
