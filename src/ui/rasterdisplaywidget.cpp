#include "ui/rasterdisplaywidget.h"


namespace aos::ui
{
	RasterDisplayWidget::RasterDisplayWidget(const geometry_t geometry) :
		RasterDisplay{ geometry },
		Widget{},
		_zoom{ 2. },
		_textureid{ nullptr }
	{
		if (geometry.rotation == geometry_t::rotation_t::ROT90)
			_imagesize = ImVec2(geometry.height * _zoom, geometry.width * _zoom);
		else
			_imagesize = ImVec2(geometry.width * _zoom, geometry.height * _zoom);
	}

	void RasterDisplayWidget::draw(aos::ui::Manager* ui) {
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		auto displaysize = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowSize(displaysize);
		if (!_textureid) {
			if (_geometry.rotation == geometry_t::rotation_t::ROT90)
				_textureid = ui->createTexture(_geometry.height, _geometry.width);
			else
				_textureid = ui->createTexture(_geometry.width, _geometry.height);
		}
		if (_refresh) {
			ui->refreshTexture(_textureid, (uint8_t*)(_buffers[1 - _current]));
			_refresh = false;
		}
		ImGui::Begin("#display", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
		auto initialCursorPos = ImGui::GetCursorPos();
		auto centralizedCursorpos = ImVec2(int((displaysize.x - _imagesize.x) * 0.5f), int((displaysize.y - _imagesize.y) * 0.5f));
		ImGui::SetCursorPos(centralizedCursorpos);
		ImGui::Image(_textureid, _imagesize);
		ImGui::End();
	}

}