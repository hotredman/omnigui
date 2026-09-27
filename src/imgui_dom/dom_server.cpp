#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "dom_server.h"
#include "dom_context.h"
#include "web_ui.h"
#include <httplib.h>
#include <iostream>
#include <chrono>

namespace ImGuiDom {

DomServer::DomServer() = default;

DomServer::~DomServer() {
    Stop();
}

bool DomServer::Start(const std::string& host, int port) {
    if (m_running) return true;

    m_host = host;
    m_port = port;
    m_server = std::make_unique<httplib::Server>();

    if (!m_server->is_valid()) {
        std::cerr << "[DomServer] Error: Failed to create httplib::Server\n";
        return false;
    }

    // 1. Web client HTML route
    m_server->Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(GetWebClientHtml(), "text/html");
    });
    m_server->Get("/index.html", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(GetWebClientHtml(), "text/html");
    });
    m_server->Get("/api/snapshot", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(DomContext::Instance().GetLatestJson(), "application/json");
    });

    // 2. Real-time DOM stream via Server-Sent Events (SSE)
    m_server->Get("/api/stream", [this](const httplib::Request&, httplib::Response& res) {
        res.set_header("Cache-Control", "no-cache");
        res.set_header("Access-Control-Allow-Origin", "*");

        res.set_chunked_content_provider("text/event-stream", [this](size_t, httplib::DataSink& sink) {
            uint64_t last_sent_frame = 0;
            while (sink.is_writable() && m_running) {
                if (DomContext::Instance().HasNewSnapshot(last_sent_frame)) {
                    std::string json = DomContext::Instance().GetLatestJson();
                    if (!json.empty()) {
                        std::string sse_packet = "data: " + json + "\n\n";
                        if (!sink.write(sse_packet.data(), sse_packet.size())) {
                            break;
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
            }
            return true;
        });
    });

    // 3. Browser interaction event receiver (HTTP POST fallback)
    m_server->Post("/api/event", [this](const httplib::Request& req, httplib::Response& res) {
        BrowserEvent evt = ParseBrowserEvent(req.body);
        if (m_event_receiver) {
            m_event_receiver(evt);
        } else {
            DomContext::Instance().PushBrowserEvent(evt);
        }
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // 4. Low-latency full-duplex WebSocket route (/ws)
    m_server->WebSocket("/ws", [this](const httplib::Request&, httplib::ws::WebSocket& ws) {
        ws.set_read_timeout(1, 0); // 1s timeout to allow non-blocking close detection

        {
            std::lock_guard<std::mutex> lock(m_ws_mutex);
            m_ws_clients.push_back(&ws);
        }

        // Send current latest snapshot immediately upon connection
        std::string initial_json = DomContext::Instance().GetLatestJson();
        if (!initial_json.empty() && ws.is_open()) {
            ws.send(initial_json);
        }

        std::string msg;
        while (m_running && ws.is_open()) {
            auto res = ws.read(msg);
            if (res == httplib::ws::ReadResult::Text) {
                BrowserEvent evt = ParseBrowserEvent(msg);
                if (m_event_receiver) {
                    m_event_receiver(evt);
                } else {
                    DomContext::Instance().PushBrowserEvent(evt);
                }
            } else if (res == httplib::ws::ReadResult::Timeout) {
                continue;
            } else {
                break;
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_ws_mutex);
            auto it = std::find(m_ws_clients.begin(), m_ws_clients.end(), &ws);
            if (it != m_ws_clients.end()) {
                m_ws_clients.erase(it);
            }
        }
    });

    // Register real-time WebSocket broadcast callback for completed frames
    DomContext::Instance().SetSnapshotCallback([this](const std::string& json) {
        BroadcastWebSocket(json);
    });

    m_running = true;
    m_thread = std::thread([this]() {
        std::cout << "[DomServer] Web DOM Server listening on http://" << m_host << ":" << m_port << "\n";
        m_server->listen(m_host, m_port);
        m_running = false;
    });

    DomContext::Instance().SetEnabled(true);
    return true;
}

void DomServer::BroadcastWebSocket(const std::string& json) {
    std::lock_guard<std::mutex> lock(m_ws_mutex);
    for (auto* ws : m_ws_clients) {
        if (ws && ws->is_open()) {
            ws->send(json);
        }
    }
}

void DomServer::Stop() {
    if (!m_running) return;

    m_running = false;
    DomContext::Instance().SetSnapshotCallback(nullptr);
    DomContext::Instance().SetEnabled(false);

    {
        std::lock_guard<std::mutex> lock(m_ws_mutex);
        for (auto* ws : m_ws_clients) {
            if (ws && ws->is_open()) {
                ws->close();
            }
        }
    }

    if (m_server) {
        m_server->stop();
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }

    {
        std::lock_guard<std::mutex> lock(m_ws_mutex);
        m_ws_clients.clear();
    }

    m_server.reset();
    std::cout << "[DomServer] Server stopped.\n";
}

std::shared_ptr<IDomTransport> CreateWebSocketTransport(const std::string& host, int port) {
    struct DomServerSharedWrapper : public IDomTransport {
        std::string m_h;
        int m_p;
        DomServerSharedWrapper(std::string h, int p) : m_h(std::move(h)), m_p(p) {}
        TransportType GetType() const override { return TransportType::WebSocket; }
        const char* GetName() const override { return "WebSocketServer"; }
        bool Start() override { return DomServer::Instance().Start(m_h, m_p); }
        void Stop() override { DomServer::Instance().Stop(); }
        bool IsRunning() const override { return DomServer::Instance().IsRunning(); }
        void SendSnapshot(const std::string& json) override { DomServer::Instance().SendSnapshot(json); }
        void SetEventReceiver(EventReceiver receiver) override { DomServer::Instance().SetEventReceiver(std::move(receiver)); }
    };
    return std::make_shared<DomServerSharedWrapper>(host, port);
}

} // namespace ImGuiDom
