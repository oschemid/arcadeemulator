#include "serial.h"
#include "memorymap.h"


using namespace ae::gameboy;

SerialLink::SerialLink() :
	_enabled{ false },
	_count{ 0 },
	_clock{ 0 },
	_sb{ 0 },
	_sc{ 0 }
{}

uint8_t SerialLink::in(const uint8_t io) const
{
	switch (io)
	{
	case 1:
		return _sb;
	case 2:
		return _sc;
	}
	return 0;
}

void SerialLink::out(const uint8_t io, const uint8_t val)
{
	switch (io)
	{
	case 1:
		_sb = val;
		if (_enabled)
			_count = 8;
		break;
	case 2:
		_sc = val;
		if (val & 0x80) {
			_enabled = true;
			_clock = 0;
			_count = 8;
		}
		break;
	}
}

void SerialLink::tick()
{
	if (!_enabled)
		return;
	if (++_clock == 512) {
		_sb <<= 1;
		_sb |= 1;
		_clock = 0;
		if (--_count == 0) {
			_enabled = false;
			_handlerWrite(0xff0f, 8);
		}
	}
}