#include "gui.h"

#include "imgui_impl_sdl.h"

using namespace ae::gui;


GuiManager::GuiManager(Engine* engine) :
    _engine(engine)
{}

void GuiManager::init() {
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO io = ImGui::GetIO();
    _fontStandard = io.Fonts->AddFontFromFileTTF("assets/hinted-RosaSans-Regular.ttf", 20);
    _fontTitle = io.Fonts->AddFontFromFileTTF("assets/hinted-RosaSans-Regular.ttf", 30);

    _engine->initImGui();
}

bool GuiManager::processEvent() {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            return true;
//        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(_window))
//            return true;
    }
    return false;
}

void GuiManager::addWidget(const string& name, widgets::Widget* widget) {
    _widgets.insert(std::pair<string, widgets::Widget*>(name, widget));
}

void GuiManager::removeWidget(const string& name) {
    _widgets.erase(name);
}

void GuiManager::renderFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    for (auto const& [name, widget] : _widgets)
    {
        if (!widget->is_hidden())
            widget->draw(this);
    }
    ImGui::ShowDemoWindow(nullptr);

    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized)
    {
        _engine->render(draw_data);
    }
}

ImTextureID GuiManager::loadTexture(const string& filename) {
    return _engine->createTextureFromFile(filename);
}

widgets::Sidebar::Sidebar(const uint16_t width) : Widget(), _width(width) {
}

void widgets::Sidebar::draw(gui::GuiManager* gui) {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    auto displaysize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(ImVec2(_width, displaysize.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(50, 50, 50, 255));
    ImGui::Begin("Panel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

    drawContents();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
