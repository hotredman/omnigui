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
    m_current_window_id = 0;
    m_hooked_items.clear();
    m_hooked_order.clear();
    m_active_clicks.clear();

    if (ImGui::GetCurrentContext()) {
        ProcessInputEvents(ImGui::GetIO());
    }
}

void DomContext::EndFrame() {
    if (!m_enabled) return;

    // Append hooked items captured from ItemAdd / ItemInfo to their respective windows
    for (uint32_t id : m_hooked_order) {
        const auto& item = m_hooked_items[id];
        if (item.w <= 0.0f || item.h <= 0.0f) continue;
        if (item.label.empty()) continue; // Skip internal/decorations

        Window* target_win = nullptr;
        for (auto& win : m_doc.windows) {
            if (win.id == item.win_id) {
                target_win = &win;
                break;
            }
        }
        if (!target_win) continue;

        bool already_exists = false;
        for (const auto& el : target_win->elements) {
            if (el.id == id) {
                already_exists = true;
                break;
            }
        }
        if (already_exists) continue;

        Element el;
        el.id = id;
        el.label = item.label;
        el.x = item.x; el.y = item.y; el.w = item.w; el.h = item.h;
        if (item.item_flags & 0x01) { // ImGuiItemFlags_Disabled
            el.disabled = true;
        }

        if (item.flags & ImGuiItemStatusFlags_Checkable) {
            el.type = ElementType::Checkbox;
            el.checked = (item.flags & ImGuiItemStatusFlags_Checked) != 0;
            auto c_it = m_checkbox_values.find(id);
            if (c_it != m_checkbox_values.end()) {
                el.checked = c_it->second;
            }
        } else if (item.flags & ImGuiItemStatusFlags_Openable) {
            el.type = ElementType::TreeNode;
            el.opened = (item.flags & ImGuiItemStatusFlags_Opened) != 0;
        } else if (item.flags & ImGuiItemStatusFlags_Inputable) {
            auto it_str = m_input_strings.find(id);
            if (it_str != m_input_strings.end()) {
                el.type = ElementType::InputText;
                el.value_str = it_str->second;
            } else {
                el.type = ElementType::SliderFloat;
                el.value_num = 0.0f;
                el.min_val = 0.0f;
                el.max_val = 100.0f;
                auto s_it = m_slider_values.find(id);
                if (s_it != m_slider_values.end()) {
                    el.value_num = s_it->second;
                }
            }
        } else {
            el.type = ElementType::Button;
        }

        target_win->elements.push_back(std::move(el));
    }
    m_hooked_items.clear();
    m_hooked_order.clear();

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

Window* DomContext::GetCurrentWindow() {
    for (auto& win : m_doc.windows) {
        if (win.id == m_current_window_id) return &win;
    }
    if (!m_doc.windows.empty()) return &m_doc.windows.back();
    return nullptr;
}

Window* DomContext::FindOrCreateWindow(uint32_t id, const char* title, float x, float y, float w, float h,
                                       float title_bar_h, float menu_bar_h,
                                       bool has_title_bar, bool has_menu_bar,
                                       bool is_popup, bool is_modal, bool collapsed,
                                       float scroll_x, float scroll_y) {
    for (auto& win : m_doc.windows) {
        if (win.id == id) {
            win.x = x; win.y = y; win.w = w; win.h = h;
            win.title_bar_h = title_bar_h;
            win.menu_bar_h = menu_bar_h;
            win.has_title_bar = has_title_bar;
            win.has_menu_bar = has_menu_bar;
            win.is_popup = is_popup;
            win.is_modal = is_modal;
            win.collapsed = collapsed;
            win.scroll_x = scroll_x;
            win.scroll_y = scroll_y;
            m_current_window_id = id;
            return &win;
        }
    }

    Window win{};
    win.id = id;
    win.title = title ? title : "";
    win.x = x; win.y = y; win.w = w; win.h = h;
    win.title_bar_h = title_bar_h;
    win.menu_bar_h = menu_bar_h;
    win.has_title_bar = has_title_bar;
    win.has_menu_bar = has_menu_bar;
    win.is_popup = is_popup;
    win.is_modal = is_modal;
    win.collapsed = collapsed;
    win.scroll_x = scroll_x;
    win.scroll_y = scroll_y;

    m_doc.windows.push_back(std::move(win));
    m_current_window_id = id;
    return &m_doc.windows.back();
}

void DomContext::RecordWindowBegin(uint32_t id, const char* title, float x, float y, float w, float h,
                                   float title_bar_h, float menu_bar_h,
                                   bool has_title_bar, bool has_menu_bar,
                                   bool is_popup, bool is_modal, bool collapsed,
                                   float scroll_x, float scroll_y) {
    if (!m_enabled) return;
    FindOrCreateWindow(id, title, x, y, w, h, title_bar_h, menu_bar_h, has_title_bar, has_menu_bar, is_popup, is_modal, collapsed, scroll_x, scroll_y);
}

void DomContext::RecordWindowEnd() {
    m_current_window_id = 0;
}

void DomContext::OnHookItemAdd(uint32_t win_id, uint32_t id, float x, float y, float w, float h, uint32_t status_flags, uint32_t item_flags) {
    if (!m_enabled) return;
    HookItem item;
    item.id = id;
    item.win_id = win_id;
    item.x = x; item.y = y; item.w = w; item.h = h;
    item.flags = status_flags;
    item.item_flags = item_flags;

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
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::Button;
    el.label = label ? label : "";
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordText(uint32_t id, const char* text, float x, float y) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::Text;
    el.value_str = text ? text : "";
    el.x = x; el.y = y;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordSliderFloat(uint32_t id, const char* label, float val, float min_v, float max_v, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::SliderFloat;
    el.label = label ? label : "";
    el.value_num = val;
    el.min_val = min_v;
    el.max_val = max_v;
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordCheckbox(uint32_t id, const char* label, bool checked, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::Checkbox;
    el.label = label ? label : "";
    el.checked = checked;
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordCanvas(uint32_t id, const char* stream_name, const float* points, size_t count, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::Canvas;
    el.stream_name = stream_name ? stream_name : "";
    el.x = x; el.y = y; el.w = w; el.h = h;
    if (points && count > 0) {
        el.points.assign(points, points + count);
    }
    win->elements.push_back(std::move(el));
}

void DomContext::RecordRadioButton(uint32_t id, const char* label, bool active, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::RadioButton;
    el.label = label ? label : "";
    el.checked = active;
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordInputText(uint32_t id, const char* label, const char* text, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::InputText;
    el.label = label ? label : "";
    el.value_str = text ? text : "";
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordCombo(uint32_t id, const char* label, const char* preview, bool opened, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::Combo;
    el.label = label ? label : "";
    el.value_str = preview ? preview : "";
    el.opened = opened;
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordProgressBar(uint32_t id, float fraction, const char* overlay, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::ProgressBar;
    el.value_num = fraction;
    el.value_str = overlay ? overlay : "";
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordSeparator(uint32_t id, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::Separator;
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
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
        if (evt.type == "input") {
            m_input_strings[evt.id] = evt.value_str;
        } else if (evt.type == "slider") {
            m_slider_values[evt.id] = evt.value_num;
        } else if (evt.type == "checkbox") {
            m_checkbox_values[evt.id] = evt.checked;
        } else if (evt.type == "click") {
            m_active_clicks.insert(evt.id);
        }
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

bool DomContext::ConsumeInputText(uint32_t id, std::string& out_str) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_input_strings.find(id);
    if (it != m_input_strings.end()) {
        out_str = it->second;
        m_input_strings.erase(it);
        return true;
    }
    return false;
}

void DomContext::ProcessInputEvents(ImGuiIO& io) {
    std::vector<BrowserEvent> events_to_process;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_pending_events.empty()) {
            events_to_process.push_back(m_pending_events.front());
            m_pending_events.pop();
        }
    }

    for (const auto& evt : events_to_process) {
        if (evt.type == "mouse_move") {
            io.AddMousePosEvent(evt.x, evt.y);
        } else if (evt.type == "mouse_down") {
            io.AddMousePosEvent(evt.x, evt.y);
            io.AddMouseButtonEvent(evt.button, true);
        } else if (evt.type == "mouse_up") {
            io.AddMousePosEvent(evt.x, evt.y);
            io.AddMouseButtonEvent(evt.button, false);
        } else if (evt.type == "mouse_wheel") {
            io.AddMouseWheelEvent(evt.dx, evt.dy);
        } else if (evt.type == "key_down") {
            io.AddKeyEvent(static_cast<ImGuiKey>(evt.key), true);
        } else if (evt.type == "key_up") {
            io.AddKeyEvent(static_cast<ImGuiKey>(evt.key), false);
        } else if (evt.type == "char") {
            if (!evt.value_str.empty()) {
                io.AddInputCharactersUTF8(evt.value_str.c_str());
            }
        } else if (evt.type == "click") {
            m_active_clicks.insert(evt.id);
            if (evt.id != 0 && ImGui::GetCurrentContext()) {
                ImGuiContext& g = *ImGui::GetCurrentContext();
                g.NavNextActivateId = evt.id;
                g.NavNextActivateFlags = ImGuiActivateFlags_PreferInput;
            }
            if (evt.x > 0.0f || evt.y > 0.0f) {
                io.AddMousePosEvent(evt.x, evt.y);
                if (evt.id == 0) {
                    io.AddMouseButtonEvent(0, true);
                    io.AddMouseButtonEvent(0, false);
                }
            }
        } else if (evt.type == "slider") {
            m_slider_values[evt.id] = evt.value_num;
        } else if (evt.type == "checkbox") {
            m_checkbox_values[evt.id] = evt.checked;
        } else if (evt.type == "input") {
            m_input_strings[evt.id] = evt.value_str;
        } else if (evt.type == "radio") {
            m_checkbox_values[evt.id] = true;
            m_active_clicks.insert(evt.id);
        }
    }
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
