#include "tools.h"


namespace aos::tools
{
	Clock::Clock(const uint64_t clockPerMs) :
		_clockPerMs(clockPerMs)
	{
	}

	void Clock::reset()
	{
		_startTime = std::chrono::high_resolution_clock::now();
		_lastThrottle = 0;
		_clock = _clockCompensation = 0;
	}
	bool Clock::tickable()
	{
		auto diff = std::chrono::high_resolution_clock::now() - _startTime;
		uint64_t currentTime = duration_cast<std::chrono::nanoseconds>(diff).count();

		if (currentTime - _lastThrottle < 1000000)
		{
			if (_clock < _clockPerMs + _clockCompensation)
			{
				_clock++;
				return true;
			}
		}
		else
		{
			_clockCompensation += _clockPerMs * (currentTime - _lastThrottle) / 1000000;
			_lastThrottle = currentTime;
		}
		return false;
	}
}