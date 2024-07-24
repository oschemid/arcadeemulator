#include "ximgui.h"


bool xImGui::FileDialog(const char* label, std::filesystem::path& path, std::filesystem::path& selected)
{
	bool isselected = false;
	ImGui::SetNextWindowSize(ImVec2(740.0f, 410.0f));
	ImGui::Begin(label, nullptr, ImGuiWindowFlags_NoResize);

	std::vector<std::filesystem::directory_entry> files;
	std::vector<std::filesystem::directory_entry> folders;
	for (auto& p : std::filesystem::directory_iterator(path))
	{
		if (p.is_directory())
		{
			folders.push_back(p);
		}
		else
		{
			files.push_back(p);
		}
	}

	ImGui::Text(path.string().c_str());
	ImGui::BeginChild("Directories##1", ImVec2(200, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::Selectable("..", false, 0, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
	{
		path = std::filesystem::path(path).parent_path();
	}
	for (int i = 0; i < folders.size(); ++i) {
		if (ImGui::Selectable(folders[i].path().filename().string().c_str(), false, 0, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			path = folders[i].path();
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("Files##1", ImVec2(516, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
	for (int i = 0; i < files.size(); ++i) {
		if (ImGui::Selectable(files[i].path().filename().string().c_str(), false, 0, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			selected = files[i].path();
			isselected = true;
		}
	}
	ImGui::EndChild();
	ImGui::End();
	return isselected;
}