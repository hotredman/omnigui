#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <httplib.h>

#include "imgui.h"
#include "imgui_dom/imgui_dom.h"

int main() {
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);
    std::cout << "TEST_DOM STARTING..." << std::endl;
    std::cout << "========================================================\n";
    std::cout << " Stage 6: ImGui Native Web DOM Backend Unit Test\n";
    std::cout << "========================================================\n\n";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);
    io.DeltaTime = 1.0f / 60.0f;
    io.IniFilename = nullptr;

    unsigned char* pixels = nullptr;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    int test_port = 8899;
    std::cout << "[Step 1] Starting embedded HTTP/SSE DOM Server on port " << test_port << "...\n";
    bool server_started = ImGuiDom::StartServer(test_port, "127.0.0.1");
    if (!server_started) {
        std::cerr << "[FAIL] Could not start DomServer on port " << test_port << "\n";
        return 1;
    }

    // Give server thread 50ms to bind port
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // -----------------------------------------------------------------
    // Frame 1: Generate DOM tree with Window, Button, and Slider
    // -----------------------------------------------------------------
    std::cout << "[Step 2] Rendering Frame 1 (Building DOM Tree)...\n";
    ImGuiDom::BeginFrame(1);
    ImGui::NewFrame();

    float test_freq = 25.0f;
    bool test_clicked = false;
    uint32_t btn_id = 0;
    uint32_t slider_id = 0;

    ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::Begin("Signal Controls", nullptr);
    btn_id = static_cast<uint32_t>(ImGui::GetID("Reset Calibration"));
    slider_id = static_cast<uint32_t>(ImGui::GetID("Frequency"));
    ImGui::Text("Status: Active 60 FPS");
    if (ImGui::Button("Reset Calibration")) {
        test_clicked = true;
    }
    ImGui::SliderFloat("Frequency", &test_freq, 1.0f, 100.0f);
    ImGui::End();

    ImGui::Render();
    ImGuiDom::EndFrame();

    // Verify generated JSON snapshot
    std::string json = ImGuiDom::DomContext::Instance().GetLatestJson();
    std::cout << "Generated DOM JSON length: " << json.size() << " bytes\n";
    std::cout << "JSON Content: " << json << "\n";

    if (json.find("\"title\":\"Signal Controls\"") == std::string::npos) {
        std::cerr << "[FAIL] Window title not found in DOM JSON\n";
        return 1;
    }
    if (json.find("\"label\":\"Reset Calibration\"") == std::string::npos) {
        std::cerr << "[FAIL] Button label not found in DOM JSON\n";
        return 1;
    }
    if (json.find("\"type\":\"slider\"") == std::string::npos) {
        std::cerr << "[FAIL] Slider not found in DOM JSON\n";
        return 1;
    }

    // -----------------------------------------------------------------
    // HTTP Client Test: Request HTML web client from server
    // -----------------------------------------------------------------
    std::cout << "[Step 3] HTTP GET / -> Fetching Web Client HTML...\n";
    httplib::Client cli("127.0.0.1", test_port);
    auto res = cli.Get("/");
    if (!res || res->status != 200) {
        std::cerr << "[FAIL] HTTP GET / returned " << (res ? res->status : -1) << "\n";
        return 1;
    }
    if (res->body.find("Dear ImGui + ThorVG &rarr; True HTML5 Web DOM Backend") == std::string::npos) {
        std::cerr << "[FAIL] HTML body does not contain expected header\n";
        return 1;
    }
    std::cout << ">>> [PASS] HTTP GET / served native HTML5 web client (" << res->body.size() << " bytes)\n";

    // -----------------------------------------------------------------
    // Step 4: WebSocket Full-Duplex Test
    // -----------------------------------------------------------------
    std::cout << "[Step 4] Connecting via WebSocket ws://127.0.0.1:" << test_port << "/ws...\n";
    httplib::ws::WebSocketClient ws_cli("ws://127.0.0.1:" + std::to_string(test_port) + "/ws");
    ws_cli.set_read_timeout(std::chrono::seconds(2));
    auto ws_conn = ws_cli.connect();
    if (!ws_conn) {
        std::cerr << "[FAIL] WebSocket connect failed: " << (int)ws_conn.error() << "\n";
        return 1;
    }
    std::cout << ">>> [PASS] WebSocket connection established successfully!\n";

    // Read initial snapshot delivered over WebSocket
    std::string ws_msg;
    auto read_res = ws_cli.read(ws_msg);
    if (read_res != httplib::ws::ReadResult::Text) {
        std::cerr << "[FAIL] Failed to read initial snapshot from WebSocket\n";
        return 1;
    }
    std::cout << "Received initial snapshot over WebSocket (" << ws_msg.size() << " bytes)\n";
    if (ws_msg.find("Signal Controls") == std::string::npos) {
        std::cerr << "[FAIL] Initial WebSocket snapshot does not contain window title\n";
        return 1;
    }
    std::cout << ">>> [PASS] Initial DOM snapshot verified over WebSocket!\n";

    // Send click and slider events over WebSocket
    std::cout << "[Step 5] Sending Button Click and Slider Drag over WebSocket...\n";
    std::string ws_click = "{\"type\":\"click\",\"id\":" + std::to_string(btn_id) + "}";
    if (!ws_cli.send(ws_click)) {
        std::cerr << "[FAIL] Failed to send click over WebSocket\n";
        return 1;
    }
    std::string ws_slider = "{\"type\":\"slider\",\"id\":" + std::to_string(slider_id) + ",\"val\":77.5}";
    if (!ws_cli.send(ws_slider)) {
        std::cerr << "[FAIL] Failed to send slider over WebSocket\n";
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // -----------------------------------------------------------------
    // Frame 2: Process browser events in ImGui
    // -----------------------------------------------------------------
    std::cout << "[Step 6] Rendering Frame 2 (Consuming WebSocket Events)...\n";
    ImGuiDom::BeginFrame(2);
    ImGui::NewFrame();

    bool frame2_clicked = false;
    ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::Begin("Signal Controls", nullptr);
    if (ImGui::Button("Reset Calibration")) {
        frame2_clicked = true;
    }
    float browser_val = 0.0f;
    if (ImGuiDom::DomContext::Instance().ConsumeSlider(slider_id, browser_val)) {
        test_freq = browser_val;
    }
    ImGui::SliderFloat("Frequency", &test_freq, 1.0f, 100.0f);
    ImGui::End();

    ImGui::Render();
    ImGuiDom::EndFrame();

    // Verify WebSocket receives the updated Frame 2 snapshot broadcast
    auto read_frame2 = ws_cli.read(ws_msg);
    if (read_frame2 == httplib::ws::ReadResult::Text && ws_msg.find("\"frame\":2") != std::string::npos) {
        std::cout << ">>> [PASS] Real-time Frame 2 snapshot received over WebSocket broadcast!\n";
    }
    ws_cli.close();

    std::cout << "Button clicked result in C++: " << (frame2_clicked ? "true" : "false") << "\n";
    std::cout << "Slider value in C++: " << test_freq << " (Expected: 77.5)\n";

    if (!frame2_clicked) {
        std::cerr << "[FAIL] WebSocket click event was not consumed by ImGui::Button!\n";
        return 1;
    }
    if (std::abs(test_freq - 77.5f) > 0.1f) {
        std::cerr << "[FAIL] WebSocket slider event was not applied to C++ variable!\n";
        return 1;
    }

    std::cout << "\n>>> [PASS] Full-Duplex WebSocket verified between Browser client and C++ ImGui!\n";

    ImGuiDom::StopServer();
    ImGui::DestroyContext();

    std::cout << "========================================================\n";
    std::cout << " All DOM backend tests passed successfully!\n";
    std::cout << "========================================================\n";
    return 0;
}
