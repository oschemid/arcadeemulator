#pragma once
#include "types.h"
#include "SDL2/SDL.h"
#include <queue>
#include "xprocessors.h"
#include "channel.h"


namespace ae {
	namespace gameboy {
		class Apu {
		protected:
			SquareChannel _channel1;
			SquareChannel _channel2;
			WaveChannel _channel3;
			NoiseChannel _channel4;
			SDL_AudioDeviceID device;
			SDL_AudioSpec want;

			float _samples[10000];
			uint16_t _sampleW;

			xprocessors::out_fn _handlerOut;
			xprocessors::in_fn _handlerIn;

			bool _enabled;
			uint8_t _leftvolume;
			uint8_t _rightvolume;
			uint16_t _cycles;
			uint16_t _nbsamples;
			uint8_t _div;

			uint8_t _frameSequencer;
			void frameSequencerTick();

			void power(const bool);
			void restart_channel1();
			void restart_channel2();
			void restart_channel3();
			void restart_channel4();

		public:
			Apu();

			// Handlers
			bool in(const xprocessors::in_fn fn) noexcept { _handlerIn = fn; return true; }
			bool out(const xprocessors::out_fn fn) noexcept { _handlerOut = fn; return true; }

			void init();
			void callback(const uint8_t, const uint8_t);
			void tick();
		};
	}
}