#include "imgui.h"
#include "imgui_internal.h"
#include "dom_context.h"
#include <cstring>
#include <string>
#include <iostream>

#ifdef IMGUI_ENABLE_TEST_ENGINE

void ImGuiTestEngineHook_ItemAdd(ImGuiContext* ctx, ImGuiID id, const ImRect& bb, const ImGuiLastItemData* item_data) {
    if (!ImGuiDom::DomContext::Instance().IsEnabled()) return;
    if (ctx == nullptr) return;

    ImGuiWindow* window = ctx->CurrentWindow;
    if (window == nullptr) return;

    // Check if this item is a window registration itself
    if (id == window->ID) {
        if (strncmp(window->Name, "Debug##", 7) == 0) return;
        float title_bar_h = window->TitleBarHeight;
        float menu_bar_h = window->MenuBarHeight;
        bool has_title_bar = !(window->Flags & ImGuiWindowFlags_NoTitleBar);
        bool has_menu_bar = (window->Flags & ImGuiWindowFlags_MenuBar) != 0;
        bool is_popup = (window->Flags & ImGuiWindowFlags_Popup) != 0;
        bool is_modal = (window->Flags & ImGuiWindowFlags_Modal) != 0;
        bool is_tooltip = (window->Flags & ImGuiWindowFlags_Tooltip) != 0;
        bool collapsed = window->Collapsed;

        // Clean window name: remove ## suffix
        const char* hash_pos = strstr(window->Name, "##");
        std::string clean_title = (hash_pos != nullptr) ? std::string(window->Name, hash_pos - window->Name) : std::string(window->Name);
        if (clean_title.empty()) {
            has_title_bar = false;
        }

        ImGuiDom::DomContext::Instance().RecordWindowBegin(
            static_cast<uint32_t>(id),
            clean_title.c_str(),
            window->Pos.x, window->Pos.y,
            window->Size.x, window->Size.y,
            title_bar_h, menu_bar_h,
            has_title_bar, has_menu_bar,
            is_popup, is_modal, is_tooltip, collapsed,
            window->Scroll.x, window->Scroll.y
        );
        return;
    }

    float w = bb.GetWidth();
    float h = bb.GetHeight();
    if (w <= 0.0f || h <= 0.0f) return;

    uint32_t status_flags = item_data ? item_data->StatusFlags : 0;
    uint32_t item_flags = item_data ? item_data->ItemFlags : 0;
    bool is_menu_bar = (window->DC.MenuBarAppending != 0) || (window->DC.NavLayerCurrent == ImGuiNavLayer_Menu);
    bool is_stepper = (ctx->GroupStack.Size > 0) && (w <= 30.0f);

    ImGuiDom::DomContext::Instance().OnHookItemAdd(
        static_cast<uint32_t>(window->ID),
        static_cast<uint32_t>(id),
        bb.Min.x, bb.Min.y, w, h,
        status_flags,
        item_flags,
        is_menu_bar,
        is_stepper
    );
}

void ImGuiTestEngineHook_ItemInfo(ImGuiContext* ctx, ImGuiID id, const char* label, ImGuiItemStatusFlags flags) {
    if (!ImGuiDom::DomContext::Instance().IsEnabled()) return;
    if (ctx == nullptr || label == nullptr) return;

    ImGuiWindow* window = ctx->CurrentWindow;
    if (window != nullptr && id == window->ID) {
        return;
    }

    ImGuiDom::DomContext::Instance().OnHookItemInfo(
        static_cast<uint32_t>(id),
        label,
        static_cast<uint32_t>(flags)
    );
}

void ImGuiTestEngineHook_Log(ImGuiContext* ctx, const char* fmt, ...) {
    (void)ctx;
    (void)fmt;
}

const char* ImGuiTestEngine_FindItemDebugLabel(ImGuiContext* ctx, ImGuiID id) {
    (void)ctx;
    return ImGuiDom::DomContext::Instance().GetItemLabel(static_cast<uint32_t>(id));
}

#endif
