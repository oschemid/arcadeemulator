#include "ximgui.h"


namespace xImGui
{
    bool ThreeDotsButton()
    {
        ImGuiWindow* wnd = ImGui::GetCurrentWindow();
        if (wnd->Collapsed)
            return false;
        ImRect title_bar_rect = wnd->TitleBarRect();
        ImGuiStyle& style = ImGui::GetStyle();
        float pad_r = style.FramePadding.x;
        float button_sz = ImGui::GetFontSize();
        if (wnd->HasCloseButton)
        {
            pad_r += button_sz + style.ItemInnerSpacing.x;
        }

        ImVec2 pos = ImVec2(title_bar_rect.Max.x - pad_r - button_sz, title_bar_rect.Min.y + style.FramePadding.y);
        ImRect bb(pos, ImVec2(pos.x + button_sz, pos.y + button_sz));

        auto id = ImGui::GetCurrentWindow()->GetID("#THREEDOTS");
        ImGui::ItemAdd(bb, id);

        bool hovered, held;
        ImGui::PushClipRect(bb.Min + ImVec2(-1.f, -1.f), bb.Max + ImVec2(1.f, 1.f), false);
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_None);
        ImU32 bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
        if (hovered || held)
            wnd->DrawList->AddCircleFilled(ImVec2(bb.GetCenter().x, bb.GetCenter().y - 0.5f), button_sz * 0.5f + 1.0f, bg_col);
        wnd->DrawList->AddCircleFilled(ImVec2(bb.GetCenter().x, bb.GetCenter().y), button_sz / 12.f, text_col);
        wnd->DrawList->AddCircleFilled(ImVec2(bb.GetCenter().x - button_sz / 4.f, bb.GetCenter().y), button_sz / 12.f, text_col);
        wnd->DrawList->AddCircleFilled(ImVec2(bb.GetCenter().x + button_sz / 4.f, bb.GetCenter().y), button_sz / 12.f, text_col);
        ImGui::PopClipRect();
        return pressed;
    }
}