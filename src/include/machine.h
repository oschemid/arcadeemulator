#pragma once
#include "types.h"

#include <map>
#include <functional>

#include "../gui/vulkan/engine.h"

namespace ae
{
	class IParameter
	{
	public:
		virtual const string getName() const = 0;
		virtual const uint8_t getValue() const = 0;
		virtual const uint8_t getMaxValue() const = 0;
		virtual const string getAlias() const = 0;
		virtual bool setValue(const uint8_t) = 0;
	};
}
