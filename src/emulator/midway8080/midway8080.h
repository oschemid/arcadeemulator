#pragma once
#include "types.h"
#include "emulator.h"
#include "xprocessors.h"
#include "mb14241.h"
#include <vector>
#include <bitset>
#include "../../controller/arcadecontroller.h"


namespace ae::midway8080
{
	class Port
	{
	public:
		Port(const uint8_t);
		void set(uint8_t, const string&);
		void reset();

		void init(const emulator::Game&);
		void tick(const ae::controller::ArcadeController&);
		uint8_t get() const;
	protected:
		using definition = std::tuple<uint8_t, string>;

		std::bitset<8> _port;
		std::vector<definition> _definition;
	};
	class GameBoard
	{
	public:
		enum class DisplayOrientation { Horizontal, Vertical };
		using Ptr = std::shared_ptr<GameBoard>;

		static Ptr create(const string&);

		GameBoard(DisplayOrientation);
		virtual ~GameBoard();
		virtual void init(const emulator::Game&) = 0;

		DisplayOrientation displayOrientation() const { return _displayRotation; }
		virtual bool romExtended() const { return false; }
		virtual std::vector<std::pair<uint16_t,string>> romFiles() const = 0;
		virtual rgb_t color(const uint8_t, const uint8_t, const bool) const = 0;
		virtual void out(const uint8_t, const uint8_t) = 0;
		virtual uint8_t in(const uint8_t) = 0;

		virtual void controllerTick() = 0;

	protected:
		DisplayOrientation _displayRotation;
	};
	using RegistryHandler = ae::RegistryHandler<GameBoard::Ptr, std::function<GameBoard::Ptr()>>;

	class Midway8080 : public emulator::Emulator
	{
	public:
		Midway8080(const emulator::Game&);
		virtual ~Midway8080();

		emulator::SystemInfo getSystemInfo() const override;
		void init() override;
		void run(ae::display::RasterDisplay*) override;

	protected:
		xprocessors::Cpu::Ptr _cpu{ nullptr };
		GameBoard::Ptr _cartridge{ nullptr };
		emulator::Game _game;
		uint8_t* _memory{ nullptr };
		bool _romextended{ false };
		ae::display::RasterDisplay* _raster;

		void updateDisplay();
	};
}