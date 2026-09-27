#pragma once

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

class DomServer {
public:
    static DomServer& Instance() {
        static DomServer instance;
        return instance;
    }

    bool Start(const std::string& host = "0.0.0.0", int port = 8080);
    void Stop();
    bool IsRunning() const { return m_running; }
    int GetPort() const { return m_port; }

    void BroadcastWebSocket(const std::string& json);

private:
    DomServer();
    ~DomServer();

    std::atomic<bool> m_running{false};
    int m_port = 8080;
    std::string m_host = "0.0.0.0";
    std::thread m_thread;
    std::unique_ptr<httplib::Server> m_server;

    std::mutex m_ws_mutex;
    std::vector<httplib::ws::WebSocket*> m_ws_clients;
};

} // namespace ImGuiDom
