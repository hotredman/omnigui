#pragma once

#include "dom_tree.h"
#include <mutex>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <functional>

struct ImGuiIO;

namespace ImGuiDom {

struct BrowserEvent {
    std::string type; // "click", "slider", "checkbox", "input", "mouse_move", "mouse_down", "mouse_up", "mouse_wheel", "key_down", "key_up", "char"
    uint32_t id = 0;
    float x = 0.0f;
    float y = 0.0f;
    float dx = 0.0f;
    float dy = 0.0f;
    int button = 0;
    int key = 0;
    float value_num = 0.0f;
    bool checked = false;
    std::string value_str;
};

class DomContext {
public:
    static DomContext& Instance() {
        static DomContext instance;
        return instance;
    }

    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

    void BeginFrame(uint64_t frame_index);
    void EndFrame();

    // DOM Element recording methods
    void RecordWindowBegin(uint32_t id, const char* title, float x, float y, float w, float h,
                           float title_bar_h = 24.0f, float menu_bar_h = 0.0f,
                           bool has_title_bar = true, bool has_menu_bar = false,
                           bool is_popup = false, bool is_modal = false, bool is_tooltip = false, bool collapsed = false,
                           float scroll_x = 0.0f, float scroll_y = 0.0f);
    void RecordWindowEnd();

    void RecordButton(uint32_t id, const char* label, float x, float y, float w, float h);
    void RecordText(uint32_t id, const char* text, float x, float y);
    void RecordSliderFloat(uint32_t id, const char* label, float val, float min_v, float max_v, float x, float y, float w, float h);
    void RecordCheckbox(uint32_t id, const char* label, bool checked, float x, float y, float w, float h);
    void RecordRadioButton(uint32_t id, const char* label, bool active, float x, float y, float w, float h);
    void RecordInputText(uint32_t id, const char* label, const char* text, float x, float y, float w, float h);
    void RecordCombo(uint32_t id, const char* label, const char* preview, bool opened, float x, float y, float w, float h);
    void RecordProgressBar(uint32_t id, float fraction, const char* overlay, float x, float y, float w, float h);
    void RecordSeparator(uint32_t id, float x, float y, float w, float h);
    void RecordCanvas(uint32_t id, const char* stream_name, const float* points, size_t count, float x, float y, float w, float h);
    void RecordTabBar(uint32_t id, const char* str_id, float x, float y, float w, float h);
    void RecordTabItem(uint32_t id, const char* label, bool selected, float x, float y, float w, float h);
    void SetItemTooltip(const char* text);
    void RecordTableBegin(uint32_t id, const char* str_id, int columns_count, uint32_t flags);
    void RecordTableColumn(const char* label);
    void RecordTableHeadersRow();
    void RecordTableEnd(float x, float y, float w, float h);
    void RecordListBox(uint32_t id, const char* label, int current_item, const std::vector<std::string>& items, float x, float y, float w, float h);

    // Browser event handling (called by HTTP server)
    void PushBrowserEvent(const BrowserEvent& evt);
    using EventCallback = std::function<void()>;
    void SetEventCallback(EventCallback cb);

    // Event consumption in ImGui frame
    bool ConsumeClick(uint32_t id);
    bool ConsumeSlider(uint32_t id, float& out_val);
    bool ConsumeCheckbox(uint32_t id, bool& out_checked);
    bool ConsumeInputText(uint32_t id, std::string& out_str);
    bool ConsumeTabSelect(uint32_t id);
    bool ConsumeListBox(uint32_t id, int& out_index);

    // Internal ImGui Hook callbacks
    void OnHookItemAdd(uint32_t win_id, uint32_t id, float x, float y, float w, float h, uint32_t status_flags, uint32_t item_flags = 0);
    void OnHookItemInfo(uint32_t id, const char* label, uint32_t status_flags);
    const char* GetItemLabel(uint32_t id);

    // Sync input events into ImGuiIO if needed
    void ProcessInputEvents(ImGuiIO& io);

    // Latest JSON for broadcast to web clients
    std::string GetLatestJson();
    bool HasNewSnapshot(uint64_t& inout_last_sent_frame);

    using SnapshotCallback = std::function<void(const std::string&)>;
    void SetSnapshotCallback(SnapshotCallback cb);

private:
    DomContext() = default;

    Window* GetCurrentWindow();
    Window* FindOrCreateWindow(uint32_t id, const char* title, float x, float y, float w, float h,
                               float title_bar_h, float menu_bar_h,
                               bool has_title_bar, bool has_menu_bar,
                               bool is_popup, bool is_modal, bool is_tooltip, bool collapsed,
                               float scroll_x, float scroll_y);

    bool m_enabled = false;
    uint64_t m_current_frame = 0;
    uint64_t m_latest_ready_frame = 0;
    Document m_doc;
    uint32_t m_current_window_id = 0;

    // Automatic item capture via ItemAdd/ItemInfo hooks
    struct HookItem {
        uint32_t id = 0;
        uint32_t win_id = 0;
        float x = 0, y = 0, w = 0, h = 0;
        uint32_t flags = 0;
        uint32_t item_flags = 0;
        std::string label;
    };
    std::unordered_map<uint32_t, HookItem> m_hooked_items;
    std::vector<uint32_t> m_hooked_order;
    std::unordered_map<uint32_t, std::string> m_item_labels;

    // Concurrency protection for JSON snapshot & events
    mutable std::mutex m_mutex;
    std::string m_latest_json;

    // Active Table recording state
    struct ActiveTable {
        uint32_t id = 0;
        std::string str_id;
        int columns_count = 0;
        uint32_t flags = 0;
        bool has_headers = false;
        std::vector<std::string> columns;
    };
    std::vector<ActiveTable> m_active_tables;

    // Incoming browser events
    std::unordered_set<uint32_t> m_active_clicks;
    std::unordered_set<uint32_t> m_tab_selections;
    std::unordered_map<uint32_t, int> m_listbox_selections;
    std::unordered_map<uint32_t, float> m_slider_values;
    std::unordered_map<uint32_t, bool> m_checkbox_values;
    std::unordered_map<uint32_t, std::string> m_input_strings;
    std::queue<BrowserEvent> m_pending_events;
    EventCallback m_event_callback;
    SnapshotCallback m_snapshot_callback;
};

} // namespace ImGuiDom
