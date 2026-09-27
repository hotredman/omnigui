#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace ImGuiDom {

struct BrowserEvent;

enum class TransportType {
    WebSocket,  // Remote HTTP + WebSocket Server (for desktop/headless remote UI)
    WasmDirect, // In-browser direct JavaScript bridge (for standalone WebAssembly)
    Loopback    // In-memory loopback (for unit tests and offline simulation)
};

class IDomTransport {
public:
    virtual ~IDomTransport() = default;

    virtual TransportType GetType() const = 0;
    virtual const char* GetName() const = 0;

    // Lifecycle
    virtual bool Start() = 0;
    virtual void Stop() = 0;
    virtual bool IsRunning() const = 0;

    // Outbound: broadcast DOM snapshot to connected client(s)
    virtual void SendSnapshot(const std::string& json) = 0;

    // Inbound: configure callback when transport receives a browser event
    using EventReceiver = std::function<void(const BrowserEvent&)>;
    virtual void SetEventReceiver(EventReceiver receiver) = 0;
};

// JSON parsing helper for browser events
BrowserEvent ParseBrowserEvent(const std::string& json);

// Factory functions
std::shared_ptr<IDomTransport> CreateWebSocketTransport(const std::string& host = "0.0.0.0", int port = 8080);
std::shared_ptr<IDomTransport> CreateWasmDirectTransport();
std::shared_ptr<IDomTransport> CreateLoopbackTransport();

} // namespace ImGuiDom

extern "C" {
void omni_wasm_send_event(const char* event_json);
}
