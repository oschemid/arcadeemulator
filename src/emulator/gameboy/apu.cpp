#include "gameboy.h"
#include <iostream>


using namespace ae::gameboy;


Apu::Apu():
	_enabled(false),
	_frameSequencer(0),
	_channel1(true),
	_channel2(false),
	_cycles(0),
	_nbsamples(0),
	_sampleW(0) {}

void Apu::init() {
	want.freq = 48000;
	want.format = AUDIO_F32SYS;
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

void Apu::power(const bool on) {
	if (on) {
		_enabled = true;
		_frameSequencer = 0;
	}
	else {
		_enabled = false;
	}
}

void Apu::frameSequencerTick() {
	if (++_frameSequencer == 8)
		_frameSequencer = 0;
	switch (_frameSequencer) {
	case 2:
	case 6:
		//sweep
	case 0:
	case 4:
		_channel1.lengthTick();
		_channel2.lengthTick();
		_channel4.lengthTick();
		break;
	case 7:
		_channel1.envelopeTick();
		_channel2.envelopeTick();
		_channel4.envelopeTick();
		break;
	}
}

void Apu::restart_channel1() {
	const uint8_t nr10 = _handlerIn(io::nr10);
	const uint8_t nr11 = _handlerIn(io::nr11);
	const uint8_t nr12 = _handlerIn(io::nr12);
	const uint8_t nr13 = _handlerIn(io::nr13);
	const uint8_t nr14 = _handlerIn(io::nr14);

	_channel1.setRegister1(nr11);
	_channel1.setRegister2(nr12);
	_channel1.setRegister3(nr13);
	_channel1.setRegister4(nr14);
	_channel1.trigger();
}

void Apu::restart_channel2() {
	const uint8_t nr21 = _handlerIn(io::nr21);
	const uint8_t nr22 = _handlerIn(io::nr22);
	const uint8_t nr23 = _handlerIn(io::nr23);
	const uint8_t nr24 = _handlerIn(io::nr24);

	_channel2.setRegister1(nr21);
	_channel2.setRegister2(nr22);
	_channel2.setRegister3(nr23);
	_channel2.setRegister4(nr24);
	_channel2.trigger();
}

void Apu::restart_channel3() {
	_channel3.setRegister0(_handlerIn(io::nr30));
	_channel3.setRegister1(_handlerIn(io::nr31));
	_channel3.setRegister2(_handlerIn(io::nr32));
	_channel3.setRegister3(_handlerIn(io::nr33));
	_channel3.setRegister4(_handlerIn(io::nr34));
	for (uint8_t i = 0x30; i < 0x40; ++i)
		_channel3.setRegisterWave(i - 0x30, _handlerIn(i));
	_channel3.trigger();
}

void Apu::restart_channel4() {
	_channel4.setRegister1(_handlerIn(io::nr41));
	_channel4.setRegister2(_handlerIn(io::nr42));
	_channel4.setRegister3(_handlerIn(io::nr43));
	_channel4.setRegister4(_handlerIn(io::nr44));
	_channel4.trigger();
}

void Apu::callback(const uint8_t address, const uint8_t value) {
	if ((!_enabled) && (address != io::nr52))
		return;
	switch (address) {
	case io::div:
		frameSequencerTick();
		break;
	case io::nr50:
		_leftvolume = (value & 0x70) >> 4;
		_rightvolume = value & 0x07;
		break;
	case io::nr51:
		break;
	case io::nr52:
		if (_enabled) {
			if (!(value & 0x80))
				power(false);
		}
		else {
			if (value & 0x80)
				power(true);
		}
		break;
	case io::nr12:
		if ((value&0xf8)==0)
			_channel1.setRegister2(0);
		break;
	case io::nr14:
		if (value & 0x80)
			restart_channel1();
		break;
	case io::nr22:
		if ((value&0xf8)==0)
			_channel2.setRegister2(0);
		break;
	case io::nr24:
		if (value & 0x80)
			restart_channel2();
		break;
	case io::nr30:
		if ((value & 0x80)==0)
			_channel3.setRegister0(0);
		break;
	case io::nr34:
		if (value & 0x80)
			restart_channel3();
		break;
	case io::nr42:
		if ((value&0xf8)==0)
			_channel4.setRegister2(0);
		break;
	case io::nr44:
		if (value & 0x80)
			restart_channel4();
		break;
	}
}

void Apu::tick() {
	if (!_enabled)
		return;

	_cycles++;
	_channel1.tick();
	_channel2.tick();
	_channel3.tick();
	_channel4.tick();

	if (_cycles == 4194304 / 48000) {
		float v = 0;
		float i = 0;
		i = _channel1.sample();
		SDL_MixAudioFormat((Uint8*)&v, (Uint8*)&i, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME);
		i = _channel2.sample();
		SDL_MixAudioFormat((Uint8*)&v, (Uint8*)&i, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME);
		i = _channel3.sample();
		SDL_MixAudioFormat((Uint8*)&v, (Uint8*)&i, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME);
		i = _channel4.sample();
		SDL_MixAudioFormat((Uint8*)&v, (Uint8*)&i, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME);
		_samples[_sampleW++] = v;
		if (_sampleW == 4096) {
			if (SDL_GetQueuedAudioSize(device) > 4096 * 4)
				SDL_Delay(1);
			SDL_QueueAudio(device, _samples, 4096 * 4);
			_sampleW = 0;
		}
		_cycles = 0;
	}

	return;
}
