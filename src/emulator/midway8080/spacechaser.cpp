#include "registry.h"
#include "tools.h"
#include "types.h"
#include "midway8080.h"
#include "mb14241.h"
#include "../../controller/arcadecontroller.h"
#include "io.h"

using namespace aos::midway8080;



namespace aos::midway8080
{
	class SpaceChaser : public Midway8080
	{
	public:
		SpaceChaser(const std::vector<aos::mmu::RomMapping>&,
			const emulator::GameConfiguration&);
		virtual ~SpaceChaser();

		emulator::SystemInfo getSystemInfo() const override {
			return emulator::SystemInfo{
				.geometry = {.width = 224, .height = 256}
			};
		}

		void init(aos::display::RasterDisplay*, aos::controller::Controller*) override;

	protected:
		xprocessors::MB14241::Ptr _shifter{ nullptr };
		std::function<rgb_t(const uint8_t, const uint8_t)> _colorfn;

		uint8_t* _colorram;

		uint8_t in(const uint8_t);
		void out(const uint8_t, const uint8_t);
		void updateDisplay();
		void writeColorRam(const uint16_t, const uint8_t);
		uint8_t readColorRam(const uint16_t) const;
	};
}

SpaceChaser::SpaceChaser(const std::vector<aos::mmu::RomMapping>& roms, const aos::emulator::GameConfiguration& config) :
	aos::midway8080::Midway8080{ roms }
{
	_port0 = 0xff;
	_port1.set(0, "_COIN");
	_port1.set(1, "_START2");
	_port1.set(2, "_START1");
	_port1.set(3, "_JOY1_DOWN");
	_port1.set(4, "_JOY1_FIRE");
	_port1.set(5, "_JOY1_LEFT");
	_port1.set(6, "_JOY1_RIGHT");
	_port1.set(7, "_JOY1_UP");
	_port1.init(config);

	_port2.set(0, "ships");
	_port2.set(1, "_JOY2_DOWN");
	_port2.set(2, "_JOY2_UP");
	_port2.set(3, "difficulty");
	_port2.set(4, "_JOY2_FIRE");
	_port2.set(5, "_JOY2_LEFT");
	_port2.set(6, "_JOY2_RIGHT");
	_port2.init(config);
}

void SpaceChaser::init(aos::display::RasterDisplay* raster, aos::controller::Controller*)
{
	Midway8080::init(raster, nullptr);


	_shifter = xprocessors::MB14241::create();
	_controller = ae::controller::ArcadeController::create();

	_mmu.bank("cpu", 0x2000).rom();
	_mmu.bank("ram", 0x0400).ram();
	_mmu.bank("vram", 0x1c00).ram();
	_mmu.bank("cpu2", 0x0800).rom();
	_mmu.bank("cram", 0x2000).managed(true).readfn([this](const uint16_t p) { return readColorRam(p); }).writefn([this](const uint16_t p, const uint8_t v) { writeColorRam(p, v); });
	
	_mmu.map(0x0000, 0x1fff, "cpu").mirror(0x7fff);
	_mmu.map(0x2000, 0x23ff, "ram").mirror(0x7fff);
	_mmu.map(0x2400, 0x3fff, "vram").mirror(0x7fff);
	_mmu.map(0x4000, 0x47ff, "cpu2");
	_mmu.map(0xc400, 0xffff, "cram");

	_mmu.init(_roms);
	_colorram = _mmu.bank("cram").memory(); // new uint8_t[0x2000];
}

uint8_t SpaceChaser::readColorRam(const uint16_t p) const
{
	return _colorram[(p & 0x1f) | ((p & 0x1f80) >> 2)];
}

void SpaceChaser::writeColorRam(const uint16_t p, const uint8_t v)
{
	_colorram[(p & 0x1f) | ((p & 0x1f80) >> 2)] = v;
}

void SpaceChaser::updateDisplay()
{
	uint8_t* _memory = _mmu.getvram();
	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint8_t VRAMByte = _memory[(x << 5) + (y >> 3)];

			for (int bit = 0; bit < 8; bit++) {
				uint8_t CoordX;
				uint8_t CoordY;
				CoordX = x;
				CoordY = (256 - 1 - (y + bit));

				aos::rgb_t color = { 0,0,255 };
				if ((VRAMByte >> bit) & 1) {
					uint16_t offset = (x << 5) + (y >> 3);
					uint8_t colorid = _colorram[((offset & 0x1f) | ((offset & 0x1f80) >> 2))] & 0x07;
					switch (colorid) {
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
				_raster->set(CoordX, CoordY, color);
			}
		}
	}
}


SpaceChaser::~SpaceChaser()
{
}

void SpaceChaser::out(const uint8_t port, const uint8_t value) {
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

uint8_t SpaceChaser::in(const uint8_t port) {
	uint8_t result = 0;
	switch (port) {
	case 0:
		return _port0.get();
	case 1:
		return _port1.get();
	case 2:
		return _port2.get();
	case 3:
		return _shifter->readValue();
	default:
		return (uint8_t)0;
	}
}

static aos::RegistryHandler<aos::emulator::GameDriver> spacechaser("spacechaser", {
	.name = "Space Chaser",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& rom) { return std::make_unique<aos::midway8080::SpaceChaser>(rom, config); },
	.roms = {
			{ "cpu", 0x400, 0xbec2b16b },
			{ "cpu", 0x400, 0x9d25e608 },
			{ "cpu", 0x400, 0x113d0635 },
			{ "cpu", 0x400, 0xf3a43c8d },
			{ "cpu", 0x400, 0x47c84f23 },
			{ "cpu", 0x400, 0x02ff2199 },
			{ "cpu", 0x400, 0x87d06b88 },
			{ "cpu", 0x400, 0x6dfaad08 },
			{ "cpu2", 0x400, 0x3d1a2ae3 },
			{ "cpu2", 0x400, 0x037edb99 }
			},
	.configuration = {
		.switches = {{ "difficulty", 0, "Difficulty", {"Normal", "Hard"} },
					 { "ships", 0, "Ships", {"3", "4"} }
		  }
	}
});
