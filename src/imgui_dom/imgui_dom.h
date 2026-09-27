#pragma once

#include "dom_tree.h"
#include "dom_context.h"
#include "dom_server.h"
#include "imgui.h"
#include <cstdarg>
#include <cstdio>

namespace ImGuiDom {

inline bool StartServer(int port = 8080, const std::string& host = "0.0.0.0") {
    return DomServer::Instance().Start(host, port);
}

inline void StopServer() {
    DomServer::Instance().Stop();
}

inline bool IsServerRunning() {
    return DomServer::Instance().IsRunning();
}

inline void BeginFrame(uint64_t frame_index) {
    DomContext::Instance().BeginFrame(frame_index);
}

inline void EndFrame() {
    DomContext::Instance().EndFrame();
}

inline void RecordCanvas(const char* name, const float* points, size_t count, float x, float y, float w, float h) {
    if (!DomContext::Instance().IsEnabled()) return;
    uint32_t id = static_cast<uint32_t>(ImGui::GetID(name));
    DomContext::Instance().RecordCanvas(id, name, points, count, x, y, w, h);
}

// Semantic DOM wrappers over standard Dear ImGui calls
inline bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0) {
    return ImGui::Begin(name, p_open, flags);
}

inline void End() {
    ImGui::End();
}

inline bool Button(const char* label, const ImVec2& size = ImVec2(0, 0)) {
    bool clicked = ImGui::Button(label, size);
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        uint32_t id = static_cast<uint32_t>(ImGui::GetItemID());
        DomContext::Instance().RecordButton(id, label, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
        if (DomContext::Instance().ConsumeClick(id)) {
            clicked = true;
        }
    }
    return clicked;
}

inline bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
    if (DomContext::Instance().IsEnabled()) {
        uint32_t id = static_cast<uint32_t>(ImGui::GetID(label));
        float browser_val = 0.0f;
        if (DomContext::Instance().ConsumeSlider(id, browser_val)) {
            *v = browser_val;
        }
    }
    bool changed = ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        uint32_t id = static_cast<uint32_t>(ImGui::GetItemID());
        DomContext::Instance().RecordSliderFloat(id, label, *v, v_min, v_max, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

inline bool Checkbox(const char* label, bool* v) {
    if (DomContext::Instance().IsEnabled()) {
        uint32_t id = static_cast<uint32_t>(ImGui::GetID(label));
        bool browser_val = false;
        if (DomContext::Instance().ConsumeCheckbox(id, browser_val)) {
            *v = browser_val;
        }
    }
    bool changed = ImGui::Checkbox(label, v);
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        uint32_t id = static_cast<uint32_t>(ImGui::GetItemID());
        DomContext::Instance().RecordCheckbox(id, label, *v, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

inline bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0) {
    if (DomContext::Instance().IsEnabled()) {
        uint32_t id = static_cast<uint32_t>(ImGui::GetID(label));
        float browser_val = 0.0f;
        if (DomContext::Instance().ConsumeSlider(id, browser_val)) {
            *v = static_cast<int>(std::round(browser_val));
        }
    }
    bool changed = ImGui::SliderInt(label, v, v_min, v_max, format, flags);
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        uint32_t id = static_cast<uint32_t>(ImGui::GetItemID());
        DomContext::Instance().RecordSliderFloat(id, label, static_cast<float>(*v), static_cast<float>(v_min), static_cast<float>(v_max), p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

inline void Text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    ImGui::TextUnformatted(buf);
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        DomContext::Instance().RecordText(0, buf, p_min.x, p_min.y);
    }
}

inline void TextColored(const ImVec4& col, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::TextUnformatted(buf);
    ImGui::PopStyleColor();
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        DomContext::Instance().RecordText(0, buf, p_min.x, p_min.y);
    }
}

inline void Separator() {
    ImGui::Separator();
}

inline void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f) {
    ImGui::SameLine(offset_from_start_x, spacing);
}

inline bool BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0) {
    bool res = ImGui::BeginTabBar(str_id, flags);
    if (DomContext::Instance().IsEnabled() && res) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        uint32_t id = static_cast<uint32_t>(ImGui::GetItemID());
        DomContext::Instance().RecordTabBar(id, str_id, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return res;
}

inline void EndTabBar() {
    ImGui::EndTabBar();
}

inline bool BeginTabItem(const char* label, bool* p_open = nullptr, ImGuiTabItemFlags flags = 0) {
    if (DomContext::Instance().IsEnabled()) {
        uint32_t id = static_cast<uint32_t>(ImGui::GetID(label));
        if (DomContext::Instance().ConsumeTabSelect(id)) {
            flags |= ImGuiTabItemFlags_SetSelected;
        }
    }
    bool selected = ImGui::BeginTabItem(label, p_open, flags);
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        uint32_t id = static_cast<uint32_t>(ImGui::GetItemID());
        DomContext::Instance().RecordTabItem(id, label, selected, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return selected;
}

inline void EndTabItem() {
    ImGui::EndTabItem();
}

inline void SetItemTooltip(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    ImGui::SetItemTooltip("%s", buf);
    if (DomContext::Instance().IsEnabled()) {
        DomContext::Instance().SetItemTooltip(buf);
    }
}

} // namespace ImGuiDom
