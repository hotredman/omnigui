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

    ImGuiDom::Begin("Signal Controls", nullptr);
    btn_id = static_cast<uint32_t>(ImGui::GetID("Reset Calibration"));
    slider_id = static_cast<uint32_t>(ImGui::GetID("Frequency"));
    ImGuiDom::Text("Status: Active 60 FPS");
    if (ImGuiDom::Button("Reset Calibration")) {
        test_clicked = true;
    }
    ImGuiDom::SliderFloat("Frequency", &test_freq, 1.0f, 100.0f);
    ImGuiDom::End();

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
    // Browser Event Simulation: User clicks button in browser!
    // -----------------------------------------------------------------
    std::cout << "[Step 4] Simulating browser POST /api/event (Button Click & Slider Drag)...\n";
    std::string click_payload = "{\"type\":\"click\",\"id\":" + std::to_string(btn_id) + "}";
    std::cout << "Posting click to /api/event: " << click_payload << "\n";
    auto click_res = cli.Post("/api/event", click_payload, "application/json");
    if (!click_res) {
        std::cerr << "[FAIL] HTTP POST click failed with error code: " << (int)click_res.error() << "\n";
        return 1;
    }
    std::cout << "HTTP POST click status: " << click_res->status << "\n";

    std::string slider_payload = "{\"type\":\"slider\",\"id\":" + std::to_string(slider_id) + ",\"val\":77.5}";
    std::cout << "Posting slider to /api/event: " << slider_payload << "\n";
    auto slider_res = cli.Post("/api/event", slider_payload, "application/json");
    if (!slider_res) {
        std::cerr << "[FAIL] HTTP POST slider failed with error code: " << (int)slider_res.error() << "\n";
        return 1;
    }
    std::cout << "HTTP POST slider status: " << slider_res->status << "\n";



    // -----------------------------------------------------------------
    // Frame 2: Process browser events in ImGui
    // -----------------------------------------------------------------
    std::cout << "[Step 5] Rendering Frame 2 (Consuming Browser Events)...\n";
    ImGuiDom::BeginFrame(2);
    ImGui::NewFrame();

    bool frame2_clicked = false;
    ImGuiDom::Begin("Signal Controls", nullptr);
    if (ImGuiDom::Button("Reset Calibration")) {
        frame2_clicked = true;
    }
    ImGuiDom::SliderFloat("Frequency", &test_freq, 1.0f, 100.0f);
    ImGuiDom::End();

    ImGui::Render();
    ImGuiDom::EndFrame();

    std::cout << "Button clicked result in C++: " << (frame2_clicked ? "true" : "false") << "\n";
    std::cout << "Slider value in C++: " << test_freq << " (Expected: 77.5)\n";

    if (!frame2_clicked) {
        std::cerr << "[FAIL] Browser click event was not consumed by ImGuiDom::Button!\n";
        return 1;
    }
    if (std::abs(test_freq - 77.5f) > 0.1f) {
        std::cerr << "[FAIL] Browser slider event was not applied to C++ variable!\n";
        return 1;
    }

    std::cout << "\n>>> [PASS] Two-way synchronization verified between Browser DOM and C++ ImGui!\n";

    ImGuiDom::StopServer();
    ImGui::DestroyContext();

    std::cout << "========================================================\n";
    std::cout << " All DOM backend tests passed successfully!\n";
    std::cout << "========================================================\n";
    return 0;
}
