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
    if (json.find("\"val\":\"Status: Active 60 FPS\"") == std::string::npos) {
        std::cerr << "[FAIL] Static text not found in DOM JSON\n";
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

    // -----------------------------------------------------------------
    // Step 7: Pointer / Keyboard Event Forwarding directly to ImGuiIO
    // -----------------------------------------------------------------
    std::cout << "[Step 7] Testing Pointer and Keyboard event forwarding to ImGuiIO...\n";
    httplib::ws::WebSocketClient ws_cli2("ws://127.0.0.1:" + std::to_string(test_port) + "/ws");
    ws_cli2.set_read_timeout(std::chrono::seconds(2));
    if (ws_cli2.connect()) {
        // Frame 3: Mouse move and Mouse down
        ws_cli2.send("{\"type\":\"mouse_move\",\"x\":120.0,\"y\":110.0}");
        ws_cli2.send("{\"type\":\"mouse_down\",\"button\":0,\"x\":120.0,\"y\":110.0}");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        ImGuiDom::BeginFrame(3);
        ImGui::NewFrame();

        ImGuiIO& io_check = ImGui::GetIO();
        std::cout << "ImGuiIO MousePos: (" << io_check.MousePos.x << ", " << io_check.MousePos.y << ")\n";
        std::cout << "ImGuiIO MouseDown[0]: " << (io_check.MouseDown[0] ? "true" : "false") << "\n";

        if (std::abs(io_check.MousePos.x - 120.0f) > 0.1f || std::abs(io_check.MousePos.y - 110.0f) > 0.1f) {
            std::cerr << "[FAIL] MousePos was not forwarded to ImGuiIO!\n";
            return 1;
        }
        if (!io_check.MouseDown[0]) {
            std::cerr << "[FAIL] MouseDown[0] was not forwarded to ImGuiIO!\n";
            return 1;
        }

        ImGui::Render();
        ImGuiDom::EndFrame();

        // Frame 4: Mouse wheel and character input
        ws_cli2.send("{\"type\":\"mouse_wheel\",\"dx\":1.0,\"dy\":-3.5}");
        ws_cli2.send("{\"type\":\"char\",\"text\":\"W\"}");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        ImGuiDom::BeginFrame(4);
        ImGui::NewFrame();

        std::cout << "ImGuiIO MouseWheel: " << io_check.MouseWheel << "\n";
        if (std::abs(io_check.MouseWheel - (-3.5f)) > 0.1f) {
            std::cerr << "[FAIL] MouseWheel was not forwarded to ImGuiIO!\n";
            return 1;
        }

        ImGui::Render();
        ImGuiDom::EndFrame();

        // Frame 5: Mouse up release
        ws_cli2.send("{\"type\":\"mouse_up\",\"button\":0,\"x\":120.0,\"y\":110.0}");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        ImGuiDom::BeginFrame(5);
        ImGui::NewFrame();
        if (io_check.MouseDown[0]) {
            std::cerr << "[FAIL] MouseUp[0] was not released in ImGuiIO!\n";
            return 1;
        }
        ImGui::Render();
        ImGuiDom::EndFrame();
        ws_cli2.close();
        std::cout << ">>> [PASS] Native ImGuiIO pointer and keyboard events verified!\n";
    }

    // -----------------------------------------------------------------
    // Step 8: Full Widget Palette Test (Radio, InputText, Combo, Progress, Separator)
    // -----------------------------------------------------------------
    std::cout << "[Step 8] Testing Full Widget Palette in DOM tree...\n";
    ImGuiDom::BeginFrame(6);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(400, 400));
    ImGui::Begin("Full Palette", nullptr);

    uint32_t radio_id = static_cast<uint32_t>(ImGui::GetID("Channel A"));
    uint32_t input_id = static_cast<uint32_t>(ImGui::GetID("Config Path"));
    uint32_t combo_id = static_cast<uint32_t>(ImGui::GetID("Trigger Mode"));
    uint32_t prog_id  = static_cast<uint32_t>(ImGui::GetID("Download Progress"));

    ImGuiDom::DomContext::Instance().RecordRadioButton(radio_id, "Channel A", true, 60, 80, 100, 20);
    ImGuiDom::DomContext::Instance().RecordInputText(input_id, "Config Path", "/etc/signal.conf", 60, 110, 250, 24);
    ImGuiDom::DomContext::Instance().RecordCombo(combo_id, "Trigger Mode", "Rising Edge", false, 60, 140, 200, 24);
    ImGuiDom::DomContext::Instance().RecordProgressBar(prog_id, 0.75f, "75%", 60, 170, 250, 20);
    ImGuiDom::DomContext::Instance().RecordSeparator(9999, 60, 200, 280, 2);

    ImGui::End();
    ImGui::Render();
    ImGuiDom::EndFrame();

    std::string palette_json = ImGuiDom::DomContext::Instance().GetLatestJson();
    std::cout << "Palette DOM JSON: " << palette_json << "\n";

    if (palette_json.find("\"type\":\"radio\"") == std::string::npos) {
        std::cerr << "[FAIL] Radio button not found in DOM JSON\n";
        return 1;
    }
    if (palette_json.find("\"type\":\"input\"") == std::string::npos) {
        std::cerr << "[FAIL] Input text not found in DOM JSON\n";
        return 1;
    }
    if (palette_json.find("\"type\":\"combo\"") == std::string::npos) {
        std::cerr << "[FAIL] Combo select not found in DOM JSON\n";
        return 1;
    }
    if (palette_json.find("\"type\":\"progress\"") == std::string::npos) {
        std::cerr << "[FAIL] Progress bar not found in DOM JSON\n";
        return 1;
    }
    if (palette_json.find("\"type\":\"separator\"") == std::string::npos) {
        std::cerr << "[FAIL] Separator not found in DOM JSON\n";
        return 1;
    }
    std::cout << ">>> [PASS] Full widget palette verified in Web DOM tree!\n";

    // Test text input change from browser event
    ImGuiDom::BrowserEvent input_evt;
    input_evt.type = "input";
    input_evt.id = input_id;
    input_evt.value_str = "/opt/custom.conf";
    ImGuiDom::DomContext::Instance().PushBrowserEvent(input_evt);

    ImGuiDom::BeginFrame(7);
    ImGui::NewFrame();

    std::string consumed_text;
    bool input_consumed = ImGuiDom::DomContext::Instance().ConsumeInputText(input_id, consumed_text);
    if (!input_consumed || consumed_text != "/opt/custom.conf") {
        std::cerr << "[FAIL] ConsumeInputText did not receive updated text!\n";
        return 1;
    }
    std::cout << ">>> [PASS] InputText two-way sync verified!\n";

    ImGui::Render();
    ImGuiDom::EndFrame();

    // -----------------------------------------------------------------
    // Step 9: TabBar, TabItem, and Tooltip Support
    // -----------------------------------------------------------------
    std::cout << "[Step 9] Testing TabBar, TabItem, and Tooltip support...\n";
    ImGuiDom::BeginFrame(8);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::Begin("TabWindow", nullptr);

    if (ImGuiDom::BeginTabBar("MyTabBar")) {
        if (ImGuiDom::BeginTabItem("General")) {
            ImGuiDom::Text("General Settings Content");
            ImGuiDom::SetItemTooltip("Helpful info about General Settings");
            ImGuiDom::EndTabItem();
        }
        if (ImGuiDom::BeginTabItem("Advanced")) {
            ImGuiDom::Text("Advanced Settings Content");
            ImGuiDom::EndTabItem();
        }
        ImGuiDom::EndTabBar();
    }
    ImGui::End();

    // Native floating tooltip
    ImGui::BeginTooltip();
    ImGui::Text("Floating Tooltip Notice");
    ImGui::EndTooltip();

    ImGui::Render();
    ImGuiDom::EndFrame();

    std::string tab_json = ImGuiDom::DomContext::Instance().GetLatestJson();
    std::cout << "Tab/Tooltip DOM JSON: " << tab_json << "\n";

    if (tab_json.find("\"type\":\"tabbar\"") == std::string::npos) {
        std::cerr << "[FAIL] TabBar not found in DOM JSON!\n";
        return 1;
    }
    if (tab_json.find("\"type\":\"tabitem\"") == std::string::npos) {
        std::cerr << "[FAIL] TabItem not found in DOM JSON!\n";
        return 1;
    }
    if (tab_json.find("\"tooltip\":\"Helpful info about General Settings\"") == std::string::npos) {
        std::cerr << "[FAIL] Item tooltip not found in DOM JSON element!\n";
        return 1;
    }
    if (tab_json.find("\"is_tooltip\":true") == std::string::npos) {
        std::cerr << "[FAIL] Tooltip window flag not set in DOM JSON!\n";
        return 1;
    }
    std::cout << ">>> [PASS] TabBar, TabItem, and Tooltip export verified!\n";

    // Test tab switching via browser event using the element ID from the DOM snapshot
    size_t adv_pos = tab_json.find("\"label\":\"Advanced\"");
    size_t id_pos = tab_json.rfind("\"id\":", adv_pos);
    uint32_t adv_tab_id = static_cast<uint32_t>(std::stoul(tab_json.substr(id_pos + 5)));
    std::cout << "Target Advanced Tab ID: " << adv_tab_id << "\n";

    ImGuiDom::BrowserEvent tab_evt;
    tab_evt.type = "tab";
    tab_evt.id = adv_tab_id;
    ImGuiDom::DomContext::Instance().PushBrowserEvent(tab_evt);

    // Frame 9: Consume tab event and queue activation
    ImGuiDom::BeginFrame(9);
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::Begin("TabWindow", nullptr);

    if (ImGuiDom::BeginTabBar("MyTabBar")) {
        if (ImGuiDom::BeginTabItem("General")) {
            ImGuiDom::EndTabItem();
        }
        if (ImGuiDom::BeginTabItem("Advanced")) {
            ImGuiDom::EndTabItem();
        }
        ImGuiDom::EndTabBar();
    }
    ImGui::End();
    ImGui::Render();
    ImGuiDom::EndFrame();

    // Frame 10: TabBar has applied layout and selected tab is now active!
    ImGuiDom::BeginFrame(10);
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::Begin("TabWindow", nullptr);

    bool adv_selected = false;
    if (ImGuiDom::BeginTabBar("MyTabBar")) {
        if (ImGuiDom::BeginTabItem("General")) {
            ImGuiDom::EndTabItem();
        }
        if (ImGuiDom::BeginTabItem("Advanced")) {
            adv_selected = true;
            ImGuiDom::Text("Inside Advanced Tab!");
            ImGuiDom::EndTabItem();
        }
        ImGuiDom::EndTabBar();
    }
    ImGui::End();
    ImGui::Render();
    ImGuiDom::EndFrame();

    if (!adv_selected) {
        std::cerr << "[FAIL] Advanced tab was not selected after browser tab event!\n";
        return 1;
    }
    std::string tab_switch_json = ImGuiDom::DomContext::Instance().GetLatestJson();
    if (tab_switch_json.find("Inside Advanced Tab!") == std::string::npos) {
        std::cerr << "[FAIL] Tab content did not switch to Advanced tab!\n";
        return 1;
    }
    std::cout << ">>> [PASS] Full-duplex Tab switching verified!\n";

    // -----------------------------------------------------------------
    // Step 10: Table and ListBox Support
    // -----------------------------------------------------------------
    std::cout << "[Step 10] Testing Table and ListBox support...\n";
    ImGuiDom::BeginFrame(11);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(500, 400));
    ImGui::Begin("TableAndListBox", nullptr);

    if (ImGuiDom::BeginTable("MetricsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGuiDom::TableSetupColumn("Channel");
        ImGuiDom::TableSetupColumn("Signal");
        ImGuiDom::TableSetupColumn("Status");
        ImGuiDom::TableHeadersRow();

        ImGuiDom::TableNextRow();
        ImGuiDom::TableNextColumn(); ImGuiDom::Text("CH-1");
        ImGuiDom::TableNextColumn(); ImGuiDom::Text("Sine 1kHz");
        ImGuiDom::TableNextColumn(); ImGuiDom::Text("Active");

        ImGuiDom::TableNextRow();
        ImGuiDom::TableNextColumn(); ImGuiDom::Text("CH-2");
        ImGuiDom::TableNextColumn(); ImGuiDom::Text("Square 500Hz");
        ImGuiDom::TableNextColumn(); ImGuiDom::Text("Idle");

        ImGuiDom::EndTable();
    }

    static int selected_theme = 1;
    const char* themes[] = { "Classic Dark", "Solarized", "Monokai", "Light" };
    ImGuiDom::ListBox("Color Theme", &selected_theme, themes, 4);

    ImGui::End();
    ImGui::Render();
    ImGuiDom::EndFrame();

    std::string tbl_json = ImGuiDom::DomContext::Instance().GetLatestJson();
    std::cout << "Table/ListBox DOM JSON: " << tbl_json << "\n";

    if (tbl_json.find("\"type\":\"table\"") == std::string::npos) {
        std::cerr << "[FAIL] Table element not found in DOM JSON!\n";
        return 1;
    }
    if (tbl_json.find("\"columns\":[\"Channel\",\"Signal\",\"Status\"]") == std::string::npos) {
        std::cerr << "[FAIL] Table columns not properly exported in DOM JSON!\n";
        return 1;
    }
    if (tbl_json.find("\"type\":\"listbox\"") == std::string::npos) {
        std::cerr << "[FAIL] ListBox element not found in DOM JSON!\n";
        return 1;
    }
    if (tbl_json.find("\"selected_idx\":1") == std::string::npos) {
        std::cerr << "[FAIL] ListBox selected_idx:1 not found in DOM JSON!\n";
        return 1;
    }
    std::cout << ">>> [PASS] Table and ListBox export verified!\n";

    // Test ListBox event two-way sync using element ID from DOM snapshot
    size_t lb_pos = tbl_json.find("\"label\":\"Color Theme\"");
    size_t lb_id_pos = tbl_json.rfind("\"id\":", lb_pos);
    uint32_t listbox_id = static_cast<uint32_t>(std::stoul(tbl_json.substr(lb_id_pos + 5)));
    std::cout << "Target ListBox ID: " << listbox_id << "\n";

    ImGuiDom::BrowserEvent lb_evt;
    lb_evt.type = "listbox";
    lb_evt.id = listbox_id;
    lb_evt.value_num = 2.0f; // Select "Monokai"
    ImGuiDom::DomContext::Instance().PushBrowserEvent(lb_evt);

    ImGuiDom::BeginFrame(12);
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(500, 400));
    ImGui::Begin("TableAndListBox", nullptr);

    ImGuiDom::ListBox("Color Theme", &selected_theme, themes, 4);

    ImGui::End();
    ImGui::Render();
    ImGuiDom::EndFrame();

    if (selected_theme != 2) {
        std::cerr << "[FAIL] ListBox selected_theme in C++: " << selected_theme << " (Expected: 2)\n";
        return 1;
    }
    std::string lb_json = ImGuiDom::DomContext::Instance().GetLatestJson();
    if (lb_json.find("\"selected_idx\":2") == std::string::npos) {
        std::cerr << "[FAIL] ListBox updated selected_idx:2 not found in DOM JSON!\n";
        return 1;
    }
    std::cout << ">>> [PASS] Table and ListBox two-way sync verified!\n";

    // -----------------------------------------------------------------
    // Step 11: ColorEdit & InputTextMultiline Widgets Test
    // -----------------------------------------------------------------
    std::cout << "[Step 11] Testing ColorEdit and InputTextMultiline widgets...\n";
    ImGuiDom::BeginFrame(13);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(500, 450));
    ImGui::Begin("ColorAndMultiline", nullptr);

    static float trace_color[3] = { 0.2f, 0.8f, 0.4f };
    ImGuiDom::ColorEdit3("Trace Color", trace_color);

    static char notes_buf[256] = "Initial notes";
    ImGuiDom::InputTextMultiline("Notes Editor", notes_buf, sizeof(notes_buf), ImVec2(350, 80));

    ImGui::End();
    ImGui::Render();
    ImGuiDom::EndFrame();

    std::string color_json = ImGuiDom::DomContext::Instance().GetLatestJson();
    std::cout << "Color/Multiline DOM JSON: " << color_json << "\n";

    if (color_json.find("\"type\":\"coloredit\"") == std::string::npos) {
        std::cerr << "[FAIL] ColorEdit element not found in DOM JSON!\n";
        return 1;
    }
    if (color_json.find("\"label\":\"Trace Color\"") == std::string::npos) {
        std::cerr << "[FAIL] Trace Color label not found in DOM JSON!\n";
        return 1;
    }
    if (color_json.find("\"type\":\"textarea\"") == std::string::npos) {
        std::cerr << "[FAIL] InputTextMultiline element not found in DOM JSON!\n";
        return 1;
    }
    if (color_json.find("\"val\":\"Initial notes\"") == std::string::npos) {
        std::cerr << "[FAIL] InputTextMultiline initial value not found in DOM JSON!\n";
        return 1;
    }
    std::cout << ">>> [PASS] ColorEdit and InputTextMultiline export verified!\n";

    // Test two-way synchronization from browser events
    size_t col_lbl_pos = color_json.find("\"label\":\"Trace Color\"");
    size_t col_id_pos = color_json.rfind("\"id\":", col_lbl_pos);
    uint32_t color_id = static_cast<uint32_t>(std::stoul(color_json.substr(col_id_pos + 5)));

    size_t notes_lbl_pos = color_json.find("\"label\":\"Notes Editor\"");
    size_t notes_id_pos = color_json.rfind("\"id\":", notes_lbl_pos);
    uint32_t notes_id = static_cast<uint32_t>(std::stoul(color_json.substr(notes_id_pos + 5)));

    std::cout << "Target ColorEdit ID: " << color_id << ", Multiline ID: " << notes_id << "\n";

    // 1. Send color event: #ff0080 (r=1.0, g=0.0, b=0.502)
    ImGuiDom::BrowserEvent col_evt;
    col_evt.type = "color";
    col_evt.id = color_id;
    col_evt.value_str = "#ff0080";
    ImGuiDom::DomContext::Instance().PushBrowserEvent(col_evt);

    // 2. Send multiline text event
    ImGuiDom::BrowserEvent txt_evt;
    txt_evt.type = "input_multiline";
    txt_evt.id = notes_id;
    txt_evt.value_str = "Line 1: OK\nLine 2: Running";
    ImGuiDom::DomContext::Instance().PushBrowserEvent(txt_evt);

    // Frame 14: Consume browser events in next Dear ImGui frame
    ImGuiDom::BeginFrame(14);
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(500, 450));
    ImGui::Begin("ColorAndMultiline", nullptr);

    ImGuiDom::ColorEdit3("Trace Color", trace_color);
    ImGuiDom::InputTextMultiline("Notes Editor", notes_buf, sizeof(notes_buf), ImVec2(350, 80));

    ImGui::End();
    ImGui::Render();
    ImGuiDom::EndFrame();

    // Verify trace_color updated in C++
    if (std::abs(trace_color[0] - 1.0f) > 0.05f || std::abs(trace_color[1] - 0.0f) > 0.05f || std::abs(trace_color[2] - 0.502f) > 0.05f) {
        std::cerr << "[FAIL] trace_color not updated properly: {" << trace_color[0] << ", " << trace_color[1] << ", " << trace_color[2] << "}\n";
        return 1;
    }

    if (std::string(notes_buf).find("Line 1: OK") == std::string::npos) {
        std::cerr << "[FAIL] notes_buf not updated in C++: " << notes_buf << "\n";
        return 1;
    }

    std::string updated_color_json = ImGuiDom::DomContext::Instance().GetLatestJson();
    if (updated_color_json.find("\"val\":\"#ff0080\"") == std::string::npos) {
        std::cerr << "[FAIL] Updated color #ff0080 not reflected in DOM JSON!\n";
        return 1;
    }
    std::cout << ">>> [PASS] ColorEdit and InputTextMultiline two-way sync verified!\n";

    // -----------------------------------------------------------------
    // Step 12: CollapsingHeader and TreeNode Verification
    // -----------------------------------------------------------------
    std::cout << "[Step 12] Testing CollapsingHeader and TreeNode distinction...\n";
    ImGuiDom::BeginFrame(15);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::Begin("Tree Window", nullptr);
    if (ImGui::CollapsingHeader("System Diagnostics")) {
        ImGui::Text("All systems nominal");
        if (ImGui::TreeNode("Subsystem Details")) {
            ImGui::Text("Core 0: OK");
            ImGui::TreePop();
        }
    }
    ImGui::End();

    ImGui::Render();
    ImGuiDom::EndFrame();

    std::string header_json = ImGuiDom::DomContext::Instance().GetLatestJson();
    std::cout << "Tree/Header DOM JSON: " << header_json << "\n";
    if (header_json.find("\"type\":\"collapsing_header\"") == std::string::npos) {
        std::cerr << "[FAIL] CollapsingHeader not found in DOM JSON!\n";
        return 1;
    }
    if (header_json.find("\"label\":\"System Diagnostics\"") == std::string::npos) {
        std::cerr << "[FAIL] CollapsingHeader label not found in DOM JSON!\n";
        return 1;
    }
    std::cout << ">>> [PASS] CollapsingHeader export verified!\n";

    // -----------------------------------------------------------------
    std::cout << "[Step 13] Testing window_move and window_focus event synchronization...\n";
    httplib::ws::WebSocketClient ws_cli3("ws://127.0.0.1:" + std::to_string(test_port) + "/ws");
    ws_cli3.set_read_timeout(std::chrono::seconds(2));
    if (ws_cli3.connect()) {
        // Send window_move to relocate "Tree Window" to (140, 220)
        ws_cli3.send("{\"type\":\"window_move\",\"title\":\"Tree Window\",\"x\":140.0,\"y\":220.0}");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        ImGuiDom::BeginFrame(16);
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Tree Window", nullptr);
        ImGui::Text("Relocated content");
        ImGui::End();

        ImGui::Render();
        ImGuiDom::EndFrame();

        std::string move_json = ImGuiDom::DomContext::Instance().GetLatestJson();
        std::cout << "Moved Window DOM JSON: " << move_json << "\n";
        if (move_json.find("\"x\":140") == std::string::npos || move_json.find("\"y\":220") == std::string::npos) {
            std::cerr << "[FAIL] window_move position (140, 220) not reflected in ImGui window!\n";
            return 1;
        }
        ws_cli3.close();
        std::cout << ">>> [PASS] window_move synchronization verified!\n";
    }

    // -----------------------------------------------------------------
    // Step 14: Window Stacking Order and Focus Synchronization Verification
    // -----------------------------------------------------------------
    std::cout << "[Step 14] Testing Window Stacking Order and Focus Synchronization...\n";
    httplib::ws::WebSocketClient ws_cli4("ws://127.0.0.1:" + std::to_string(test_port) + "/ws");
    ws_cli4.set_read_timeout(std::chrono::seconds(2));
    if (ws_cli4.connect()) {
        // Frame 17: Create two windows in order: Window Alpha, Window Beta
        ImGuiDom::BeginFrame(17);
        ImGui::NewFrame();
        ImGui::Begin("Window Alpha", nullptr);
        ImGui::Text("Alpha Content");
        ImGui::End();
        ImGui::Begin("Window Beta", nullptr);
        ImGui::Text("Beta Content");
        ImGui::End();
        ImGui::Render();
        ImGuiDom::EndFrame();

        std::string initial_order_json = ImGuiDom::DomContext::Instance().GetLatestJson();
        std::cout << "Initial Stacking JSON: " << initial_order_json << "\n";
        size_t pos_alpha = initial_order_json.find("\"title\":\"Window Alpha\"");
        size_t pos_beta = initial_order_json.find("\"title\":\"Window Beta\"");
        if (pos_alpha == std::string::npos || pos_beta == std::string::npos) {
            std::cerr << "[FAIL] Windows not found in initial DOM JSON!\n";
            return 1;
        }

        // Now activate Window Alpha via window_focus event over WebSocket
        ws_cli4.send("{\"type\":\"window_focus\",\"title\":\"Window Alpha\"}");
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        // Frame 18: Render both windows again
        ImGuiDom::BeginFrame(18);
        ImGui::NewFrame();
        ImGui::Begin("Window Alpha", nullptr);
        ImGui::Text("Alpha Content");
        ImGui::End();
        ImGui::Begin("Window Beta", nullptr);
        ImGui::Text("Beta Content");
        ImGui::End();
        ImGui::Render();
        ImGuiDom::EndFrame();

        std::string alpha_focused_json = ImGuiDom::DomContext::Instance().GetLatestJson();
        std::cout << "Alpha Focused JSON: " << alpha_focused_json << "\n";
        pos_alpha = alpha_focused_json.find("\"title\":\"Window Alpha\"");
        pos_beta = alpha_focused_json.find("\"title\":\"Window Beta\"");
        if (pos_alpha < pos_beta) {
            std::cerr << "[FAIL] Window Alpha not brought to front in DOM stacking order!\n";
            return 1;
        }
        std::cout << ">>> [PASS] Window Alpha successfully brought to front (display front)!\n";

        // Now activate Window Beta via window_focus
        ws_cli4.send("{\"type\":\"window_focus\",\"title\":\"Window Beta\"}");
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        // Frame 19: Render both windows again
        ImGuiDom::BeginFrame(19);
        ImGui::NewFrame();
        ImGui::Begin("Window Alpha", nullptr);
        ImGui::Text("Alpha Content");
        ImGui::End();
        ImGui::Begin("Window Beta", nullptr);
        ImGui::Text("Beta Content");
        ImGui::End();
        ImGui::Render();
        ImGuiDom::EndFrame();

        std::string beta_focused_json = ImGuiDom::DomContext::Instance().GetLatestJson();
        std::cout << "Beta Focused JSON: " << beta_focused_json << "\n";
        pos_alpha = beta_focused_json.find("\"title\":\"Window Alpha\"");
        pos_beta = beta_focused_json.find("\"title\":\"Window Beta\"");
        if (pos_beta < pos_alpha) {
            std::cerr << "[FAIL] Window Beta not brought to front in DOM stacking order!\n";
            return 1;
        }
        std::cout << ">>> [PASS] Window Beta successfully brought to front (display front)!\n";
        ws_cli4.close();
    }

    ImGuiDom::StopServer();
    ImGui::DestroyContext();

    std::cout << "========================================================\n";
    std::cout << " All DOM backend tests passed successfully!\n";
    std::cout << "========================================================\n";
    return 0;
}
