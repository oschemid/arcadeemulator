#pragma once
#include "types.h"
#include "emulator.h"
#include "xprocessors.h"


namespace ae::taito8080
{
	class Cartridge;
	using UCartridge = std::unique_ptr<Cartridge>;
	using RegistryHandler = ae::RegistryHandler<UCartridge>;

	class Cartridge
	{
	protected:
		uint8_t* _memory;

	public:
		Cartridge();
		virtual ~Cartridge();
		virtual void init(const json&) = 0;
		virtual uint8_t read(const uint16_t) = 0;
		virtual void out(const uint8_t, const uint8_t) = 0;
		virtual void write(const uint16_t, const uint8_t) = 0;
		virtual uint8_t in(const uint8_t) = 0;
		virtual void updateDisplay(uint32_t*) = 0;

	public:
		static UCartridge create(const string&);
	};

	class Taito8080 : public emulator::Emulator
	{
	protected:
		xprocessors::Cpu::Ptr _cpu;
		UCartridge _cartridge;
		uint32_t* _src;

	public:
		Taito8080();
		virtual ~Taito8080() = default;

		emulator::SystemInfo getSystemInfo() const override;
		void init(const json&) override;
		void run(ae::gui::RasterDisplay*) override;
	};
}