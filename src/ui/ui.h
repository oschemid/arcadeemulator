#pragma once
#include "types.h"
#include "SDL2/SDL.h"

#include "machine.h"


namespace ae
{
	namespace ui
	{
		// UI management
		bool init();
		bool destroy();

		// Accessor
		SDL_Renderer* getRenderer();

		// Graphic
		bool createButton(const string&, bool);

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
		class InterfaceGameSettings : public IInterface
		{
		protected:
			IMachine* _machine;

		protected:
			bool render() override;

		public:
			InterfaceGameSettings(IMachine*);
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