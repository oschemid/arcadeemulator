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
        string label = string("  ") + (iter.second->title());
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
        ImGui::Text(_console->title().c_str());
        ImGui::PopFont();
        ImGui::Columns(nbcolumns, 0, false);
        for (auto& game : _console->games) {
            drawTile(gui, *game.second);
            ImGui::NextColumn();
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::End();
}

void GameSelection::drawTile(ae::gui::GuiManager* gui, library::Game& game) {
    const string gameid = game.name();
    const string title = game.driver().name;
    const uint8_t size = 200;
    ImGui::BeginChild(gameid.c_str(), ImVec2(size, size));
    ImVec2 topleft = ImGui::GetCursorScreenPos();

    static string hovered = "";
    bool settings = false;
    string idn = "##select " + gameid;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    VkDescriptorSet ds{ 0 };

    string filename = game.cover();
    if (!filename.empty())
        ds = static_cast<VkDescriptorSet>(gui->loadTexture(filename));

    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::Selectable(idn.c_str(), gameid == hovered, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(200, 200))) {
        if (ImGui::IsMouseDoubleClicked(0))
            _selected = &game;
    }
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Run"))
            _selected = &game;
        if (game.emulators().size() == 1) {
            ImGui::MenuItem("No version", nullptr, nullptr, false);
        }
        else {
            string selected = game.selected();
            if (ImGui::BeginMenu("Version")) {
                for (auto emulator : game.emulators()) {
                    string v = emulator.first;
                    const bool unavailable = emulator.second.is_unavailable();
                    if (ImGui::MenuItem(v.c_str(), nullptr, v == selected, !unavailable)) {
                        game.select(v);
                    }
                }
                ImGui::EndMenu();
            }
        }
        if (ImGui::MenuItem("Settings...", nullptr, nullptr, game.driver().has_configuration())) {
            settings = true;
        }
        ImGui::EndPopup();
    }
    if (settings) {
        ImGui::OpenPopup(("Settings " + gameid).c_str());
    }
    drawSettings(game);

    // Image
    const uint8_t imagesize = 100;
    ImVec2 image_topleft = ImVec2(topleft.x + size / 2 - imagesize / 2, topleft.y + imagesize / 4);
    ImVec2 image_bottomright = ImVec2(image_topleft.x + imagesize, image_topleft.y + imagesize);

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
            if (!name.empty()) {
                int value = setting.value;
                string value_str;
                int maxvalue;
                value_str = setting.values[value];
                maxvalue = setting.values.size() - 1;
                if (ImGui::SliderInt(name.c_str(), &value, 0, maxvalue, value_str.c_str())) {
                    setting.value = value;
                }
            }
        }
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

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