#pragma once
#include "types.h"
#include "tools.h"
//#include "display.h"
#include "device.h"
#include "memory.h"
//#include "tilemap.h"
#include <vector>
#include <map>
#include <algorithm>


using aos::string;
using aos::geometry_t;
using std::vector;
using std::pair;


namespace aos::emulator
{
	struct SystemInfo
	{
		geometry_t geometry;
	};
	struct DipSwitch
	{
		string name;
		uint8_t value;
		string description;
		vector<string> values;
	};
}