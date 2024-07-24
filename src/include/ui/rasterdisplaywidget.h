#pragma once
#include "ui.h"
#include "device/rasterdisplay.h"


namespace aos::ui
{ 
	class RasterDisplayWidget : public ui::Widget, public device::RasterDisplay
	{
	public:
		RasterDisplayWidget(const geometry_t);
		virtual void draw(ui::Manager*) override;
		virtual void tickui() override {}

	protected:
		ImTextureID _textureid{ nullptr };
		ImVec2 _imagesize;
		float _zoom;
	};
}