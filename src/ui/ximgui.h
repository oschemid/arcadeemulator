#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include <filesystem>


namespace xImGui
{
    IMGUI_API bool ThreeDotsButton();

    IMGUI_API bool FileDialog(const char* label, std::filesystem::path& path, std::filesystem::path& selected);
}