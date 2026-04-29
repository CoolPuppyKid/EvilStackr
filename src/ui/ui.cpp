#include "ui.h"


ImGuiID UI::SetupDockspace() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking |
                                   ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                                   ImGuiWindowFlags_NoNavFocus;

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    g_MainDockspaceId = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(g_MainDockspaceId, ImGui::GetContentRegionAvail());
    ImGui::End();
    return g_MainDockspaceId;
}

bool UI::MatchesVisibleWindowTitle(const char* windowName, const char* expectedTitle) {
    if (!windowName || !expectedTitle) {
        return false;
    }
    const char* idSep = std::strstr(windowName, "###");
    if (!idSep) {
        idSep = std::strstr(windowName, "##");
    }
    const size_t visibleLen = idSep ? static_cast<size_t>(idSep - windowName) : std::strlen(windowName);
    return std::strlen(expectedTitle) == visibleLen &&
           std::strncmp(windowName, expectedTitle, visibleLen) == 0;
}

ImGuiWindow* UI::FindWindowByVisibleTitle(const char* expectedTitle) {
    if (ImGuiWindow* exact = ImGui::FindWindowByName(expectedTitle)) {
        return exact;
    }
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) {
        return nullptr;
    }
    for (ImGuiWindow* window : ctx->Windows) {
        if (!window || (window->Flags & ImGuiWindowFlags_ChildWindow) != 0 || !window->DockNode) {
            continue;
        }
        if (MatchesVisibleWindowTitle(window->Name, expectedTitle)) {
            return window;
        }
    }
    return nullptr;
}

DockTabInteractionState UI::QueryDockTabInteraction(const DockDrawerTarget& target, const char* const* anchorWindows, int anchorCount) {
    DockTabInteractionState out;

    if (ImGuiTabBar* tabBar = target.drawerBranch ? target.drawerBranch->TabBar : nullptr) {
        if (ImGui::IsMouseHoveringRect(tabBar->BarRect.Min, tabBar->BarRect.Max, false)) {
            out.hovered = true;
        }
    }

    for (int i = 0; i < anchorCount; ++i) {
        ImGuiWindow* window = FindWindowByVisibleTitle(anchorWindows[i]);
        if (!window) {
            continue;
        }
        const ImRect tabRect = window->DC.DockTabItemRect;
        if (tabRect.GetWidth() <= 0.0f || tabRect.GetHeight() <= 0.0f) {
            continue;
        }
        if (ImGui::IsMouseHoveringRect(tabRect.Min, tabRect.Max, false)) {
            out.hovered = true;
            break;
        }
    }

    if (out.hovered) {
        ImGuiIO& io = ImGui::GetIO();
        out.doubleClicked = io.MouseClicked[ImGuiMouseButton_Left] &&
                            io.MouseClickedCount[ImGuiMouseButton_Left] >= 2;
        out.clicked = !out.doubleClicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    }
    return out;
}

void UI::QueueDrawerTabFocus(DockDrawerState& state, ImGuiTabBar* tabBar, ImGuiID tabId) {
    if (!tabBar || tabId == 0) {
        return;
    }
    for (int i = 0; i < tabBar->Tabs.Size; ++i) {
        ImGuiTabItem* tab = &tabBar->Tabs[i];
        if (tab->ID != tabId) {
            continue;
        }
        ImGui::TabBarQueueFocus(tabBar, tab);
        state.pendingTabFocusId = tabId;
        return;
    }
}

void UI::ApplyPendingDrawerTabFocus(DockDrawerState& state, ImGuiTabBar* tabBar) {
    if (!tabBar || state.pendingTabFocusId == 0) {
        return;
    }
    for (int i = 0; i < tabBar->Tabs.Size; ++i) {
        ImGuiTabItem* tab = &tabBar->Tabs[i];
        if (tab->ID != state.pendingTabFocusId) {
            continue;
        }
        if (tabBar->SelectedTabId == tab->ID || tabBar->VisibleTabId == tab->ID) {
            state.pendingTabFocusId = 0;
            return;
        }
        ImGui::TabBarQueueFocus(tabBar, tab);
        return;
    }
    state.pendingTabFocusId = 0;
}

void UI::DrawVerticalText(ImDrawList* draw, ImVec2 pos, ImU32 color, const char* text, int maxChars) {
    if (!draw || !text) {
        return;
    }
    char compact[8] = {};
    int write = 0;
    for (int i = 0; text[i] != '\0' && write < maxChars; ++i) {
        if (text[i] == ' ') {
            continue;
        }
        compact[write++] = static_cast<char>(std::toupper(static_cast<unsigned char>(text[i])));
    }
    compact[write] = '\0';
    for (int i = 0; i < write; ++i) {
        char line[2] = {compact[i], '\0'};
        draw->AddText(ImVec2(pos.x, pos.y + i * (ImGui::GetFontSize() - 1.0f)), color, line);
    }
}

void UI::RenderCollapsedSideDockRail(DockDrawerState& state, const DockDrawerTarget& target, DockDrawerSide side, float railWidth, float revealAmount) {
    if (!target.drawerBranch || !target.splitParent) {
        return;
    }
    ImGuiTabBar* tabBar = target.drawerBranch->TabBar;
    if (!tabBar || tabBar->Tabs.Size <= 0) {
        return;
    }

    const float reveal = std::clamp(revealAmount, 0.0f, 1.0f);
    if (reveal <= 0.001f) {
        return;
    }

    const float visibleRailWidth = ImMax(1.0f, railWidth * reveal);
    const bool left = side == DockDrawerSide::Left;

    ImVec2 railPos = target.drawerBranch->Pos;
    railPos.x = left ? (target.drawerBranch->Pos.x + target.drawerBranch->Size.x - visibleRailWidth)
                     : target.drawerBranch->Pos.x;
    ImVec2 railSize(visibleRailWidth, target.drawerBranch->Size.y);

    char railWindowName[64];
    std::snprintf(railWindowName, sizeof(railWindowName), "##DockRail_%c_%08X", left ? 'L' : 'R', target.splitParent->ID);

    ImGuiWindowFlags railFlags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoDocking |
                                 ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoNav |
                                 ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::SetNextWindowPos(railPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(railSize, ImGuiCond_Always);
    if (target.drawerBranch->HostWindow) {
        ImGui::SetNextWindowViewport(target.drawerBranch->HostWindow->ViewportId);
    }
    ImGui::SetNextWindowBgAlpha(0.94f * reveal);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    if (ImGui::Begin(railWindowName, nullptr, railFlags)) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImGuiStyle& style = ImGui::GetStyle();
        const bool tabBarFocused = (tabBar->Flags & ImGuiTabBarFlags_IsFocused) != 0;
        float cursorY = ImGui::GetCursorPosY() + style.FramePadding.y;
        const float slotWidth = ImGui::GetContentRegionAvail().x;

        const ImVec2 windowPos = ImGui::GetWindowPos();
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 windowMax(windowPos.x + windowSize.x, windowPos.y + windowSize.y);
        draw->AddRectFilled(windowPos, windowMax, ImGui::GetColorU32(ImGuiCol_Tab));
        draw->AddRect(windowPos, windowMax, ImGui::GetColorU32(ImGuiCol_Border));

        for (int i = 0; i < tabBar->Tabs.Size; ++i) {
            ImGuiTabItem* tab = &tabBar->Tabs[i];
            const char* tabName = ImGui::TabBarGetTabName(tabBar, tab);
            const bool selected = (tabBar->SelectedTabId == tab->ID) || (tabBar->VisibleTabId == tab->ID);
            const float slotHeight = std::clamp(ImGui::CalcTextSize(tabName).x + 12.0f, 42.0f, 112.0f);

            ImGui::PushID(static_cast<int>(tab->ID));
            ImGui::SetCursorPosY(cursorY);
            ImVec2 slotPos = ImGui::GetCursorScreenPos();
            ImVec2 slotSize(slotWidth, slotHeight);
            if (ImGui::InvisibleButton("##SideTab", slotSize)) {
                QueueDrawerTabFocus(state, tabBar, tab->ID);
                state.collapsed = false;
            }
            const bool hovered = ImGui::IsItemHovered();
            const ImRect slotRect(slotPos, ImVec2(slotPos.x + slotSize.x, slotPos.y + slotSize.y));
            const ImU32 bg = selected
                                 ? ImGui::GetColorU32(tabBarFocused ? ImGuiCol_TabSelected : ImGuiCol_TabDimmedSelected)
                                 : (hovered ? ImGui::GetColorU32(ImGuiCol_TabHovered)
                                            : ImGui::GetColorU32(tabBarFocused ? ImGuiCol_Tab : ImGuiCol_TabDimmed));

            draw->AddRectFilled(slotRect.Min, slotRect.Max, bg, style.TabRounding);
            draw->AddRect(slotRect.Min, slotRect.Max, ImGui::GetColorU32(ImGuiCol_Border), style.TabRounding);

            if (selected) {
                const float accentW = ImMax(1.0f, style.TabBarOverlineSize + 1.0f);
                if (left) {
                    draw->AddRectFilled(ImVec2(slotRect.Max.x - accentW, slotRect.Min.y),
                                        ImVec2(slotRect.Max.x, slotRect.Max.y),
                                        ImGui::GetColorU32(tabBarFocused ? ImGuiCol_TabSelectedOverline : ImGuiCol_TabDimmedSelectedOverline));
                } else {
                    draw->AddRectFilled(ImVec2(slotRect.Min.x, slotRect.Min.y),
                                        ImVec2(slotRect.Min.x + accentW, slotRect.Max.y),
                                        ImGui::GetColorU32(tabBarFocused ? ImGuiCol_TabSelectedOverline : ImGuiCol_TabDimmedSelectedOverline));
                }
            }

            DrawVerticalText(draw,
                             ImVec2(slotRect.Min.x + 4.0f, slotRect.Min.y + 6.0f),
                             ImGui::GetColorU32((selected || hovered) ? ImGuiCol_Text : ImGuiCol_TextDisabled),
                             tabName);

            ImGui::PopID();
            cursorY += slotHeight + 2.0f;
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void UI::RenderCollapsedBottomDockRail(DockDrawerState& state, const DockDrawerTarget& target, float railHeight, float revealAmount) {
    if (!target.drawerBranch || !target.splitParent) {
        return;
    }
    ImGuiTabBar* tabBar = target.drawerBranch->TabBar;
    if (!tabBar || tabBar->Tabs.Size <= 0) {
        return;
    }

    const float reveal = std::clamp(revealAmount, 0.0f, 1.0f);
    if (reveal <= 0.001f) {
        return;
    }

    ImVec2 railPos(target.drawerBranch->Pos.x, target.drawerBranch->Pos.y);
    ImVec2 railSize(target.drawerBranch->Size.x, ImMax(1.0f, railHeight * reveal));

    char railWindowName[64];
    std::snprintf(railWindowName, sizeof(railWindowName), "##DockRail_B_%08X", target.splitParent->ID);

    ImGuiWindowFlags railFlags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoDocking |
                                 ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoNav |
                                 ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::SetNextWindowPos(railPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(railSize, ImGuiCond_Always);
    if (target.drawerBranch->HostWindow) {
        ImGui::SetNextWindowViewport(target.drawerBranch->HostWindow->ViewportId);
    }
    ImGui::SetNextWindowBgAlpha(0.96f * reveal);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    if (ImGui::Begin(railWindowName, nullptr, railFlags)) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImGuiStyle& style = ImGui::GetStyle();
        const bool tabBarFocused = (tabBar->Flags & ImGuiTabBarFlags_IsFocused) != 0;
        float cursorX = ImGui::GetCursorPosX();

        const ImVec2 windowPos = ImGui::GetWindowPos();
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 windowMax(windowPos.x + windowSize.x, windowPos.y + windowSize.y);
        draw->AddRectFilled(windowPos,
                            windowMax,
                            ImGui::GetColorU32(ImGuiCol_Tab),
                            style.WindowRounding,
                            ImDrawFlags_RoundCornersTop);
        draw->AddRect(windowPos,
                      windowMax,
                      ImGui::GetColorU32(ImGuiCol_Border),
                      style.WindowRounding,
                      ImDrawFlags_RoundCornersTop);

        for (int i = 0; i < tabBar->Tabs.Size; ++i) {
            ImGuiTabItem* tab = &tabBar->Tabs[i];
            const char* tabName = ImGui::TabBarGetTabName(tabBar, tab);
            const bool selected = (tabBar->SelectedTabId == tab->ID) || (tabBar->VisibleTabId == tab->ID);
            const float tabWidth = std::clamp(ImGui::CalcTextSize(tabName).x + 26.0f, 58.0f, 124.0f);

            ImGui::PushID(static_cast<int>(tab->ID));
            ImGui::SetCursorPos(ImVec2(cursorX, ImGui::GetCursorPosY()));
            ImVec2 slotPos = ImGui::GetCursorScreenPos();
            ImVec2 slotSize(tabWidth, ImMax(16.0f, railSize.y - 4.0f));
            if (ImGui::InvisibleButton("##BottomTab", slotSize)) {
                QueueDrawerTabFocus(state, tabBar, tab->ID);
                state.collapsed = false;
            }
            const bool hovered = ImGui::IsItemHovered();
            const ImRect slotRect(slotPos, ImVec2(slotPos.x + slotSize.x, slotPos.y + slotSize.y));
            const ImU32 bg = selected
                                 ? ImGui::GetColorU32(tabBarFocused ? ImGuiCol_TabSelected : ImGuiCol_TabDimmedSelected)
                                 : (hovered ? ImGui::GetColorU32(ImGuiCol_TabHovered)
                                            : ImGui::GetColorU32(tabBarFocused ? ImGuiCol_Tab : ImGuiCol_TabDimmed));

            draw->AddRectFilled(slotRect.Min, slotRect.Max, bg, style.FrameRounding, ImDrawFlags_RoundCornersTop);
            draw->AddRect(slotRect.Min, slotRect.Max, ImGui::GetColorU32(ImGuiCol_Border), style.FrameRounding, ImDrawFlags_RoundCornersTop);
            if (selected) {
                const float accentH = ImMax(1.0f, style.TabBarOverlineSize + 1.0f);
                draw->AddRectFilled(ImVec2(slotRect.Min.x + 1.0f, slotRect.Max.y - accentH),
                                    ImVec2(slotRect.Max.x - 1.0f, slotRect.Max.y),
                                    ImGui::GetColorU32(tabBarFocused ? ImGuiCol_TabSelectedOverline : ImGuiCol_TabDimmedSelectedOverline),
                                    style.FrameRounding,
                                    ImDrawFlags_RoundCornersTop);
            }

            const ImVec2 textSize = ImGui::CalcTextSize(tabName);
            const ImVec2 textPos(slotRect.Min.x + (slotRect.GetWidth() - textSize.x) * 0.5f,
                                 slotRect.Min.y + (slotRect.GetHeight() - textSize.y) * 0.5f);
            draw->AddText(textPos,
                          ImGui::GetColorU32((selected || hovered) ? ImGuiCol_Text : ImGuiCol_TextDisabled),
                          tabName);

            ImGui::PopID();
            cursorX += tabWidth + style.ItemSpacing.x;
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}

DockDrawerTarget UI::FindDockDrawerTarget(const char* const* anchorWindows, int anchorCount, DockDrawerSide side) {
    const ImGuiAxis axis = (side == DockDrawerSide::Bottom) ? ImGuiAxis_Y : ImGuiAxis_X;

    for (int anchorIndex = 0; anchorIndex < anchorCount; ++anchorIndex) {
        ImGuiWindow* anchor = FindWindowByVisibleTitle(anchorWindows[anchorIndex]);
        if (!anchor || !anchor->DockNode) {
            continue;
        }

        ImGuiDockNode* source = anchor->DockNode;
        for (ImGuiDockNode* current = source; current && current->ParentNode; current = current->ParentNode) {
            ImGuiDockNode* parent = current->ParentNode;
            if (parent->SplitAxis != axis || !parent->ChildNodes[0] || !parent->ChildNodes[1]) {
                continue;
            }

            ImGuiDockNode* child0 = parent->ChildNodes[0];
            ImGuiDockNode* child1 = parent->ChildNodes[1];
            const bool sourceInChild0 = ImGui::DockNodeIsInHierarchyOf(source, child0);
            const bool sourceInChild1 = ImGui::DockNodeIsInHierarchyOf(source, child1);
            if (!sourceInChild0 && !sourceInChild1) {
                continue;
            }

            ImGuiDockNode* drawerCandidate = sourceInChild0 ? child0 : child1;
            ImGuiDockNode* oppositeCandidate = drawerCandidate == child0 ? child1 : child0;

            bool matchesSide = false;
            if (side == DockDrawerSide::Bottom) {
                matchesSide = drawerCandidate->Pos.y >= oppositeCandidate->Pos.y - 0.5f;
            } else if (side == DockDrawerSide::Left) {
                matchesSide = drawerCandidate->Pos.x <= oppositeCandidate->Pos.x + 0.5f;
            } else {
                matchesSide = drawerCandidate->Pos.x >= oppositeCandidate->Pos.x - 0.5f;
            }

            if (matchesSide) {
                return DockDrawerTarget{parent, drawerCandidate, oppositeCandidate};
            }
        }
    }

    return {};
}

void UI::UpdateDockDrawerAnimation(DockDrawerState& state, const DockDrawerTarget& target, DockDrawerSide side, const char* const* anchorWindows, int anchorCount) {
    if (!target.splitParent || !target.drawerBranch || !target.oppositeBranch ||
        !target.splitParent->ChildNodes[0] || !target.splitParent->ChildNodes[1]) {
        state = {};
        state.openAmount = 1.0f;
        return;
    }

    if (state.activeSplitParentId != target.splitParent->ID) {
        state.activeSplitParentId = target.splitParent->ID;
        state.collapsed = false;
        state.openAmount = 1.0f;
        state.expandedExtent = (side == DockDrawerSide::Bottom) ? target.drawerBranch->Size.y : target.drawerBranch->Size.x;
        state.pendingTabFocusId = 0;
    }

    ImGuiTabBar* tabBar = target.drawerBranch->TabBar;
    ApplyPendingDrawerTabFocus(state, tabBar);

    const float collapsedExtent = side == DockDrawerSide::Bottom ? 24.0f : 20.0f;
    const DockTabInteractionState interaction = QueryDockTabInteraction(target, anchorWindows, anchorCount);
    if (interaction.doubleClicked) {
        if (!state.collapsed) {
            const float liveExtent = (side == DockDrawerSide::Bottom) ? target.drawerBranch->Size.y : target.drawerBranch->Size.x;
            if (liveExtent > 1.0f) {
                state.expandedExtent = liveExtent;
            }
        }
        state.collapsed = !state.collapsed;
    } else if (state.collapsed && interaction.clicked) {
        state.collapsed = false;
    }

    const float totalExtent = side == DockDrawerSide::Bottom ? target.splitParent->Size.y : target.splitParent->Size.x;
    const float minOppositeExtent = side == DockDrawerSide::Bottom ? 96.0f : 220.0f;
    const float maxDrawerExtent = ImMax(collapsedExtent, totalExtent - minOppositeExtent);
    const float expandedMinExtent = ImMin(collapsedExtent + (side == DockDrawerSide::Bottom ? 24.0f : 40.0f), maxDrawerExtent);

    if (!state.collapsed && state.openAmount >= 0.995f) {
        const float liveExtent = side == DockDrawerSide::Bottom ? target.drawerBranch->Size.y : target.drawerBranch->Size.x;
        const float clampedLiveExtent = std::clamp(liveExtent, collapsedExtent, maxDrawerExtent);
        if (clampedLiveExtent > collapsedExtent + 1.0f) {
            state.expandedExtent = clampedLiveExtent;
        }
    }

    if (state.expandedExtent < collapsedExtent + 1.0f) {
        const float defaultRatio = side == DockDrawerSide::Bottom ? 0.30f : 0.22f;
        state.expandedExtent = std::clamp(totalExtent * defaultRatio, expandedMinExtent, maxDrawerExtent);
    } else {
        state.expandedExtent = std::clamp(state.expandedExtent, expandedMinExtent, maxDrawerExtent);
    }

    const float targetOpen = state.collapsed ? 0.0f : 1.0f;
    const float blend = 1.0f - std::exp(-12.0f * ImGui::GetIO().DeltaTime);
    state.openAmount += (targetOpen - state.openAmount) * blend;
    if (std::fabs(state.openAmount - targetOpen) < 0.001f) {
        state.openAmount = targetOpen;
    }

    const float drawerExtent = std::clamp(collapsedExtent + (state.expandedExtent - collapsedExtent) * state.openAmount,
                                          collapsedExtent,
                                          maxDrawerExtent);
    const float oppositeExtent = ImMax(minOppositeExtent, totalExtent - drawerExtent);

    target.splitParent->AuthorityForSize = ImGuiDataAuthority_DockNode;
    target.drawerBranch->AuthorityForSize = ImGuiDataAuthority_DockNode;
    target.oppositeBranch->AuthorityForSize = ImGuiDataAuthority_DockNode;

    if (side == DockDrawerSide::Bottom) {
        target.drawerBranch->Size.y = drawerExtent;
        target.drawerBranch->SizeRef.y = drawerExtent;
        target.oppositeBranch->Size.y = oppositeExtent;
        target.oppositeBranch->SizeRef.y = oppositeExtent;
    } else {
        target.drawerBranch->Size.x = drawerExtent;
        target.drawerBranch->SizeRef.x = drawerExtent;
        target.oppositeBranch->Size.x = oppositeExtent;
        target.oppositeBranch->SizeRef.x = oppositeExtent;
    }

    const float railReveal = std::clamp(1.0f - state.openAmount, 0.0f, 1.0f);
    ImGuiDockNodeFlags desiredFlags = target.drawerBranch->LocalFlags;
    if (railReveal > 0.01f) {
        desiredFlags |= ImGuiDockNodeFlags_HiddenTabBar;
    } else {
        desiredFlags &= ~ImGuiDockNodeFlags_HiddenTabBar;
    }
    if (desiredFlags != target.drawerBranch->LocalFlags) {
        target.drawerBranch->SetLocalFlags(desiredFlags);
    }

    if (railReveal > 0.01f) {
        if (side == DockDrawerSide::Bottom) {
            RenderCollapsedBottomDockRail(state, target, collapsedExtent, railReveal);
        } else {
            RenderCollapsedSideDockRail(state, target, side, collapsedExtent, railReveal);
        }
    }
}

void UI::UpdateDockDrawerAnimations() {
    static DockDrawerState leftState;
    static DockDrawerState rightState;
    static DockDrawerState bottomState;

    static const char* kBottomAnchors[] = {"Project", "Console"};

    UpdateDockDrawerAnimation(leftState,
                              FindDockDrawerTarget(kBottomAnchors, IM_ARRAYSIZE(kBottomAnchors), DockDrawerSide::Bottom),
                              DockDrawerSide::Bottom,
                              kBottomAnchors,
                              IM_ARRAYSIZE(kBottomAnchors));
}

UI::UI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
}

UI::~UI()
{
    ImGui::DestroyContext();
}