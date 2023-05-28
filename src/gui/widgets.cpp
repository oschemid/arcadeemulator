#include "widgets.h"
#include "imgui_internal.h"
#include "emulator.h"


using namespace aos;
using namespace ae;


ConsolesSidebar::ConsolesSidebar(library::Consoles& consoles) :
    ae::gui::widgets::Sidebar{ 200 },
    _consoles{ consoles },
    _selected{ "" }
{
}

void ConsolesSidebar::drawContents() {
    ImGui::Text("Console");

    for (auto& iter : _consoles)
    {
        string label = string("  ") + (iter.second->title);
        if (ImGui::Selectable(label.c_str(), _selected == iter.first)) {
            _selected = iter.first;
        }
    }
}

GameSelection::GameSelection() :
    _console(nullptr),
    _selected(nullptr)
{
}

void GameSelection::filterConsole(library::Console* console)
{
    _console = console;
}

void GameSelection::draw(ae::gui::GuiManager* gui)
{
    ImGui::SetNextWindowPos(ImVec2(200.0f, 0.0f));
    auto displaysize = ImGui::GetIO().DisplaySize;
    auto size = displaysize.x - 200;
    auto nbcolumns = (int)(size / 200);
    ImGui::SetNextWindowSize(ImVec2(size, displaysize.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(100, 100, 100, 255));
    ImGui::Begin("Panel2", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);

    if (_console) {
        ImGui::PushFont(gui->fontTitle());
        ImGui::Text(_console->title.c_str());
        ImGui::PopFont();
        ImGui::Columns(nbcolumns, 0, false);
        for (auto& game : _console->games) {
            ImTextureID id{ 0 };
            //string filename = game.cover();
            //if (!filename.empty())
            //    id = gui->loadTexture(filename);
            drawTile(game, static_cast<VkDescriptorSet>(id));
            ImGui::NextColumn();
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::End();
}

void GameSelection::drawTile(library::Game& game, VkDescriptorSet ds) {
    const string gameid = game.name();
    const string title = game.driver().name;
    const uint8_t size = 200;
    ImGui::BeginChild(gameid.c_str(), ImVec2(size, size));
    ImVec2 topleft = ImGui::GetCursorScreenPos();

    static string hovered = "";
    string idn = "##select " + gameid;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::Selectable(idn.c_str(), gameid == hovered, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(200, 200))) {
    }
    bool s = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped);

    // Image
    const uint8_t imagesize = 100;
    ImVec2 image_topleft = ImVec2(topleft.x + size / 2 - imagesize / 2, topleft.y + imagesize / 4);
    ImVec2 image_bottomright = ImVec2(image_topleft.x + imagesize, image_topleft.y + imagesize);
    if (s) {
        hovered = gameid;
        ImGui::SetCursorScreenPos(ImVec2(topleft.x + 40, image_bottomright.y + 20));
        if (ImGui::ArrowButton("Run", ImGuiDir_Right)) {
            _selected = &game;
        }
        ImGui::SetItemAllowOverlap();
        ImGui::SetCursorScreenPos(ImVec2(topleft.x + 120, image_bottomright.y + 20));
        if (ImGui::Button("Config", ImVec2(40, 40))) {
            ImGui::OpenPopup(("Settings " + gameid).c_str());
        }
        ImGui::SetItemAllowOverlap();
    }
    drawSettings(game);

    if (ds != 0)
        draw_list->AddImage(ds, image_topleft, image_bottomright);
    else
        draw_list->AddRectFilled(image_topleft, image_bottomright, IM_COL32(250, 0, 0, 255));
    auto textWidth = ImGui::CalcTextSize(title.c_str()).x;
    draw_list->AddText(ImVec2(topleft.x + 100 - textWidth / 2, image_bottomright.y), IM_COL32(255, 255, 255, 255), title.c_str());
    ImGui::EndChild();
}

void GameSelection::drawSettings(library::Game& game) {
    if (ImGui::BeginPopupModal(("Settings " + game.name()).c_str())) {
        for (auto& setting : game.driver().configuration.switches) {
            string name = setting.description;
            int value = setting.value;
            string value_str;
            int maxvalue;
            value_str = setting.values[value];
            maxvalue = setting.values.size()-1;
            if (ImGui::SliderInt(name.c_str(), &value, 0, maxvalue, value_str.c_str())) {
                setting.value = value;
            }
        }
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
/*
void GameSelection::drawTile(Game& game, VkDescriptorSet ds) {
    const string title = game.title();
    const uint8_t size = 200;
    ImGui::BeginChild(title.c_str(), ImVec2(size, size));
    ImVec2 topleft = ImGui::GetCursorScreenPos();

    static string selected = "";
    string idn = "##select " + title;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::Selectable(idn.c_str(), title == selected, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(200, 200))) {
    }
    bool s = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped);

    // Image
    const uint8_t imagesize = 100;
    ImVec2 image_topleft = ImVec2(topleft.x + size / 2 - imagesize / 2, topleft.y + imagesize / 4);
    ImVec2 image_bottomright = ImVec2(image_topleft.x + imagesize, image_topleft.y + imagesize);
    if (s) {
        _selected = &game;
        selected = title;
        ImGui::SetCursorScreenPos(ImVec2(topleft.x + 40, image_bottomright.y + 20));
        if (ImGui::ArrowButton("Run", ImGuiDir_Right)) {
            _action = "run";
        }
        ImGui::SetItemAllowOverlap();
        ImGui::SetCursorScreenPos(ImVec2(topleft.x + 120, image_bottomright.y + 20));
        if (ImGui::Button("Config", ImVec2(40, 40))) {
            ImGui::OpenPopup(("Settings "+title).c_str());
            _action = "settings";
        }
        ImGui::SetItemAllowOverlap();
    }
    drawSettings(game);

    if (ds!=0)
        draw_list->AddImage(ds, image_topleft, image_bottomright);
    else
        draw_list->AddRectFilled(image_topleft, image_bottomright, IM_COL32(250, 0, 0, 255));
    auto textWidth = ImGui::CalcTextSize(title.c_str()).x;
    draw_list->AddText(ImVec2(topleft.x + 100 - textWidth / 2, image_bottomright.y), IM_COL32(255, 255, 255, 255), title.c_str());
    ImGui::EndChild();
}

void GameSelection::draw(gui::GuiManager* gui) {
    ImGui::SetNextWindowPos(ImVec2(200.0f, 0.0f));
    auto displaysize = ImGui::GetIO().DisplaySize;
    auto size = displaysize.x - 200;
    auto nbcolumns = (int)(size / 200);
    ImGui::SetNextWindowSize(ImVec2(size, displaysize.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(100, 100, 100, 255));
    ImGui::Begin("Panel2", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::PushFont(gui->fontTitle());
    ImGui::Text(_console.name().c_str());
    ImGui::PopFont();
    ImGui::Columns(nbcolumns, 0, false);
    for (auto& game : _library.games(_filtered)) {
        ImTextureID id{ 0 };
        string filename = game.cover();
        if (!filename.empty())
            id = gui->loadTexture(filename);
        drawTile(game, static_cast<VkDescriptorSet>(id));
        ImGui::NextColumn();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::End();
}
*/
DisplayWidget::DisplayWidget(const string& name, display::RasterDisplay* raster, const float zoom) : _name(name), _raster(raster), _zoom(zoom)
{
}

void DisplayWidget::draw(gui::GuiManager* manager) {
    if (!_textureid) {
        geometry_t geometry = _raster->getGeometry();
        _textureid = manager->createTexture(geometry.width, geometry.height);
        _imagesize = ImVec2(geometry.width * _zoom, geometry.height * _zoom);
    }
    if (_raster->needRefresh()) {
        manager->refreshTexture(_textureid, (uint8_t*)(_raster->getBuffer()));
    }
    ImGui::Begin(_name.c_str());
    ImGui::Image(_textureid, _imagesize);
    ImGui::End();
}