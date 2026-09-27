#include "dom_transport.h"
#include "dom_context.h"
#include <iostream>
#include <mutex>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, omni_wasm_js_apply_snapshot, (const char* json_ptr), {
    if (typeof window !== 'undefined' && typeof window.__omniDomApplySnapshot === 'function') {
        window.__omniDomApplySnapshot(UTF8ToString(json_ptr));
    }
});
#endif

namespace ImGuiDom {

// --- JSON Parsing Helpers for Inbound Browser Events ---
static std::string ExtractJsonString(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    pos = json.find("\"", pos);
    if (pos == std::string::npos) return "";
    size_t end = json.find("\"", pos + 1);
    if (end == std::string::npos) return "";
    return json.substr(pos + 1, end - (pos + 1));
}

static float ExtractJsonFloat(const std::string& json, const std::string& key, float default_val = 0.0f) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return default_val;
    pos = json.find(":", pos);
    if (pos == std::string::npos) return default_val;
    pos = json.find_first_not_of(" \t", pos + 1);
    if (pos == std::string::npos) return default_val;
    size_t end = json.find_first_of(",}\" \t\r\n", pos);
    try {
        return std::stof(json.substr(pos, end - pos));
    } catch (...) {
        return default_val;
    }
}

static uint64_t ExtractJsonUint64(const std::string& json, const std::string& key, uint64_t default_val = 0) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return default_val;
    pos = json.find(":", pos);
    if (pos == std::string::npos) return default_val;
    pos = json.find_first_not_of(" \t", pos + 1);
    if (pos == std::string::npos) return default_val;
    size_t end = json.find_first_of(",}\" \t\r\n", pos);
    try {
        return std::stoull(json.substr(pos, end - pos));
    } catch (...) {
        return default_val;
    }
}

static uint32_t ExtractJsonUint(const std::string& json, const std::string& key) {
    return static_cast<uint32_t>(ExtractJsonUint64(json, key, 0));
}

static bool ExtractJsonBool(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;
    pos = json.find(":", pos);
    if (pos == std::string::npos) return false;
    size_t val_pos = json.find("true", pos);
    size_t comma_pos = json.find_first_of(",}", pos);
    return (val_pos != std::string::npos && val_pos < comma_pos);
}

static int ExtractJsonInt(const std::string& json, const std::string& key, int default_val = 0) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return default_val;
    pos = json.find(":", pos);
    if (pos == std::string::npos) return default_val;
    pos = json.find_first_not_of(" \t", pos + 1);
    if (pos == std::string::npos) return default_val;
    size_t end = json.find_first_of(",}\" \t\r\n", pos);
    try {
        return std::stoi(json.substr(pos, end - pos));
    } catch (...) {
        return default_val;
    }
}

BrowserEvent ParseBrowserEvent(const std::string& json) {
    BrowserEvent evt;
    evt.type = ExtractJsonString(json, "type");
    evt.id = ExtractJsonUint(json, "id");
    evt.x = ExtractJsonFloat(json, "x");
    evt.y = ExtractJsonFloat(json, "y");
    evt.dx = ExtractJsonFloat(json, "dx");
    evt.dy = ExtractJsonFloat(json, "dy");
    evt.button = ExtractJsonInt(json, "button", 0);
    evt.key = ExtractJsonInt(json, "key", 0);
    evt.value_num = ExtractJsonFloat(json, "val");
    if (evt.value_num == 0.0f) {
        evt.value_num = ExtractJsonFloat(json, "value_num");
    }
    evt.checked = ExtractJsonBool(json, "checked");
    evt.value_str = ExtractJsonString(json, "text");
    if (evt.value_str.empty()) {
        evt.value_str = ExtractJsonString(json, "color");
    }
    if (evt.value_str.empty()) {
        evt.value_str = ExtractJsonString(json, "title");
    }
    evt.label = ExtractJsonString(json, "label");
    if (evt.value_str.empty()) {
        evt.value_str = evt.label;
    }
    return evt;
}

// --- WasmDirectTransport Implementation ---
class WasmDirectTransport : public IDomTransport {
public:
    TransportType GetType() const override { return TransportType::WasmDirect; }
    const char* GetName() const override { return "WasmDirect"; }

    bool Start() override {
        m_running = true;
        s_active_wasm_transport = this;
        return true;
    }

    void Stop() override {
        m_running = false;
        if (s_active_wasm_transport == this) {
            s_active_wasm_transport = nullptr;
        }
    }

    bool IsRunning() const override { return m_running; }

    void SendSnapshot(const std::string& json) override {
        if (!m_running) return;
#ifdef __EMSCRIPTEN__
        omni_wasm_js_apply_snapshot(json.c_str());
#else
        std::lock_guard<std::mutex> lock(m_mutex);
        m_latest_snapshot = json;
#endif
    }

    void SetEventReceiver(EventReceiver receiver) override {
        m_receiver = std::move(receiver);
    }

    void OnBrowserEventReceived(const std::string& json) {
        if (m_receiver) {
            BrowserEvent evt = ParseBrowserEvent(json);
            m_receiver(evt);
        }
    }

    std::string GetLatestSnapshot() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_latest_snapshot;
    }

    static WasmDirectTransport* GetActiveInstance() {
        return s_active_wasm_transport;
    }

private:
    bool m_running = false;
    EventReceiver m_receiver;
    mutable std::mutex m_mutex;
    std::string m_latest_snapshot;
    static inline WasmDirectTransport* s_active_wasm_transport = nullptr;
};

// --- LoopbackTransport Implementation (for unit tests & mock runs) ---
class LoopbackTransport : public IDomTransport {
public:
    TransportType GetType() const override { return TransportType::Loopback; }
    const char* GetName() const override { return "Loopback"; }

    bool Start() override {
        m_running = true;
        return true;
    }

    void Stop() override {
        m_running = false;
    }

    bool IsRunning() const override { return m_running; }

    void SendSnapshot(const std::string& json) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_snapshots.push_back(json);
        m_latest_snapshot = json;
    }

    void SetEventReceiver(EventReceiver receiver) override {
        m_receiver = std::move(receiver);
    }

    void InjectEvent(const BrowserEvent& evt) {
        if (m_receiver) {
            m_receiver(evt);
        }
    }

    std::string GetLatestSnapshot() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_latest_snapshot;
    }

    size_t GetSnapshotCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_snapshots.size();
    }

private:
    bool m_running = false;
    EventReceiver m_receiver;
    mutable std::mutex m_mutex;
    std::vector<std::string> m_snapshots;
    std::string m_latest_snapshot;
};

std::shared_ptr<IDomTransport> CreateWasmDirectTransport() {
    return std::make_shared<WasmDirectTransport>();
}

std::shared_ptr<IDomTransport> CreateLoopbackTransport() {
    return std::make_shared<LoopbackTransport>();
}

} // namespace ImGuiDom

// --- Global C-Bridge for WebAssembly / JS Invocation ---
extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void omni_wasm_send_event(const char* event_json) {
    if (!event_json) return;
    auto* wasm_transport = ImGuiDom::WasmDirectTransport::GetActiveInstance();
    if (wasm_transport) {
        wasm_transport->OnBrowserEventReceived(std::string(event_json));
    }
}

}
