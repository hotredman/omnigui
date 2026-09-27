#pragma once

#include "dom_tree.h"
#include "dom_context.h"
#include "dom_server.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <algorithm>

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

inline bool RadioButton(const char* label, bool active) {
    if (DomContext::Instance().IsEnabled()) {
        uint32_t id = static_cast<uint32_t>(ImGui::GetID(label));
        bool browser_val = false;
        if (DomContext::Instance().ConsumeCheckbox(id, browser_val)) {
            active = browser_val;
        }
    }
    bool clicked = ImGui::RadioButton(label, active);
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        uint32_t id = static_cast<uint32_t>(ImGui::GetItemID());
        DomContext::Instance().RecordRadioButton(id, label, active, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
        if (DomContext::Instance().ConsumeClick(id)) {
            clicked = true;
        }
    }
    return clicked;
}

inline bool RadioButton(const char* label, int* v, int v_button) {
    bool active = (*v == v_button);
    bool clicked = RadioButton(label, active);
    if (clicked) {
        *v = v_button;
    }
    return clicked;
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

inline bool BeginTable(const char* str_id, int columns_count, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f) {
    bool res = ImGui::BeginTable(str_id, columns_count, flags, outer_size, inner_width);
    if (DomContext::Instance().IsEnabled() && res) {
        uint32_t id = static_cast<uint32_t>(ImGui::GetID(str_id));
        DomContext::Instance().RecordTableBegin(id, str_id, columns_count, static_cast<uint32_t>(flags));
    }
    return res;
}

inline void TableSetupColumn(const char* label, ImGuiTableColumnFlags flags = 0, float init_width_or_weight = 0.0f, ImGuiID user_id = 0) {
    ImGui::TableSetupColumn(label, flags, init_width_or_weight, user_id);
    if (DomContext::Instance().IsEnabled()) {
        DomContext::Instance().RecordTableColumn(label);
    }
}

inline void TableHeadersRow() {
    ImGui::TableHeadersRow();
    if (DomContext::Instance().IsEnabled()) {
        DomContext::Instance().RecordTableHeadersRow();
    }
}

inline void TableNextRow(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f) {
    ImGui::TableNextRow(row_flags, min_row_height);
}

inline bool TableNextColumn() {
    return ImGui::TableNextColumn();
}

inline bool TableSetColumnIndex(int column_n) {
    return ImGui::TableSetColumnIndex(column_n);
}

inline void EndTable() {
    if (DomContext::Instance().IsEnabled()) {
        ImGuiContext* g = ImGui::GetCurrentContext();
        if (g && g->CurrentTable) {
            ImGuiTable* table = g->CurrentTable;
            float x = table->OuterRect.Min.x;
            float y = table->OuterRect.Min.y;
            float w = table->OuterRect.GetWidth();
            float h = (table->RowPosY2 > y) ? (table->RowPosY2 - y) : table->OuterRect.GetHeight();
            DomContext::Instance().RecordTableEnd(x, y, w, h);
        } else {
            ImVec2 p_min = ImGui::GetItemRectMin();
            ImVec2 p_max = ImGui::GetItemRectMax();
            DomContext::Instance().RecordTableEnd(p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
        }
    }
    ImGui::EndTable();
}

inline bool ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1) {
    uint32_t id = 0;
    if (DomContext::Instance().IsEnabled() && current_item) {
        id = static_cast<uint32_t>(ImGui::GetID(label));
        int consumed_idx = -1;
        if (DomContext::Instance().ConsumeListBox(id, consumed_idx)) {
            if (consumed_idx >= 0 && consumed_idx < items_count) {
                *current_item = consumed_idx;
            }
        }
    }
    bool changed = ImGui::ListBox(label, current_item, items, items_count, height_in_items);
    if (DomContext::Instance().IsEnabled() && current_item) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        std::vector<std::string> item_list;
        item_list.reserve(items_count);
        for (int i = 0; i < items_count; ++i) {
            item_list.push_back(items[i] ? items[i] : "");
        }
        DomContext::Instance().RecordListBox(id, label, *current_item, item_list, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

inline bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1) {
    uint32_t id = 0;
    if (DomContext::Instance().IsEnabled() && current_item) {
        id = static_cast<uint32_t>(ImGui::GetID(label));
        int consumed_idx = -1;
        if (DomContext::Instance().ConsumeListBox(id, consumed_idx)) {
            if (consumed_idx >= 0 && consumed_idx < items_count) {
                *current_item = consumed_idx;
            }
        }
    }
    bool changed = ImGui::Combo(label, current_item, items, items_count, popup_max_height_in_items);
    if (DomContext::Instance().IsEnabled() && current_item) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        const char* preview = (*current_item >= 0 && *current_item < items_count) ? items[*current_item] : "";
        DomContext::Instance().RecordCombo(id, label, preview, false, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

inline void ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-1.0f, 0.0f), const char* overlay = nullptr) {
    ImGui::ProgressBar(fraction, size_arg, overlay);
    if (DomContext::Instance().IsEnabled()) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        uint32_t id = static_cast<uint32_t>(ImGui::GetItemID());
        DomContext::Instance().RecordProgressBar(id, fraction, overlay ? overlay : "", p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
}

inline bool CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0) {
    return ImGui::CollapsingHeader(label, flags);
}

inline bool CollapsingHeader(const char* label, bool* p_visible, ImGuiTreeNodeFlags flags = 0) {
    return ImGui::CollapsingHeader(label, p_visible, flags);
}

inline bool TreeNode(const char* label) {
    return ImGui::TreeNode(label);
}

inline void TreePop() {
    ImGui::TreePop();
}

inline bool ColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags = 0) {
    uint32_t id = 0;
    if (DomContext::Instance().IsEnabled() && col) {
        id = static_cast<uint32_t>(ImGui::GetID(label));
        float new_col[3];
        if (DomContext::Instance().ConsumeColor3(id, new_col)) {
            col[0] = new_col[0];
            col[1] = new_col[1];
            col[2] = new_col[2];
        }
    }
    bool changed = ImGui::ColorEdit3(label, col, flags);
    if (DomContext::Instance().IsEnabled() && col) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        DomContext::Instance().RecordColorEdit(id, label, col[0], col[1], col[2], 1.0f, false, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

inline bool ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0) {
    uint32_t id = 0;
    if (DomContext::Instance().IsEnabled() && col) {
        id = static_cast<uint32_t>(ImGui::GetID(label));
        float new_col[4];
        if (DomContext::Instance().ConsumeColor4(id, new_col)) {
            col[0] = new_col[0];
            col[1] = new_col[1];
            col[2] = new_col[2];
            col[3] = new_col[3];
        }
    }
    bool changed = ImGui::ColorEdit4(label, col, flags);
    if (DomContext::Instance().IsEnabled() && col) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        DomContext::Instance().RecordColorEdit(id, label, col[0], col[1], col[2], col[3], true, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

inline bool InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr) {
    uint32_t id = 0;
    if (DomContext::Instance().IsEnabled() && buf && buf_size > 0) {
        id = static_cast<uint32_t>(ImGui::GetID(label));
        std::string new_text;
        if (DomContext::Instance().ConsumeInputText(id, new_text)) {
            size_t copy_len = std::min(new_text.size(), buf_size - 1);
            std::memcpy(buf, new_text.data(), copy_len);
            buf[copy_len] = '\0';
        }
    }
    bool changed = ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
    if (DomContext::Instance().IsEnabled() && buf) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        DomContext::Instance().RecordInputText(id, label, buf, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

inline bool InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr) {
    uint32_t id = 0;
    if (DomContext::Instance().IsEnabled() && buf && buf_size > 0) {
        id = static_cast<uint32_t>(ImGui::GetID(label));
        std::string new_text;
        if (DomContext::Instance().ConsumeInputText(id, new_text)) {
            size_t copy_len = std::min(new_text.size(), buf_size - 1);
            std::memcpy(buf, new_text.data(), copy_len);
            buf[copy_len] = '\0';
        }
    }
    bool changed = ImGui::InputTextMultiline(label, buf, buf_size, size, flags, callback, user_data);
    if (DomContext::Instance().IsEnabled() && buf) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        DomContext::Instance().RecordInputTextMultiline(id, label, buf, p_min.x, p_min.y, p_max.x - p_min.x, p_max.y - p_min.y);
    }
    return changed;
}

} // namespace ImGuiDom
