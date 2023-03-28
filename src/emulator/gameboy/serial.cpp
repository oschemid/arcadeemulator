#include "serial.h"
#include "memorymap.h"


using namespace ae::gameboy;

SerialLink::SerialLink() :
	_enabled{ false },
	_count{ 0 },
	_clock{ 0 },
	_sb{ 0 },
	_sc{ 0 },
	Device{Device::MEM_AVAILABLE}
{}

uint8_t SerialLink::getRegister(const MemoryMap io) const
{
	switch (io)
	{
	case MemoryMap::REGISTER_SB:
		return _sb;
	case MemoryMap::REGISTER_SC:
		return _sc;
	default:
		return 0;
	}
}

void SerialLink::setRegister(const MemoryMap io, const uint8_t val)
{
	switch (io)
	{
	case MemoryMap::REGISTER_SB:
		_sb = val;
		if (_enabled)
			_count = 8;
		break;
	case MemoryMap::REGISTER_SC:
		_sc = val;
		if ((val & 0x81) == 0x81) {
			_enabled = true;
			_clock = 0;
			_count = 8;
		}
		break;
	default:
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
			_handlerWrite(MemoryMap::REGISTER_IF, _handlerRead(MemoryMap::REGISTER_IF)|0x08);
		}
	}
}