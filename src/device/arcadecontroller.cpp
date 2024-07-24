#include "device/arcadecontroller.h"


using namespace aos::device;


ArcadeController::Ptr ArcadeController::create()
{
	return std::shared_ptr<ArcadeController>(new ArcadeController());
}