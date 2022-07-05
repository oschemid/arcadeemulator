#pragma once
#include "types.h"


namespace ae
{
	class IMachine
	{
	public:
		virtual bool init() = 0;
		virtual bool run() = 0;
	};

	IMachine* newMachine(const string&);
}
