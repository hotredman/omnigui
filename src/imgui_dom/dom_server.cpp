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

static BrowserEvent ParseBrowserEvent(const std::string& json) {
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
    return evt;
}

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
    m_server->Post("/api/event", [](const httplib::Request& req, httplib::Response& res) {
        DomContext::Instance().PushBrowserEvent(ParseBrowserEvent(req.body));
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
                DomContext::Instance().PushBrowserEvent(ParseBrowserEvent(msg));
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

} // namespace ImGuiDom
