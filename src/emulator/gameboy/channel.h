#pragma once
#include "types.h"


namespace aos::gameboy {
	class Channel {
	protected:
		bool _enabled;
		bool _dacenabled;

		// Length
		bool _lengthEnable;
		uint16_t _lengthMax;
		uint16_t _lengthInit;
		uint16_t _lengthCounter;

	public:
		Channel(const uint16_t);

		virtual void setRegister0(const uint8_t) {}
		virtual void setRegister1(const uint8_t);
		virtual void setRegister2(const uint8_t) {}
		virtual void setRegister3(const uint8_t) {}
		virtual void setRegister4(const uint8_t);

		void lengthTick();

		virtual void trigger();
		virtual void tick() = 0;

		virtual float sample() = 0;
	};

	class SquareChannel : public Channel {
	protected:
		bool _sweepAvailable;

		static const uint8_t _dutyPeriod[4];
		uint8_t _duty;
		uint8_t _dutySequence;

//		bool _sweepEnable;
//		uint8_t _sweepPeriodInit;
//		uint8_t _sweepPeriod;
//		bool _sweepDecrease;
//		uint8_t _sweepShift;
//		uint8_t _sweepShadow;
		
		uint16_t _frequencyInit;
		uint16_t _frequencyCounter;

		uint8_t _envelopeVolume;
		bool _envelopeDecrease;
		uint8_t _envelopeInit;
		uint8_t _envelopeCounter;

		uint8_t _currentVolume;


	public:
		SquareChannel(const bool);

		void setRegister0(const uint8_t) override;
		void setRegister1(const uint8_t) override;
		void setRegister2(const uint8_t) override;
		void setRegister3(const uint8_t) override;
		void setRegister4(const uint8_t) override;

		void trigger() override;

		void envelopeTick();

		void tick() override;
		float sample() override;
	};

	class WaveChannel : public Channel {
	protected:
		uint16_t _frequencyInit;
		uint16_t _frequencyCounter;

		uint8_t _volumeInit;
		uint8_t _waveIndex;
		uint8_t _wave[32];

	public:
		WaveChannel();

		void setRegister0(const uint8_t) override;
		void setRegister1(const uint8_t) override;
		void setRegister2(const uint8_t) override;
		void setRegister3(const uint8_t) override;
		void setRegister4(const uint8_t) override;
		void setRegisterWave(const uint8_t, const uint8_t);

		void trigger() override;

		void tick() override;
		float sample() override;
	};

	class NoiseChannel : public Channel {
	protected:
		uint16_t _lfsr;
		bool _width;
		uint8_t _divisor;
		uint8_t _shift;
		uint16_t _timer;

		static const uint8_t _divisors[8];

		uint8_t _envelopeVolume;
		bool _envelopeDecrease;
		uint8_t _envelopeInit;
		uint8_t _envelopeCounter;

	public:
		NoiseChannel();

		void setRegister2(const uint8_t) override;
		void setRegister3(const uint8_t) override;

		void trigger() override;

		void envelopeTick();

		void tick() override;
		float sample() override;
	};
}