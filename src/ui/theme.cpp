#include "theme.h"

Theme::Theme()
{

}

Theme::~Theme()
{
    
}

void Theme::Apply()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(3.0f, 3.0f);
    style.FramePadding = ImVec2(4.0f, 4.0f);
    style.ItemSpacing = ImVec2(10.0f, 5.0f);
    style.ItemInnerSpacing = ImVec2(2.0f, 2.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 11.0f;
    style.GrabMinSize = 8.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowRounding = 12.0f;
    style.ChildRounding = 12.0f;
    style.FrameRounding = 12.0f;
    style.PopupRounding = 12.0f;
    style.GrabRounding = 12.0f;
    style.ScrollbarSize = 11.0f;
    style.ScrollbarRounding = 10.0f;
    style.TabBorderSize = 1.0f;
    style.TabBarBorderSize = 1.0f;
    style.TabBarOverlineSize = 1.0f;
    style.TabRounding = 10.0f;
    style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ButtonTextAlign = ImVec2(0.50f, 0.50f);
    style.DockingNodeHasCloseButton = true;
    style.DockingSeparatorSize = 0.0f;
}