#pragma once
#include "types.h"


namespace aos
{
	class Device
	{
	public:
		using SharedPtr = std::shared_ptr<Device>;

	public:
		virtual ~Device() = default;
		virtual void init() = 0;
	};

	class ControllerDevice
	{
	public:
		using SharedPtr = std::shared_ptr<Device>;

		struct Configuration
		{
			string type;
			string option;
		};
	};
}