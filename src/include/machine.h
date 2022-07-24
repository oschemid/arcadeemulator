#pragma once
#include "types.h"

#include <map>
#include <functional>


namespace ae
{
	class IMachine
	{
	public:
		virtual const string getID() const = 0;
		virtual const string getName() const = 0;
		virtual const string getDescription() const = 0;

		virtual bool init() = 0;
		virtual bool run() = 0;
	};

	typedef std::map<const string, std::function<IMachine* ()>> machinesList;

	IMachine* newMachine(const string&);
	machinesList getMachines();
}
