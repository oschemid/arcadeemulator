#include "wsg.h"

#include <ranges>


namespace aos::namco
{
	wsg::~wsg()
	{
		if (_waves)
			delete[] _waves;
	}

	void wsg::init(const aos::emulator::RomsConfiguration& roms)
	{
		_waves = new uint8_t[0x100];

		uint16_t offset = 0;
		for (const auto& rom : roms | std::ranges::views::filter([](const auto i) { return i.region == "sound"; })) {
			offset += rom.rom.read(_waves + offset);
		}

		want.freq = 96000;
		want.format = AUDIO_S16SYS;
		want.channels = 1;
		want.samples = 1024;
		want.callback = nullptr;
		want.userdata = this;

		SDL_AudioSpec a;

		if ((device = SDL_OpenAudioDevice(NULL, 0, &want, &a, SDL_AUDIO_ALLOW_ANY_CHANGE)) == 0)
		{
			throw std::runtime_error("Warning: failed to open audio device");
		}
		SDL_PauseAudioDevice(device, 0);
	}

	uint8_t wsg::read(const uint8_t address) const
	{
		switch (address)
		{
		case 0x00:
			return _voices[0].counter & 0x0f;
		case 0x01:
			return (_voices[0].counter >> 4) & 0x0f;
		case 0x02:
			return (_voices[0].counter >> 8) & 0x0f;
		case 0x03:
			return (_voices[0].counter >> 12) & 0x0f;
		case 0x04:
			return (_voices[0].counter >> 16) & 0x0f;
		case 0x05:
			return _voices[0].waveform & 0x07;
		case 0x06:
			return _voices[1].counter & 0x0f;
		case 0x07:
			return (_voices[1].counter >> 4) & 0x0f;
		case 0x08:
			return (_voices[1].counter >> 8) & 0x0f;
		case 0x09:
			return (_voices[1].counter >> 12) & 0x0f;
		case 0x0a:
			return _voices[1].waveform & 0x07;
		case 0x0b:
			return _voices[0].counter & 0x0f;
		case 0x0c:
			return (_voices[2].counter >> 4) & 0x0f;
		case 0x0d:
			return (_voices[2].counter >> 8) & 0x0f;
		case 0x0e:
			return (_voices[2].counter >> 12) & 0x0f;
		case 0x0f:
			return _voices[2].waveform & 0x07;
		case 0x10:
			return _voices[0].frequency & 0x0f;
		case 0x11:
			return (_voices[0].frequency >> 4) & 0x0f;
		case 0x12:
			return (_voices[0].frequency >> 8) & 0x0f;
		case 0x13:
			return (_voices[0].frequency >> 12) & 0x0f;
		case 0x14:
			return (_voices[0].frequency >> 16) & 0x0f;
		case 0x15:
			return _voices[0].volume & 0x0f;
		case 0x16:
			return _voices[1].frequency & 0x0f;
		case 0x17:
			return (_voices[1].frequency >> 4) & 0x0f;
		case 0x18:
			return (_voices[1].frequency >> 8) & 0x0f;
		case 0x19:
			return (_voices[1].frequency >> 12) & 0x0f;
		case 0x1a:
			return _voices[1].volume & 0x0f;
		case 0x1b:
			return _voices[2].frequency & 0x0f;
		case 0x1c:
			return (_voices[2].frequency >> 4) & 0x0f;
		case 0x1d:
			return (_voices[2].frequency >> 8) & 0x0f;
		case 0x1e:
			return (_voices[2].frequency >> 12) & 0x0f;
		case 0x1f:
			return _voices[2].volume & 0x0f;
		default:
			return 0;
		}
	}

	void wsg::write(const uint8_t address, const uint8_t value)
	{
		switch (address)
		{
		case 0x00:
			_voices[0].counter &= 0xffff0;
			_voices[0].counter |= value & 0x0f;
			break;
		case 0x01:
			_voices[0].counter &= 0xfff0f;
			_voices[0].counter |= (value & 0x0f) << 4;
			break;
		case 0x02:
			_voices[0].counter &= 0xff0ff;
			_voices[0].counter |= (value & 0x0f) << 8;
			break;
		case 0x03:
			_voices[0].counter &= 0xf0fff;
			_voices[0].counter |= (value & 0x0f) << 12;
			break;
		case 0x04:
			_voices[0].counter &= 0x0ffff;
			_voices[0].counter |= (value & 0x0f) << 16;
			break;
		case 0x05:
			_voices[0].waveform = value & 0x07;
			break;
		case 0x06:
			_voices[1].counter &= 0xfff0;
			_voices[1].counter |= value & 0x0f;
			break;
		case 0x07:
			_voices[1].counter &= 0xff0f;
			_voices[1].counter |= (value & 0x0f) << 4;
			break;
		case 0x08:
			_voices[1].counter &= 0xf0ff;
			_voices[1].counter |= (value & 0x0f) << 8;
			break;
		case 0x09:
			_voices[1].counter &= 0x0fff;
			_voices[1].counter |= (value & 0x0f) << 12;
			break;
		case 0x0a:
			_voices[1].waveform = value & 0x07;
			break;
		case 0x0b:
			_voices[2].counter &= 0xfff0;
			_voices[2].counter |= value & 0x0f;
			break;
		case 0x0c:
			_voices[2].counter &= 0xff0f;
			_voices[2].counter |= (value & 0x0f) << 4;
			break;
		case 0x0d:
			_voices[2].counter &= 0xf0ff;
			_voices[2].counter |= (value & 0x0f) << 8;
			break;
		case 0x0e:
			_voices[2].counter &= 0x0fff;
			_voices[2].counter |= (value & 0x0f) << 12;
			break;
		case 0x0f:
			_voices[2].waveform = value & 0x07;
			break;
		case 0x10:
			_voices[0].frequency &= 0xffff0;
			_voices[0].frequency |= value & 0x0f;
			break;
		case 0x11:
			_voices[0].frequency &= 0xfff0f;
			_voices[0].frequency |= (value & 0x0f) << 4;
			break;
		case 0x12:
			_voices[0].frequency &= 0xff0ff;
			_voices[0].frequency |= (value & 0x0f) << 8;
			break;
		case 0x13:
			_voices[0].frequency &= 0xf0fff;
			_voices[0].frequency |= (value & 0x0f) << 12;
			break;
		case 0x14:
			_voices[0].frequency &= 0x0ffff;
			_voices[0].frequency |= (value & 0x0f) << 16;
			break;
		case 0x15:
			_voices[0].volume = value & 0x0f;
		case 0x16:
			_voices[1].frequency &= 0xfff00;
			_voices[1].frequency |= (value & 0x0f) << 4;
			break;
		case 0x17:
			_voices[1].frequency &= 0xff0f0;
			_voices[1].frequency |= (value & 0x0f) << 8;
			break;
		case 0x18:
			_voices[1].frequency &= 0xf0ff0;
			_voices[1].frequency |= (value & 0x0f) << 12;
			break;
		case 0x19:
			_voices[1].frequency &= 0x0fff0;
			_voices[1].frequency |= (value & 0x0f) << 16;
			break;
		case 0x1a:
			_voices[1].volume = value & 0x0f;
			break;
		case 0x1b:
			_voices[2].frequency &= 0xfff00;
			_voices[2].frequency |= (value & 0x0f) << 4;
			break;
		case 0x1c:
			_voices[2].frequency &= 0xff0f0;
			_voices[2].frequency |= (value & 0x0f) << 8;
			break;
		case 0x1d:
			_voices[2].frequency &= 0xf0ff0;
			_voices[2].frequency |= (value & 0x0f) << 12;
			break;
		case 0x1e:
			_voices[2].frequency &= 0x0fff0;
			_voices[2].frequency |= (value & 0x0f) << 16;
			break;
		case 0x1f:
			_voices[2].volume = value & 0x0f;
			break;
		defaut:
			break;
		}
	}

	void wsg::tick()
	{
		int16_t sample = 0;
		for (int voice_no = 0; voice_no < 3; voice_no++) {
			voice* const v = &_voices[voice_no];

			if (v->frequency == 0 || v->volume == 0) {
				continue;
			}

			v->counter = (v->counter + v->frequency) & 0xfffff;
			int sample_pos = v->waveform * 32 + (v->counter >> 15);

			int16_t voice_sample = (_waves[sample_pos] - 8) * v->volume;
			sample += voice_sample*85;
		}
		_samples[_sampleW++] = sample;

		if (_sampleW == 4096) {
			SDL_QueueAudio(device, _samples, 4096 * 2);
			_sampleW = 0;
		}
	}
}