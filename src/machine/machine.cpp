#include "machine.h"
#include "spaceinvaders.h"
#include "spacechaser.h"
#include "pacman.h"

#include <map>
#include <functional>


using namespace ae;


static machinesList machines
{
	{"SpaceInvadersMidway", [] { return new machine::SpaceInvadersMidway(); }},
	{"SpaceInvadersTV", [] { return new machine::SpaceInvadersTV(); }},
	{"SpaceChaserCV", [] { return new machine::SpaceChaserCV(); }},
	{"Pacman", [] { return new machine::Pacman(); }}

};

IMachine* IMachine::create(const string& name) {
	return machines[name]();
}

machinesList ae::getMachines() {
	return machines;
}
