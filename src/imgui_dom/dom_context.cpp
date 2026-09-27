#include "dom_context.h"
#include "imgui.h"
#include <iostream>

namespace ImGuiDom {

void DomContext::BeginFrame(uint64_t frame_index) {
    if (!m_enabled) return;
    m_current_frame = frame_index;
    m_doc.Clear();
    m_doc.frame_index = frame_index;
    m_has_active_window = false;

    // Pop any processed clicks from previous frame
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_pending_events.empty()) {
        BrowserEvent evt = m_pending_events.front();
        m_pending_events.pop();

        if (evt.type == "click") {
            m_active_clicks.insert(evt.id);
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
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_latest_json = std::move(json);
        m_latest_ready_frame = m_current_frame;
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
    m_doc.windows.push_back(std::move(m_current_window));
    m_current_window = Window{};
    m_has_active_window = false;
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
