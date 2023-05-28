#pragma once
#include "types.h"


using aos::string;


namespace ae::filemanager
{
	void readRoms(const string&, std::vector<std::pair<uint16_t,string>>, uint8_t*);
}
