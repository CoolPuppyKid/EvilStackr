#include "modularityTheme.h"

void ModularityTheme::Apply()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    ImVec4 slate = ImVec4(0.11f, 0.12f, 0.19f, 1.00f);
    ImVec4 panel = ImVec4(0.16f, 0.16f, 0.24f, 1.00f);
    ImVec4 overlay = ImVec4(0.10f, 0.11f, 0.17f, 0.98f);
    ImVec4 accent = ImVec4(0.48f, 0.56f, 0.86f, 1.00f);
    ImVec4 accentMuted = ImVec4(0.38f, 0.46f, 0.74f, 1.00f);
    ImVec4 highlight = ImVec4(0.22f, 0.23f, 0.34f, 1.00f);

    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.93f, 0.97f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.62f, 0.70f, 1.00f);
    colors[ImGuiCol_WindowBg] = slate;
    colors[ImGuiCol_ChildBg] = panel;
    colors[ImGuiCol_PopupBg] = overlay;
    colors[ImGuiCol_Border] = ImVec4(0.22f, 0.23f, 0.34f, 0.70f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.10f, 0.16f, 1.00f);
    colors[ImGuiCol_Header] = highlight;
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.28f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.28f, 0.30f, 0.42f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.23f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.30f, 0.42f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.33f, 0.36f, 0.48f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.28f, 0.40f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.34f, 0.46f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.17f, 0.24f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.10f, 0.15f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.16f, 0.24f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.34f, 0.48f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.22f, 0.32f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline] = accent;
    colors[ImGuiCol_TabDimmed] = ImVec4(0.11f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.16f, 0.18f, 0.26f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.22f, 0.23f, 0.34f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.34f, 0.36f, 0.52f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.44f, 0.50f, 0.70f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.26f, 0.36f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.32f, 0.35f, 0.48f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.36f, 0.42f, 0.58f, 1.00f);
    colors[ImGuiCol_CheckMark] = accent;
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = accentMuted;
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.30f, 0.42f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.38f, 0.44f, 0.60f, 0.80f);
    colors[ImGuiCol_ResizeGripActive] = accent;
    colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.45f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.24f);
    colors[ImGuiCol_NavHighlight] = accent;
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.22f, 0.32f, 1.00f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.06f, 0.09f, 0.70f);
}