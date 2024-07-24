#include "widgets.h"
#include "imgui_internal.h"
#include "emulator.h"
#include "ui.h"
#include "ximgui.h"

using namespace aos;
using namespace ae;


MenuWidget::MenuWidget(aos::database::Driver** driver)
    : _driverlaunched{ driver }
{
}

void MenuWidget::draw(aos::ui::Manager* ui)
{
    drawBackground(ui);
    drawSidebar(ui);
    drawGames(ui);
    if (_driverselected) {
        drawGame(ui);
    }
}

void MenuWidget::drawBackground(aos::ui::Manager* ui)
{
    auto displaysize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(displaysize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(100, 100, 100, 255));
    ImGui::Begin("#MenuBG", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration);
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void MenuWidget::drawSidebar(aos::ui::Manager* ui)
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    auto displaysize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(ImVec2(200, displaysize.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(50, 50, 50, 255));
    ImGui::Begin("#MenuSidebar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

    for (auto category : aos::database::listPlatformCategories() )
    {
        ImGui::Text(aos::database::get(category).c_str());
        for (auto& iter : aos::database::listPlatforms(category))
        {
            string label = string("  ") + (iter.second->name());
            if (ImGui::Selectable(label.c_str(), _platformselected == iter.second)) {
                _platformselected = iter.second;
                _driverselected = nullptr;
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void MenuWidget::drawGames(aos::ui::Manager* ui)
{
    ImGui::SetNextWindowPos(ImVec2(200.0f, 0.0f));
    auto displaysize = ImGui::GetIO().DisplaySize;
    auto size = displaysize.x - 600;
    auto nbcolumns = (int)(size / 200);
    ImGui::SetNextWindowSize(ImVec2(size, displaysize.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    ImGui::Begin("#MenuGames", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

    if (_platformselected) {
        ImGui::PushFont(ui->fontTitle());
        ImGui::Text(_platformselected->name().c_str());
        ImGui::PopFont();
        ImGui::Columns(nbcolumns, 0, false);
        for (auto& driver : aos::database::listDrivers(*_platformselected))
        {
            drawTile(ui, *driver.second);
            ImGui::NextColumn();
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::End();
}

void MenuWidget::drawTile(aos::ui::Manager* ui, database::Driver& driver) {
    const string gameid = driver.id();
    const string title = driver.name();
    const uint8_t size = 200;
    ImGui::BeginChild(gameid.c_str(), ImVec2(size, size));
    ImVec2 topleft = ImGui::GetCursorScreenPos();

    bool hovered = false;
    if (_driverselected)
        hovered = (_driverselected == &driver);
    bool settings = false;
    string idn = "##select " + gameid;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    VkDescriptorSet ds{ 0 };

    string filename = ""; // game.cover();
    if (!filename.empty())
        ds = static_cast<VkDescriptorSet>(ui->loadTexture(filename));

    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::Selectable(idn.c_str(), hovered, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(200, 200))) {
        _driverselected = &driver;
        if (ImGui::IsMouseDoubleClicked(0))
            *_driverlaunched = &driver;
    }

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

void MenuWidget::drawGame(aos::ui::Manager* ui)
{
    auto displaysize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(displaysize.x-400, 10.0f));
    ImGui::SetNextWindowSize(ImVec2(390, displaysize.y-20));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(50, 50, 50, 255));
    ImGui::Begin("#MenuGame", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration );
    ImGui::Text(_driverselected->name().c_str());
    ImGui::TextWrapped(_driverselected->overview().c_str());
    auto version = _driverselected->version();
    auto versions = _driverselected->versions();
    auto& emul = _driverselected->emulator();
    json& settings = emul.settings;

    ImGui::Text("Version");
    if (ImGui::BeginCombo("##MenuGameVersion", settings["name"].template get<string>().c_str()))
    {
        for(auto& it : versions)
        {
            bool is_selected = (version == it.first);
            if (ImGui::Selectable(it.second.c_str(), is_selected))
            {
                _driverselected->version(it.first);
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (settings.contains("settings"))
    {
        for (auto& [_, setting] : settings["settings"].items())
        {
            if (setting.contains("input"))
            {
                string name = setting["name"].template get<std::string>();
                uint8_t current_value = setting["value"].template get<uint8_t>();
                int max_values = setting["input"].size() - 1;
                int temp_value = -1;
                int i = 0;
                std::vector<uint8_t> temp_values( max_values + 1 );
                std::string temp_label;
                for (auto& [label, value] : setting["input"].items())
                {
                    if (value == current_value)
                    {
                        temp_value = i;
                        temp_label = label;
                    }
                    temp_values[i++] = value;
                }
                if (ImGui::SliderInt(name.c_str(), &temp_value, 0, max_values, temp_label.c_str()))
                {
                    setting["value"] = temp_values[temp_value];
                }
            }
        }
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::End();
}
