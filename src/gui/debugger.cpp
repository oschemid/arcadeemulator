#include "debugger.h"
#include "tilemap.h"


using namespace ae;

const static uint16_t textureWidth = 192;

TileMapWidget::TileMapWidget(const string& name, gui::GuiManager* manager) : _name{ name }, _manager{ manager }
{
}

void TileMapWidget::reset(Emulator* emulator)
{
    _emulator = emulator;
    _palettes = _emulator->getPalettes();
    _current_palette = 0;
    refreshTextures();
    _current_map = _maps.begin()->first;
}

void TileMapWidget::refreshTextures()
{
    for (const auto& map : _maps)
        if (!map.second)
            _manager->removeTexture(map.second);
    _maps.clear();
    for (const auto& tiles : _emulator->getTiles())
        _maps[tiles.first] = createTexture(tiles.second);
    _needRefresh = false;
}

void TileMapWidget::draw(gui::GuiManager* manager) {
    if (_needRefresh)
        refreshTextures();

    ImGui::Begin(_name.c_str());
    ImGui::BeginTable("TileMapTable", 2);
    ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (ImGui::BeginCombo("Palette", std::to_string(_current_palette).c_str()))
    {
        for (uint8_t i = 0; i < _palettes.size(); ++i)
        {
            string label = std::to_string(i);
            if (ImGui::Selectable(label.c_str(), i == _current_palette))
            {
                _current_palette = i;
                _needRefresh = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::TableNextColumn();
    if (ImGui::BeginTabBar("TileMap", ImGuiTabBarFlags_None))
    {
        for (const auto& map : _maps)
        {
            if (ImGui::BeginTabItem(map.first.c_str()))
            {
                ImGui::Image(map.second, ImVec2(textureWidth*2, textureWidth*2));
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::EndTable();
    ImGui::End();
}

ImTextureID TileMapWidget::createTexture(const std::vector<ae::tilemap::Tile> tiles)
{
    const uint8_t size = tiles[0].size();

    ae::tilemap::TileMap map{ textureWidth, textureWidth };
    ae::display::RasterDisplay display{ {textureWidth, textureWidth} };

    uint16_t x = 2;
    uint16_t y = 2;
    display.init();
    for (auto tile : tiles)
    {
        map.drawTile(display, tile, x, y, _palettes[_current_palette], false, false);
        x += size + 4;
        if (x > textureWidth - size)
        {
            y += size + 4;
            x = 2;
        }
    }
    display.refresh();
    ImTextureID textureid = _manager->createTexture(textureWidth, textureWidth);
    _manager->refreshTexture(textureid, (uint8_t*)(display.getBuffer()));
    return textureid;
}
