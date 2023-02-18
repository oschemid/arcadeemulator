#include "widgets.h"

using namespace ae;

AppSidebar::AppSidebar(Consoles consoles) :
	gui::widgets::Sidebar(200),
    _consoles(consoles),
    _selected() {
}

void AppSidebar::drawContents() {
    ImGui::Text("Console");

    for (auto& iter : _consoles)
    {
        ae::Console& c = iter.second;
        string label = string("  ") + (c.name());
        if (ImGui::Selectable(label.c_str(), _selected.id() == iter.first)) {
            _selected = c;
        }
    }
}

GameSelection::GameSelection(Library library) :
    _console(),
    _library(library),
    _selected(),
    _filtered("") {
}

void GameSelection::setFiltered(Console console) {
    if (console.id() != _filtered) {
        _filtered = console.id();
        _console = console;
        _selected = Game();
    }
}

void GameSelection::drawTile(const Game& game, VkDescriptorSet ds) {
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
        selected = title;
        ImGui::SetCursorScreenPos(ImVec2(topleft.x + 80, image_bottomright.y + 20));
        if (ImGui::Button("Run", ImVec2(40, 40))) {
            _selected = game;
        }
        ImGui::SetItemAllowOverlap();
    }

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
    ImGui::Begin("Panel2", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

    ImGui::PushFont(gui->fontTitle());
    ImGui::Text(_console.name().c_str());
    ImGui::PopFont();
    ImGui::Columns(nbcolumns, 0, false);
    for (auto& game : _library.games(_filtered)) {
        ImTextureID id = game.coverId();
        if (id == 0) {
            string filename = game.cover();
            if (filename != "") {
                id = gui->loadTexture(filename);
                game.setCoverId(id);
            }
        }
        drawTile(game, static_cast<VkDescriptorSet>(id));
        ImGui::NextColumn();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::End();
}

RasterDisplay::RasterDisplay(const string& name, const ImTextureID id) : _name(name), _textureid(id)
{}

void RasterDisplay::draw(gui::GuiManager*) {
    ImGui::Begin(_name.c_str());
    ImGui::Image(_textureid, ImVec2(224*2, 288*2));
    ImGui::End();
}