#include "dom_context.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <iostream>
#include <cstring>

namespace ImGuiDom {

void DomContext::BeginFrame(uint64_t frame_index) {
    if (!m_enabled) return;

#ifdef IMGUI_ENABLE_TEST_ENGINE
    if (ImGui::GetCurrentContext()) {
        ImGui::GetCurrentContext()->TestEngineHookItems = true;
    }
#endif

    m_current_frame = frame_index;
    m_doc.Clear();
    m_doc.frame_index = frame_index;
    m_has_active_window = false;
    m_hooked_items.clear();
    m_hooked_order.clear();

    // Pop any processed clicks from previous frame
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_pending_events.empty()) {
        BrowserEvent evt = m_pending_events.front();
        m_pending_events.pop();

        if (evt.type == "click") {
            m_active_clicks.insert(evt.id);
            // Programmatically activate in Dear ImGui!
            if (ImGui::GetCurrentContext()) {
                ImGuiContext& g = *ImGui::GetCurrentContext();
                g.NavNextActivateId = evt.id;
                g.NavNextActivateFlags = ImGuiActivateFlags_PreferInput;
            }
        } else if (evt.type == "slider") {
            m_slider_values[evt.id] = evt.value_num;
        } else if (evt.type == "checkbox") {
            m_checkbox_values[evt.id] = evt.checked;
        }
    }
}

void DomContext::EndFrame() {
    if (!m_enabled) return;
    if (m_has_active_window) {
        RecordWindowEnd();
    }

    std::string json = m_doc.ToJson();
    SnapshotCallback cb;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_latest_json = std::move(json);
        m_latest_ready_frame = m_current_frame;
        cb = m_snapshot_callback;
    }
    if (cb) {
        cb(m_latest_json);
    }
}

void DomContext::RecordWindowBegin(uint32_t id, const char* title, float x, float y, float w, float h) {
    if (!m_enabled) return;
    if (m_has_active_window) {
        RecordWindowEnd();
    }
    m_current_window = Window{};
    m_current_window.id = id;
    m_current_window.title = title ? title : "";
    m_current_window.x = x;
    m_current_window.y = y;
    m_current_window.w = w;
    m_current_window.h = h;
    m_has_active_window = true;
}

void DomContext::RecordWindowEnd() {
    if (!m_enabled || !m_has_active_window) return;

    // Append hooked items captured from ItemAdd / ItemInfo
    for (uint32_t id : m_hooked_order) {
        bool already_exists = false;
        for (const auto& el : m_current_window.elements) {
            if (el.id == id) {
                already_exists = true;
                break;
            }
        }
        if (already_exists) continue;

        const auto& item = m_hooked_items[id];
        if (item.w <= 0.0f || item.h <= 0.0f) continue;
        if (item.label.empty()) continue; // Skip internal/decorations

        Element el;
        el.id = id;
        el.label = item.label;
        el.x = item.x; el.y = item.y; el.w = item.w; el.h = item.h;

        if (item.flags & ImGuiItemStatusFlags_Checkable) {
            el.type = ElementType::Checkbox;
            el.checked = (item.flags & ImGuiItemStatusFlags_Checked) != 0;
        } else if (item.flags & ImGuiItemStatusFlags_Inputable) {
            el.type = ElementType::SliderFloat;
            el.value_num = 0.0f;
            el.min_val = 0.0f;
            el.max_val = 100.0f;
            auto s_it = m_slider_values.find(id);
            if (s_it != m_slider_values.end()) {
                el.value_num = s_it->second;
            }
        } else {
            el.type = ElementType::Button;
        }

        m_current_window.elements.push_back(std::move(el));
    }
    m_hooked_items.clear();
    m_hooked_order.clear();

    m_doc.windows.push_back(std::move(m_current_window));
    m_current_window = Window{};
    m_has_active_window = false;
}

void DomContext::OnHookItemAdd(uint32_t id, float x, float y, float w, float h, uint32_t status_flags) {
    if (!m_enabled) return;
    HookItem item;
    item.id = id;
    item.x = x; item.y = y; item.w = w; item.h = h;
    item.flags = status_flags;

    auto it = m_item_labels.find(id);
    if (it != m_item_labels.end()) {
        item.label = it->second;
    }

    if (m_hooked_items.find(id) == m_hooked_items.end()) {
        m_hooked_order.push_back(id);
    }
    m_hooked_items[id] = item;
}

void DomContext::OnHookItemInfo(uint32_t id, const char* label, uint32_t status_flags) {
    if (!m_enabled || label == nullptr) return;

    // Clean label: remove ## suffix
    const char* hash_pos = strstr(label, "##");
    std::string clean_name = (hash_pos != nullptr) ? std::string(label, hash_pos) : std::string(label);

    m_item_labels[id] = clean_name;

    auto it = m_hooked_items.find(id);
    if (it != m_hooked_items.end()) {
        it->second.label = clean_name;
        it->second.flags |= status_flags;
    }
}

const char* DomContext::GetItemLabel(uint32_t id) {
    auto it = m_item_labels.find(id);
    if (it != m_item_labels.end()) {
        return it->second.c_str();
    }
    return "";
}

void DomContext::RecordButton(uint32_t id, const char* label, float x, float y, float w, float h) {
    if (!m_enabled || !m_has_active_window) return;
    Element el;
    el.id = id;
    el.type = ElementType::Button;
    el.label = label ? label : "";
    el.x = x; el.y = y; el.w = w; el.h = h;
    m_current_window.elements.push_back(std::move(el));
}

void DomContext::RecordText(uint32_t id, const char* text, float x, float y) {
    if (!m_enabled || !m_has_active_window) return;
    Element el;
    el.id = id;
    el.type = ElementType::Text;
    el.value_str = text ? text : "";
    el.x = x; el.y = y;
    m_current_window.elements.push_back(std::move(el));
}

void DomContext::RecordSliderFloat(uint32_t id, const char* label, float val, float min_v, float max_v, float x, float y, float w, float h) {
    if (!m_enabled || !m_has_active_window) return;
    Element el;
    el.id = id;
    el.type = ElementType::SliderFloat;
    el.label = label ? label : "";
    el.value_num = val;
    el.min_val = min_v;
    el.max_val = max_v;
    el.x = x; el.y = y; el.w = w; el.h = h;
    m_current_window.elements.push_back(std::move(el));
}

void DomContext::RecordCheckbox(uint32_t id, const char* label, bool checked, float x, float y, float w, float h) {
    if (!m_enabled || !m_has_active_window) return;
    Element el;
    el.id = id;
    el.type = ElementType::Checkbox;
    el.label = label ? label : "";
    el.checked = checked;
    el.x = x; el.y = y; el.w = w; el.h = h;
    m_current_window.elements.push_back(std::move(el));
}

void DomContext::RecordCanvas(uint32_t id, const char* stream_name, const float* points, size_t count, float x, float y, float w, float h) {
    if (!m_enabled || !m_has_active_window) return;
    Element el;
    el.id = id;
    el.type = ElementType::Canvas;
    el.stream_name = stream_name ? stream_name : "";
    el.x = x; el.y = y; el.w = w; el.h = h;
    if (points && count > 0) {
        el.points.assign(points, points + count);
    }
    m_current_window.elements.push_back(std::move(el));
}

void DomContext::SetEventCallback(EventCallback cb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_event_callback = std::move(cb);
}

void DomContext::SetSnapshotCallback(SnapshotCallback cb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_snapshot_callback = std::move(cb);
}

void DomContext::PushBrowserEvent(const BrowserEvent& evt) {
    EventCallback cb;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending_events.push(evt);
        cb = m_event_callback;
    }
    if (cb) {
        cb();
    }
}

bool DomContext::ConsumeClick(uint32_t id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_active_clicks.find(id);
    if (it != m_active_clicks.end()) {
        m_active_clicks.erase(it);
        return true;
    }
    return false;
}

bool DomContext::ConsumeSlider(uint32_t id, float& out_val) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_slider_values.find(id);
    if (it != m_slider_values.end()) {
        out_val = it->second;
        m_slider_values.erase(it);
        return true;
    }
    return false;
}

bool DomContext::ConsumeCheckbox(uint32_t id, bool& out_checked) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_checkbox_values.find(id);
    if (it != m_checkbox_values.end()) {
        out_checked = it->second;
        m_checkbox_values.erase(it);
        return true;
    }
    return false;
}

void DomContext::ProcessInputEvents(ImGuiIO& io) {
    (void)io;
    // Clicks and state changes are dispatched directly via ConsumeClick/ConsumeSlider
}

std::string DomContext::GetLatestJson() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_latest_json;
}

bool DomContext::HasNewSnapshot(uint64_t& inout_last_sent_frame) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_latest_ready_frame > inout_last_sent_frame) {
        inout_last_sent_frame = m_latest_ready_frame;
        return true;
    }
    return false;
}

} // namespace ImGuiDom
