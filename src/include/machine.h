#pragma once
#include "types.h"

#include <map>
#include <functional>


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
	class IMachine
	{
	public:
		virtual const string getID() const = 0;
		virtual const string getName() const = 0;
		virtual const string getDescription() const = 0;
		virtual std::list<ae::IParameter*> getParameters() const = 0;

		virtual bool init() = 0;
		virtual bool run() = 0;

	public:
		static IMachine* create(const string&);
	};

	typedef std::map<const string, std::function<IMachine* ()>> machinesList;

	machinesList getMachines();
}
