#include "display.h"
#include "rasterdisplay.h"

using namespace ae;

Display* Display::create() {
	return new display::RasterDisplay();
}
