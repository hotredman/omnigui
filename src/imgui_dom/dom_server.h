#pragma once

#include "dom_transport.h"
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace httplib {
    class Server;
    namespace ws {
        class WebSocket;
    }
}

namespace ImGuiDom {

class DomServer : public IDomTransport {
public:
    static DomServer& Instance() {
        static DomServer instance;
        return instance;
    }

    TransportType GetType() const override { return TransportType::WebSocket; }
    const char* GetName() const override { return "WebSocketServer"; }

    bool Start() override { return Start(m_host, m_port); }
    bool Start(const std::string& host, int port);
    void Stop() override;
    bool IsRunning() const override { return m_running; }
    int GetPort() const { return m_port; }
    const std::string& GetHost() const { return m_host; }

    void SendSnapshot(const std::string& json) override {
        BroadcastWebSocket(json);
    }

    void SetEventReceiver(EventReceiver receiver) override {
        m_event_receiver = std::move(receiver);
    }

    void BroadcastWebSocket(const std::string& json);

private:
    DomServer();
    ~DomServer() override;

    std::atomic<bool> m_running{false};
    int m_port = 8080;
    std::string m_host = "0.0.0.0";
    std::thread m_thread;
    std::unique_ptr<httplib::Server> m_server;

    std::mutex m_ws_mutex;
    std::vector<httplib::ws::WebSocket*> m_ws_clients;
    EventReceiver m_event_receiver;
};

} // namespace ImGuiDom
