#include "cpu.h"
#include "i8080.h"
#include "z80.h"

#include <map>
#include <functional>


using namespace ae;

typedef std::map<const string, std::function<Cpu* ()>> cpusList;

static cpusList cpus
{
	{"i8080", [] { return new cpu::Intel8080(); }},
	{"Z80", [] { return new cpu::Z80(); }}
};

Cpu* Cpu::create(const string& name) {
	return cpus[name]();
}

bool Cpu::read(const Cpu::readfn fn) {
	_handlerRead = fn;
	return true;
}
bool Cpu::write(const Cpu::writefn fn) {
	_handlerWrite = fn;
	return true;
}
