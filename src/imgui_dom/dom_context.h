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
    std::string type; // "click", "slider", "checkbox", "input"
    uint32_t id = 0;
    float x = 0.0f;
    float y = 0.0f;
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
    void RecordWindowBegin(uint32_t id, const char* title, float x, float y, float w, float h);
    void RecordWindowEnd();

    void RecordButton(uint32_t id, const char* label, float x, float y, float w, float h);
    void RecordText(uint32_t id, const char* text, float x, float y);
    void RecordSliderFloat(uint32_t id, const char* label, float val, float min_v, float max_v, float x, float y, float w, float h);
    void RecordCheckbox(uint32_t id, const char* label, bool checked, float x, float y, float w, float h);
    void RecordCanvas(uint32_t id, const char* stream_name, const float* points, size_t count, float x, float y, float w, float h);

    // Browser event handling (called by HTTP server)
    void PushBrowserEvent(const BrowserEvent& evt);
    using EventCallback = std::function<void()>;
    void SetEventCallback(EventCallback cb);

    // Event consumption in ImGui frame
    bool ConsumeClick(uint32_t id);
    bool ConsumeSlider(uint32_t id, float& out_val);
    bool ConsumeCheckbox(uint32_t id, bool& out_checked);

    // Internal ImGui Hook callbacks
    void OnHookItemAdd(uint32_t id, float x, float y, float w, float h, uint32_t status_flags);
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

    bool m_enabled = false;
    uint64_t m_current_frame = 0;
    uint64_t m_latest_ready_frame = 0;
    Document m_doc;
    Window m_current_window;
    bool m_has_active_window = false;

    // Automatic item capture via ItemAdd/ItemInfo hooks
    struct HookItem {
        uint32_t id = 0;
        float x = 0, y = 0, w = 0, h = 0;
        uint32_t flags = 0;
        std::string label;
    };
    std::unordered_map<uint32_t, HookItem> m_hooked_items;
    std::vector<uint32_t> m_hooked_order;
    std::unordered_map<uint32_t, std::string> m_item_labels;

    // Concurrency protection for JSON snapshot & events
    mutable std::mutex m_mutex;
    std::string m_latest_json;

    // Incoming browser events
    std::unordered_set<uint32_t> m_active_clicks;
    std::unordered_map<uint32_t, float> m_slider_values;
    std::unordered_map<uint32_t, bool> m_checkbox_values;
    std::queue<BrowserEvent> m_pending_events;
    EventCallback m_event_callback;
    SnapshotCallback m_snapshot_callback;
};

} // namespace ImGuiDom
