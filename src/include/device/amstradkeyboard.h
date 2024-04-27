#pragma once
#include "device.h"


namespace aos::device
{
	class AmstradKeyboard : public Device
	{
	public:
		using Ptr = std::shared_ptr<AmstradKeyboard>;

	public:
		static Ptr create();

		virtual ~AmstradKeyboard() = default;

		virtual void init() override {}
		
		uint8_t getLine(const uint8_t line) { return (line < 10) ? _lines[line] : 0xff; }

	protected:
		AmstradKeyboard() = default;
		uint8_t _lines[10]{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	};
}