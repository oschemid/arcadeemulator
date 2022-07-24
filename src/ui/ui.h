#pragma once
#include "types.h"
#include "machine.h"

#include <cstdint>


namespace ae
{
	namespace ui
	{
		// UI creation
		bool init();
		bool refresh();
		bool destroy();

		// Console Display
		bool createDisplay(const std::uint16_t, const std::uint16_t);
		bool updateDisplay(const std::uint16_t*);
		bool destroyDisplay();

		// Interfaces
		class IInterface
		{
		protected:
			virtual bool render() = 0;
			bool setStyle();
		public:
			bool run();
		};
		class InterfaceGameSelection : public IInterface
		{
		protected:
			string selection;

		protected:
			bool render() override;
		public:
			InterfaceGameSelection(const string&);
			const string getSelection() const;
		};

		// Menu
		class menu
		{
		public:
			enum response { NOTHING, LAUNCH, GAMESELECTION, GAMESETTINGS, QUIT };

		public:
			menu();
			response run(IMachine*);
		};
	}
}