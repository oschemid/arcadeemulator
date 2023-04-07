#include "sound.h"
#include <stdexcept>


using namespace ae;

AudioDevice::AudioDevice() :
	sound(nullptr),
	buffer(nullptr),
	length(0) {
	want.freq = 11025;
	want.format = AUDIO_U8;
	want.channels = 1;
	want.samples = 4096;
	want.callback = forwardCallback;
	want.userdata = this;
	want.silence = 0x80;

	SDL_AudioSpec a;

	if ((device = SDL_OpenAudioDevice(NULL, 0, &want, &a, SDL_AUDIO_ALLOW_ANY_CHANGE)) == 0)
	{
		throw std::runtime_error("Warning: failed to open audio device");
	}
	SDL_PauseAudioDevice(device, 0);
}

void AudioDevice::playSound(Sound* s) {
	SDL_LockAudioDevice(device);
	sound = s;
	buffer = s->buffer;
	length = s->length;
	SDL_UnlockAudioDevice(device);
}

void AudioDevice::callback(uint8_t* stream, int len) {
	SDL_memset(stream, 0, len);
	if (sound) {
		int l = (len < length) ? len : length;
		SDL_MixAudioFormat(stream, buffer, AUDIO_U8, l, SDL_MIX_MAXVOLUME);
		buffer += l;
		length -= l;

		if (length == 0)
			sound = nullptr;
	}
}

Sound::Sound(const string& filename) {
	if (SDL_LoadWAV(filename.c_str(), &audio, &buffer, &length) == NULL) {
		throw std::runtime_error("Failed to open wav file");
	}
}

