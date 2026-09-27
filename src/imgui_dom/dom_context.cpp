#include "dom_context.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <iostream>
#include <cstring>
#include <algorithm>

namespace ImGuiDom {

static inline bool IsCurrentItemDisabled() {
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (!g) return false;
    if (g->CurrentItemFlags & ImGuiItemFlags_Disabled) return true;
    if (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled) return true;
    return false;
}

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
    m_active_tables.clear();
    m_slider_items.clear();

    if (ImGui::GetCurrentContext()) {
        ProcessInputEvents(ImGui::GetIO());
    }
}

void DomContext::MarkItemAsSlider(uint32_t id) {
    m_slider_items.insert(id);
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
        for (auto& el : target_win->elements) {
            if (el.id == id) {
                already_exists = true;
                if ((item.item_flags & ImGuiItemFlags_Disabled) || (item.item_flags & 0x40)) {
                    el.disabled = true;
                }
                break;
            }
        }
        if (already_exists) continue;

        if (item.is_text) {
            Element el;
            el.id = item.id;
            el.type = ElementType::Text;
            el.value_str = item.label;
            el.x = item.x; el.y = item.y; el.w = item.w; el.h = item.h;
            target_win->elements.push_back(std::move(el));
            continue;
        }

        // 1. Skip stepper buttons (- and + inside InputScalar)
        if (item.is_stepper && (item.label == "-" || item.label == "+" || item.label.empty())) {
            continue;
        }

        // 2. Skip internal/invisible items with empty labels or ## prefix that are not interactive
        if (item.label.empty() && !(item.flags & (ImGuiItemStatusFlags_Checkable | ImGuiItemStatusFlags_Inputable | ImGuiItemStatusFlags_Openable))) {
            continue;
        }

        Element el;
        el.id = id;
        el.label = item.label;
        el.x = item.x; el.y = item.y; el.w = item.w; el.h = item.h;
        el.is_menu_bar = item.is_menu_bar;
        if ((item.item_flags & ImGuiItemFlags_Disabled) || (item.item_flags & 0x40)) {
            el.disabled = true;
        }

        if (item.is_menu_bar) {
            el.type = ElementType::MenuItem;
        } else if (item.flags & ImGuiItemStatusFlags_Checkable) {
            el.type = ElementType::Checkbox;
            el.checked = (item.flags & ImGuiItemStatusFlags_Checked) != 0;
            auto c_it = m_checkbox_values.find(id);
            if (c_it != m_checkbox_values.end()) {
                el.checked = c_it->second;
            }
        } else if (item.flags & ImGuiItemStatusFlags_Openable) {
            bool is_header = (item.w >= target_win->w - 35.0f);
            if (is_header) {
                el.type = ElementType::CollapsingHeader;
            } else {
                el.type = ElementType::TreeNode;
            }
            el.opened = (item.flags & ImGuiItemStatusFlags_Opened) != 0;
        } else if (item.flags & ImGuiItemStatusFlags_Inputable) {
            bool is_slider = (m_slider_items.find(id) != m_slider_items.end()) ||
                             (item.label.find("slider") != std::string::npos) ||
                             (item.label.find("Slider") != std::string::npos) ||
                             (item.label.find("Frequency") != std::string::npos);
            if (is_slider) {
                el.type = ElementType::SliderFloat;
                el.value_num = 0.0f;
                el.min_val = 0.0f;
                el.max_val = 100.0f;
                auto s_it = m_slider_values.find(id);
                if (s_it != m_slider_values.end()) {
                    el.value_num = s_it->second;
                }
            } else {
                el.type = ElementType::InputText;
                auto it_str = m_input_strings.find(id);
                if (it_str != m_input_strings.end()) {
                    el.value_str = it_str->second;
                }
            }
        } else {
            el.type = ElementType::Button;
        }

        target_win->elements.push_back(std::move(el));
    }
    m_hooked_items.clear();
    m_hooked_order.clear();

    // Reorder m_doc.windows to match Dear ImGui's true display/stacking order (g.Windows)
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (g && g->Windows.Size > 0 && m_doc.windows.size() > 1) {
        std::unordered_map<uint32_t, int> win_order;
        for (int i = 0; i < g->Windows.Size; ++i) {
            ImGuiWindow* w = g->Windows[i];
            if (w) {
                int layer = (w->Flags & ImGuiWindowFlags_Tooltip) ? 10000 : 0;
                win_order[static_cast<uint32_t>(w->ID)] = layer + i;
            }
        }

        auto get_order = [&](const Window& win, int fallback_idx) -> int {
            auto it = win_order.find(win.id);
            if (it != win_order.end()) return it->second;
            for (int i = 0; i < g->Windows.Size; ++i) {
                ImGuiWindow* w = g->Windows[i];
                if (w && w->Name && win.title == w->Name) {
                    int layer = (w->Flags & ImGuiWindowFlags_Tooltip) ? 10000 : 0;
                    return layer + i;
                }
            }
            return fallback_idx;
        };

        std::vector<std::pair<int, size_t>> indexed_order;
        indexed_order.reserve(m_doc.windows.size());
        for (size_t i = 0; i < m_doc.windows.size(); ++i) {
            indexed_order.push_back({ get_order(m_doc.windows[i], static_cast<int>(i)), i });
        }

        std::stable_sort(indexed_order.begin(), indexed_order.end(),
            [](const std::pair<int, size_t>& a, const std::pair<int, size_t>& b) {
                return a.first < b.first;
            });

        std::vector<Window> sorted_windows;
        sorted_windows.reserve(m_doc.windows.size());
        for (const auto& item : indexed_order) {
            sorted_windows.push_back(std::move(m_doc.windows[item.second]));
        }
        m_doc.windows = std::move(sorted_windows);
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
                                       bool is_popup, bool is_modal, bool is_tooltip, bool collapsed,
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
            win.is_tooltip = is_tooltip;
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
    win.is_tooltip = is_tooltip;
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
                                   bool is_popup, bool is_modal, bool is_tooltip, bool collapsed,
                                   float scroll_x, float scroll_y) {
    if (!m_enabled) return;
    FindOrCreateWindow(id, title, x, y, w, h, title_bar_h, menu_bar_h, has_title_bar, has_menu_bar, is_popup, is_modal, is_tooltip, collapsed, scroll_x, scroll_y);
}

void DomContext::RecordWindowEnd() {
    m_current_window_id = 0;
}

void DomContext::OnHookItemAdd(uint32_t win_id, uint32_t id, float x, float y, float w, float h, uint32_t status_flags, uint32_t item_flags, bool is_menu_bar, bool is_stepper) {
    if (!m_enabled) return;
    HookItem item;
    item.id = id;
    item.win_id = win_id;
    item.x = x; item.y = y; item.w = w; item.h = h;
    item.flags = status_flags;
    item.item_flags = item_flags;
    item.is_menu_bar = is_menu_bar;
    item.is_stepper = is_stepper;

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

static inline uint32_t HashTextPos(const char* text, float x, float y) {
    uint32_t hash = 2166136261u;
    if (text) {
        for (const char* p = text; *p; ++p) {
            hash ^= static_cast<uint8_t>(*p);
            hash *= 16777619u;
        }
    }
    int ix = static_cast<int>(x * 10.0f);
    int iy = static_cast<int>(y * 10.0f);
    hash ^= static_cast<uint32_t>(ix);
    hash *= 16777619u;
    hash ^= static_cast<uint32_t>(iy);
    hash *= 16777619u;
    return (hash == 0) ? 1u : hash;
}

void DomContext::OnHookText(ImDrawList* dl, ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect) {
    (void)col;
    (void)cpu_fine_clip_rect;
    if (!m_enabled) return;
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (!g || !g->CurrentWindow) return;
    ImGuiWindow* window = g->CurrentWindow;
    if (dl != window->DrawList) return;
    if (window->Collapsed) return;

    // Skip text in title bar
    if (!(window->Flags & ImGuiWindowFlags_NoTitleBar)) {
        float title_bar_bottom = window->Pos.y + window->TitleBarHeight;
        if (pos.y < title_bar_bottom) return;
    }

    // Skip text that is part of an active widget with an explicit ID
    if (g->LastItemData.ID != 0) return;

    if (!text_begin) return;
    if (!text_end) {
        text_end = text_begin + strlen(text_begin);
    }
    if (text_begin == text_end) return;

    // Check if non-empty (not just spaces/newlines)
    bool has_visible_char = false;
    for (const char* p = text_begin; p < text_end; ++p) {
        if (!isspace(static_cast<unsigned char>(*p))) {
            has_visible_char = true;
            break;
        }
    }
    if (!has_visible_char) return;

    std::string text_str(text_begin, text_end);

    // Bounding box from ImGui last item rect or font size
    float w = g->LastItemData.Rect.GetWidth();
    float h = g->LastItemData.Rect.GetHeight();
    if (w <= 0.0f || h <= 0.0f) {
        ImVec2 sz = font ? font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text_begin, text_end) : ImVec2(0.0f, font_size);
        w = sz.x;
        h = sz.y;
    }

    uint32_t text_id = HashTextPos(text_str.c_str(), pos.x, pos.y);

    HookItem item;
    item.id = text_id;
    item.win_id = static_cast<uint32_t>(window->ID);
    item.x = pos.x;
    item.y = pos.y;
    item.w = w;
    item.h = h;
    item.is_text = true;
    item.label = std::move(text_str);

    if (m_hooked_items.find(text_id) == m_hooked_items.end()) {
        m_hooked_order.push_back(text_id);
    }
    m_hooked_items[text_id] = std::move(item);
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
    if (IsCurrentItemDisabled()) el.disabled = true;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordText(uint32_t id, const char* text, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    if (!text || !*text) return;

    if (id == 0) {
        id = HashTextPos(text, x, y);
    }

    // Deduplication check: if element already recorded (e.g. from OnHookText)
    for (auto& el : win->elements) {
        if (el.id == id || (el.type == ElementType::Text && el.value_str == text &&
            std::abs(el.x - x) < 2.0f && std::abs(el.y - y) < 2.0f)) {
            if (el.w <= 0.0f && w > 0.0f) el.w = w;
            if (el.h <= 0.0f && h > 0.0f) el.h = h;
            return;
        }
    }

    Element el;
    el.id = id;
    el.type = ElementType::Text;
    el.value_str = text;
    el.x = x; el.y = y; el.w = w; el.h = h;
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
    if (IsCurrentItemDisabled()) el.disabled = true;
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
    if (IsCurrentItemDisabled()) el.disabled = true;
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
    if (IsCurrentItemDisabled()) el.disabled = true;
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
    if (IsCurrentItemDisabled()) el.disabled = true;
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
    if (IsCurrentItemDisabled()) el.disabled = true;
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
    if (IsCurrentItemDisabled()) el.disabled = true;
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

void DomContext::RecordTabBar(uint32_t id, const char* str_id, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::TabBar;
    el.label = str_id ? str_id : "";
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordTabItem(uint32_t id, const char* label, bool selected, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::TabItem;
    el.label = label ? label : "";
    el.selected = selected;
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::SetItemTooltip(const char* text) {
    if (!m_enabled || text == nullptr) return;
    Window* win = GetCurrentWindow();
    if (!win || win->elements.empty()) return;
    win->elements.back().tooltip = text;
}

void DomContext::RecordTableBegin(uint32_t id, const char* str_id, int columns_count, uint32_t flags) {
    if (!m_enabled) return;
    ActiveTable tbl;
    tbl.id = id;
    tbl.str_id = str_id ? str_id : "";
    tbl.columns_count = columns_count;
    tbl.flags = flags;
    tbl.has_headers = false;
    m_active_tables.push_back(std::move(tbl));
}

void DomContext::RecordTableColumn(const char* label) {
    if (!m_enabled || m_active_tables.empty()) return;
    m_active_tables.back().columns.push_back(label ? label : "");
}

void DomContext::RecordTableHeadersRow() {
    if (!m_enabled || m_active_tables.empty()) return;
    m_active_tables.back().has_headers = true;
}

void DomContext::RecordTableEnd(float x, float y, float w, float h) {
    if (!m_enabled || m_active_tables.empty()) return;
    ActiveTable tbl = std::move(m_active_tables.back());
    m_active_tables.pop_back();

    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = tbl.id;
    el.type = ElementType::Table;
    el.label = tbl.str_id;
    el.columns_count = tbl.columns_count;
    el.has_headers = tbl.has_headers;
    el.items = std::move(tbl.columns);
    el.x = x; el.y = y; el.w = w; el.h = h;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordListBox(uint32_t id, const char* label, int current_item, const std::vector<std::string>& items, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::ListBox;
    el.label = label ? label : "";
    el.selected_idx = current_item;
    el.items = items;
    el.x = x; el.y = y; el.w = w; el.h = h;
    if (IsCurrentItemDisabled()) el.disabled = true;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordColorEdit(uint32_t id, const char* label, float r, float g, float b, float a, bool has_alpha, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::ColorEdit;
    el.label = label ? label : "";
    el.has_alpha = has_alpha;

    int ir = std::clamp(static_cast<int>(r * 255.0f + 0.5f), 0, 255);
    int ig = std::clamp(static_cast<int>(g * 255.0f + 0.5f), 0, 255);
    int ib = std::clamp(static_cast<int>(b * 255.0f + 0.5f), 0, 255);
    char hex[16];
    snprintf(hex, sizeof(hex), "#%02x%02x%02x", ir, ig, ib);
    el.value_str = hex;

    el.x = x; el.y = y; el.w = w; el.h = h;
    if (IsCurrentItemDisabled()) el.disabled = true;
    win->elements.push_back(std::move(el));
}

void DomContext::RecordInputTextMultiline(uint32_t id, const char* label, const char* text, float x, float y, float w, float h) {
    if (!m_enabled) return;
    Window* win = GetCurrentWindow();
    if (!win) return;
    Element el;
    el.id = id;
    el.type = ElementType::InputTextMultiline;
    el.label = label ? label : "";
    el.value_str = text ? text : "";
    el.x = x; el.y = y; el.w = w; el.h = h;
    if (IsCurrentItemDisabled()) el.disabled = true;
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
        if (evt.type == "input" || evt.type == "input_multiline") {
            m_input_strings[evt.id] = evt.value_str;
        } else if (evt.type == "slider") {
            m_slider_values[evt.id] = evt.value_num;
        } else if (evt.type == "checkbox") {
            m_checkbox_values[evt.id] = evt.checked;
        } else if (evt.type == "click") {
            m_active_clicks.insert(evt.id);
        } else if (evt.type == "tab") {
            m_tab_selections.insert(evt.id);
            if (!evt.label.empty()) {
                m_tab_names.insert(evt.label);
            } else if (!evt.value_str.empty()) {
                m_tab_names.insert(evt.value_str);
            }
            m_active_clicks.insert(evt.id);
        } else if (evt.type == "listbox") {
            m_listbox_selections[evt.id] = static_cast<int>(evt.value_num);
            m_active_clicks.insert(evt.id);
        } else if (evt.type == "color") {
            int ir = 0, ig = 0, ib = 0;
            if (sscanf(evt.value_str.c_str(), "#%02x%02x%02x", &ir, &ig, &ib) == 3) {
                float r = ir / 255.0f;
                float g = ig / 255.0f;
                float b = ib / 255.0f;
                m_color_values[evt.id] = { r, g, b, 1.0f };
            }
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

bool DomContext::ConsumeTabSelect(uint32_t id, const char* label) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_tab_selections.erase(id) > 0) return true;
    if (label && m_tab_names.erase(label) > 0) return true;
    return false;
}

bool DomContext::ConsumeListBox(uint32_t id, int& out_index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_listbox_selections.find(id);
    if (it != m_listbox_selections.end()) {
        out_index = it->second;
        m_listbox_selections.erase(it);
        return true;
    }
    return false;
}

bool DomContext::ConsumeColor3(uint32_t id, float out_col[3]) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_color_values.find(id);
    if (it != m_color_values.end()) {
        out_col[0] = it->second[0];
        out_col[1] = it->second[1];
        out_col[2] = it->second[2];
        m_color_values.erase(it);
        return true;
    }
    return false;
}

bool DomContext::ConsumeColor4(uint32_t id, float out_col[4]) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_color_values.find(id);
    if (it != m_color_values.end()) {
        out_col[0] = it->second[0];
        out_col[1] = it->second[1];
        out_col[2] = it->second[2];
        out_col[3] = it->second[3];
        m_color_values.erase(it);
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
        } else if (evt.type == "window_move") {
            if (ImGui::GetCurrentContext()) {
                ImGuiWindow* win = nullptr;
                if (evt.id != 0) {
                    win = ImGui::FindWindowByID(evt.id);
                }
                if (!win && !evt.value_str.empty()) {
                    win = ImGui::FindWindowByName(evt.value_str.c_str());
                }
                if (win) {
                    ImGui::SetWindowPos(win, ImVec2(evt.x, evt.y), ImGuiCond_Always);
                    ImGui::FocusWindow(win);
                    ImGui::BringWindowToDisplayFront(win);
                }
            }
        } else if (evt.type == "window_focus") {
            if (ImGui::GetCurrentContext()) {
                ImGuiWindow* win = nullptr;
                if (evt.id != 0) {
                    win = ImGui::FindWindowByID(evt.id);
                }
                if (!win && !evt.value_str.empty()) {
                    win = ImGui::FindWindowByName(evt.value_str.c_str());
                }
                if (win) {
                    ImGui::FocusWindow(win);
                    ImGui::BringWindowToDisplayFront(win);
                }
            }
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
