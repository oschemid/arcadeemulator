#include "channel.h"


using namespace ae::gameboy;


Channel::Channel(const uint16_t lengthMax) :
	_enabled(false),
	_dacenabled(false),
	_lengthEnable(false),
	_lengthMax(lengthMax),
	_lengthInit(0),
	_lengthCounter(0)
{}

void Channel::setRegister1(const uint8_t data) {
	_lengthInit = data & 0x3f;
	_lengthCounter = _lengthMax - _lengthInit;
}

void Channel::setRegister4(const uint8_t data) {
	_lengthEnable = (data & 0x40)? true : false;
}

void Channel::lengthTick() {
	if (_lengthEnable && _lengthCounter > 0)
		if (--_lengthCounter == 0)
			_enabled = false;
}

void Channel::trigger() {
	_enabled = true;
	if (_lengthCounter == 0)
		_lengthCounter = _lengthMax;
}

SquareChannel::SquareChannel(const bool sweepAvailable) :
	Channel(64),
	_duty(0),
	_dutySequence(0),
	_frequencyInit(0),
	_envelopeInit(0)
	{}

const uint8_t SquareChannel::_dutyPeriod[4] = { 0b00000001, 0b10000001, 0b10000111, 0b01111110 };

void SquareChannel::setRegister0(const uint8_t data) {
	if (!_sweepAvailable)
		return;
	if (data != 0)
		throw std::runtime_error("Unimplemented");
//	_sweepPeriodInit = (data & 0x70) >> 4;
//	_sweepShift = data & 0x07;
//	_sweepDecrease = (data & 0x08) ? true : false;
}

void SquareChannel::setRegister1(const uint8_t data) {
	Channel::setRegister1(data);
	_duty = data >> 6;
}

void SquareChannel::setRegister2(const uint8_t data) {
	if (data & 0xf8) {
		_dacenabled = true;
	}
	else {
		_dacenabled = false;
		_enabled = false;
	}
	_envelopeVolume = (data & 0xf0) >> 4;
	_envelopeDecrease = (data & 0x08) ? false : true;
	_envelopeInit = (data & 0x07);
}

void SquareChannel::setRegister3(const uint8_t data) {
	_frequencyInit = (_frequencyInit & 0x700) | data;
}

void SquareChannel::setRegister4(const uint8_t data) {
	Channel::setRegister4(data);
	_frequencyInit = ((data & 7) << 8) | (_frequencyInit & 0xff);
}

void SquareChannel::tick() {
	if (--_frequencyCounter == 0) {
		_frequencyCounter = (2048 - _frequencyInit) * 4;
		if (++_dutySequence == 8)
			_dutySequence = 0;
	}
}

void SquareChannel::envelopeTick() {
	if (_envelopeInit > 0) {
		if (--_envelopeCounter == 0) {
			_envelopeCounter = _envelopeInit;
			if (_envelopeDecrease) {
				if (_envelopeVolume > 0)
					_envelopeVolume--;
			}
			else {
				if (_envelopeVolume < 0x0f)
					_envelopeVolume++;
			}
		}
	}
}

float SquareChannel::sample() {
	if ((!_enabled)||(!_dacenabled))
		return 0.;
	float res = float(_envelopeVolume) / float(0xf);
	return (_dutyPeriod[_duty] & (128 >> _dutySequence)) ? res : -res;
}

void SquareChannel::trigger() {
	Channel::trigger();

	_frequencyCounter = (2048 - _frequencyInit) * 4;
	_envelopeCounter = _envelopeInit;
}


WaveChannel::WaveChannel() :
	Channel(256) 
{}

void WaveChannel::setRegister0(const uint8_t data) {
	_dacenabled = (data & 0x80) ? true : false;
}

void WaveChannel::setRegister1(const uint8_t data) {
	_lengthInit = data;
	_lengthCounter = _lengthMax - _lengthInit;
}

void WaveChannel::setRegister2(const uint8_t data) {
	_volumeInit = (data >> 5) & 0x3;
}

void WaveChannel::setRegister3(const uint8_t data) {
	_frequencyInit = (_frequencyInit & 0x700) | data;
}

void WaveChannel::setRegister4(const uint8_t data) {
	Channel::setRegister4(data);
	_frequencyInit = ((data & 7) << 8) | (_frequencyInit & 0xff);
}

void WaveChannel::setRegisterWave(const uint8_t index, const uint8_t data) {
	_wave[index << 1] = data >> 4;
	_wave[(index << 1) + 1] = data & 0xf;
}

void WaveChannel::trigger() {
	Channel::trigger();
	_waveIndex = 0;
	_frequencyCounter = (2048 - _frequencyInit) * 2;
}

void WaveChannel::tick() {
	if (--_frequencyCounter == 0) {
		_frequencyCounter = (2048 - _frequencyInit) * 2;
		if (++_waveIndex == 32)
			_waveIndex = 0;
	}
}

float WaveChannel::sample() {
	if (_volumeInit == 0)
		return 0.;
	float res = float(_wave[_waveIndex] >> (_volumeInit - 1)) / float(0xf);
	return (res * 2.) - 1.;
}

NoiseChannel::NoiseChannel() : 
	Channel(64)
{}

const uint8_t NoiseChannel::_divisors[] = { 8, 16, 32, 48, 64, 80, 96, 112 };

void NoiseChannel::setRegister2(const uint8_t data) {
	if (data & 0xf8) {
		_dacenabled = true;
	}
	else {
		_dacenabled = false;
		_enabled = false;
	}
	_envelopeVolume = (data & 0xf0) >> 4;
	_envelopeDecrease = (data & 0x08) ? false : true;
	_envelopeInit = (data & 0x07);
}

void NoiseChannel::setRegister3(const uint8_t data) {
	_divisor = data & 0x7;
	_width = (data & 0x8) == 0x8;
	_shift = (data >> 4) & 0xF;
}

void NoiseChannel::envelopeTick() {
	if (_envelopeInit > 0) {
		if (--_envelopeCounter == 0) {
			_envelopeCounter = _envelopeInit;
			if (_envelopeDecrease) {
				if (_envelopeVolume > 0)
					_envelopeVolume--;
			}
			else {
				if (_envelopeVolume < 0x0f)
					_envelopeVolume++;
			}
		}
	}
}

void NoiseChannel::trigger() {
	Channel::trigger();
	_lfsr = 0x7fff;
	_timer = _divisors[_divisor] << _shift;
}

void NoiseChannel::tick() {
	if (--_timer == 0) {
		_timer = _divisors[_divisor] << _shift;

		uint8_t lfsr_xor = (_lfsr & 0x01) ^ ((_lfsr >> 1) & 0x01);
		_lfsr >>= 1;
		_lfsr |= lfsr_xor << 14;
		if (_width) {
			_lfsr &= ~0X40;
			_lfsr |= lfsr_xor << 6;
		}
	}
}

float NoiseChannel::sample() {
	if ((!_enabled) || (!_dacenabled))
		return 0.;
	float res = float(_envelopeVolume) / float(0xf);
	return ((_lfsr & 0x01) == 0) ? res : -res;
}