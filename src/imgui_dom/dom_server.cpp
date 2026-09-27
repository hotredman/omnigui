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

    // 3. Browser interaction event receiver
    m_server->Post("/api/event", [](const httplib::Request& req, httplib::Response& res) {
        BrowserEvent evt;
        evt.type = ExtractJsonString(req.body, "type");
        evt.id = ExtractJsonUint(req.body, "id");
        evt.x = ExtractJsonFloat(req.body, "x");
        evt.y = ExtractJsonFloat(req.body, "y");
        evt.value_num = ExtractJsonFloat(req.body, "val");
        evt.checked = ExtractJsonBool(req.body, "checked");
        evt.value_str = ExtractJsonString(req.body, "text");

        DomContext::Instance().PushBrowserEvent(evt);

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("{\"status\":\"ok\"}", "application/json");
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

void DomServer::Stop() {
    if (!m_running) return;

    m_running = false;
    DomContext::Instance().SetEnabled(false);

    if (m_server) {
        m_server->stop();
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_server.reset();
    std::cout << "[DomServer] Server stopped.\n";
}

} // namespace ImGuiDom
