#include "display.h"
#include <SDL2/SDL.h>
#include "../ui/ui.h"

using namespace ae;

Layout::Layout() :
	_texture(nullptr) {
}
Layout::~Layout() {
	if (_texture)
		SDL_DestroyTexture(_texture);
}

bool Layout::setSize(const uint16_t w, const uint16_t h) {
	_width = w;
	_height = h;
	return true;
}

bool Layout::setZones(zones z) {
	_zones = z;
	return true;
}

bool Layout::init() {
	SDL_Renderer* renderer = ae::ui::getRenderer();
	_texture = SDL_CreateTexture(renderer,
								 SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_TARGET,
								 _width, _height);
	SDL_SetRenderTarget(renderer, _texture);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	for (auto z : _zones) {
		rgb_t color = z.first;
		rect_t rect = z.second;
		SDL_Rect r;
		r.x = rect.left;
		r.y = rect.top;
		r.h = rect.bottom - rect.top;
		r.w = rect.right - rect.left;
		SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, 0);
		SDL_RenderFillRect(renderer, &r);
	}
	SDL_SetRenderTarget(renderer, NULL);
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_MOD);
	return true;
}

bool Layout::update(const SDL_Rect rect) {
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_MOD);
	SDL_RenderCopy(ae::ui::getRenderer(), _texture, NULL, &rect);
	return true;
}

Layout* Layout::create() {
	return new Layout();
}
