#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

// Idk what half of this does because anenut's gave me it, And I'm too lazy to read it
enum class DockDrawerSide { Left, Right, Bottom };

struct DockDrawerTarget {
    ImGuiDockNode* splitParent = nullptr;
    ImGuiDockNode* drawerBranch = nullptr;
    ImGuiDockNode* oppositeBranch = nullptr;
};

struct DockDrawerState {
    ImGuiID activeSplitParentId = 0;
    bool collapsed = false;
    float openAmount = 1.0f;
    float expandedExtent = 0.0f;
    ImGuiID pendingTabFocusId = 0;
};

struct DockTabInteractionState {
    bool hovered = false;
    bool clicked = false;
    bool doubleClicked = false;
};

class UI
{
private:
    ImGuiID g_MainDockspaceId = 0;
public:
    ImGuiID SetupDockspace();
    void BuildDefaultDockLayout();
    bool MatchesVisibleWindowTitle(const char* windowName, const char* expectedTitle);
    ImGuiWindow* FindWindowByVisibleTitle(const char* expectedTitle);
    DockTabInteractionState QueryDockTabInteraction(const DockDrawerTarget& target, const char* const* anchorWindows, int anchorCount);
    void QueueDrawerTabFocus(DockDrawerState& state, ImGuiTabBar* tabBar, ImGuiID tabId);
    void ApplyPendingDrawerTabFocus(DockDrawerState& state, ImGuiTabBar* tabBar);
    void DrawVerticalText(ImDrawList* draw, ImVec2 pos, ImU32 color, const char* text, int maxChars = 5);
    void RenderCollapsedSideDockRail(DockDrawerState& state, const DockDrawerTarget& target, DockDrawerSide side, float railWidth, float revealAmount);
    void RenderCollapsedBottomDockRail(DockDrawerState& state, const DockDrawerTarget& target, float railHeight, float revealAmount);
    DockDrawerTarget FindDockDrawerTarget(const char* const* anchorWindows, int anchorCount, DockDrawerSide side);
    void UpdateDockDrawerAnimation(DockDrawerState& state, const DockDrawerTarget& target, DockDrawerSide side, const char* const* anchorWindows, int anchorCount);
    void UpdateDockDrawerAnimations();
    UI();
    ~UI();
};