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

	class DisplayDevice : public Device
	{
	public:
		using SharedPtr = std::shared_ptr<DisplayDevice>;

		struct DisplayRequirements {
			geometry_t geometry;
		};
	};
}