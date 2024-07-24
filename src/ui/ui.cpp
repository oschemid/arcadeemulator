#include "ui.h"
#include "types.h"
#include "SDL2/SDL.h"
#include "imgui_impl_sdl2.h"


using namespace aos::ui;
using aos::string;
using aos::uint16_t;


void aos::ui::init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
		throw std::runtime_error("SDL could not be initialized: " + string(SDL_GetError()));
	}
}

SDL_Window* createWindow(WindowInfo info)
{
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
        throw std::runtime_error("SDL Video is not initialized");

    SDL_Window* window = SDL_CreateWindow(info.title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        0,0,
        SDL_WINDOW_VULKAN|SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_BORDERLESS);

    if (!window) {
        throw std::runtime_error("SDL Window could not be initialized: " + string(SDL_GetError()));
    }
    return window;
}

Manager::Manager(WindowInfo window) :
    _window{ window }
{
}

void Manager::init()
{
    SDL_Window* window = createWindow(_window);
    _engine = new ae::gui::Engine(window);
    _engine->init();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO io = ImGui::GetIO();

    _fontStandard = io.Fonts->AddFontFromFileTTF("assets/hinted-RosaSans-Regular.ttf", 20);
    _fontTitle = io.Fonts->AddFontFromFileTTF("assets/hinted-RosaSans-Regular.ttf", 30);

    _engine->initImGui();
}

bool Manager::processEvent()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            auto a = event.key.keysym;
        }
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            return true;
        // Temporaire
        if ((event.type == SDL_KEYDOWN) && (event.key.keysym.sym==SDLK_ESCAPE))
            return true;
    }
    return false;
}

void Manager::addWidget(const string& name, Widget::Ptr widget)
{
    _widgets.insert({name, std::move(widget)});
}

void Manager::removeWidget(const string& name)
{
    _widgets.erase(name);
}

void Manager::toggleWidget(const string& name)
{
    auto it = _widgets.find(name);
    if (it != _widgets.end())
    {
        auto&& widget = it->second;
        if (widget->is_hidden())
            widget->hide(false);
        else
            widget->hide(true);
    }
}

void Manager::hideWidget(const string& name, const bool hide)
{
    auto it = getWidget(name);
    if (it != _widgets.end())
    {
        it->second->hide(hide);
    }
}
std::map<string, Widget::Ptr>::iterator Manager::getWidget(const string& name)
{
    return _widgets.find(name);
}

void Manager::renderFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Widgets display
    for (auto const& [name, widget] : _widgets)
    {
        widget->tickui();
        if (!widget->is_hidden())
            widget->draw(this);
    }
    // Temporaire
    bool displayed = true;
    ImGui::ShowDemoWindow(&displayed);

    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized)
    {
        _engine->render(draw_data);
    }
}

ImTextureID Manager::loadTexture(const string& filename)
{
    auto entry = _textures.find(filename);
    if (entry == _textures.end())
    {
        ImTextureID id = _engine->createTextureFromFile(filename);
        _textures.insert({ filename, id });
        return id;
    }
    return entry->second;
}

ImTextureID Manager::createTexture(const uint16_t width, const uint16_t height)
{
    ImTextureID id = _engine->createTexture(width, height);
    return id;
}

void Manager::refreshTexture(ImTextureID textureID, const uint8_t* src)
{
    _engine->fillTextureFromBuffer(textureID, src);
}

void Manager::removeTexture(ImTextureID textureID)
{
    _engine->removeTexture(textureID);
}
