#include "taito8080.h"


using namespace ae::taito8080;


Taito8080::Taito8080() :
	_cpu(nullptr)
{
}

void Taito8080::init()
{
	_cpu = xprocessors::Cpu::create("i8080");
}

void Taito8080::run(ae::gui::RasterDisplay*)
{
}