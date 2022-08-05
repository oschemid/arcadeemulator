#pragma once

#include "cpu.h"
#include "i8080.h"

#include <map>
#include <functional>


using namespace ae;

typedef std::map<const string, std::function<ICpu* ()>> cpusList;

static cpusList cpus
{
	{"i8080", [] { return new cpu::Intel8080(); }},
};

ICpu* ae::ICpu::create(const string& name) {
	return cpus[name]();
}