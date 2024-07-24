#include "device/amstradkeyboard.h"


using namespace aos::device;


AmstradKeyboard::Ptr AmstradKeyboard::create()
{
	return std::shared_ptr<AmstradKeyboard>(new AmstradKeyboard());
}
