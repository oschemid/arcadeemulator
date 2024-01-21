#pragma once
#include "types.h"
#include "emulator.h"
#include "SDL2/SDL.h"


using aos::uint32_t;


namespace aos::namco
{
	class wsg
	{
	public:
		struct voice
		{
			uint32_t frequency;
			uint32_t counter;
			uint8_t waveform;
			uint8_t volume;
		};

	public:
		wsg() :_voices{ {},{},{} } {}
		~wsg();
		void init(const mmu::RomMappings&);
		uint8_t read(const uint8_t) const;
		void write(const uint8_t, const uint8_t);

		void tick();

	protected:
		uint8_t* _waves{ nullptr };
		voice _voices[3];

		SDL_AudioDeviceID device;
		SDL_AudioSpec want;

		int16_t _samples[10000];
		uint16_t _sampleW;
	};
}