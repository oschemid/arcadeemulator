#include "machine.h"
#include "spaceinvaders.h"

#include <map>
#include <functional>


using namespace ae;

typedef std::map<const string, std::function<IMachine* ()>> machinesList;

static machinesList machines
{
	{"SpaceInvaders", [] { return new machine::spaceinvaders("TV"); }}
};



IMachine* ae::newMachine(const string& name) {
	return machines[name]();
}