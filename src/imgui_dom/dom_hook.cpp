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
        ImGuiDom::DomContext::Instance().RecordWindowBegin(
            static_cast<uint32_t>(id),
            window->Name,
            window->Pos.x, window->Pos.y,
            window->Size.x, window->Size.y
        );
        return;
    }

    float w = bb.GetWidth();
    float h = bb.GetHeight();
    if (w <= 0.0f || h <= 0.0f) return;

    uint32_t status_flags = item_data ? item_data->StatusFlags : 0;
    ImGuiDom::DomContext::Instance().OnHookItemAdd(
        static_cast<uint32_t>(id),
        bb.Min.x, bb.Min.y, w, h,
        status_flags
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
