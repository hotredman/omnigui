#include "OmniKitShowcase.hpp"
#include "omnikit.hpp"
#include "imgui.h"

#include <cmath>
#include <chrono>
#include <string>
#include <vector>

namespace OmniKitShowcase {

// Demonstration state
static bool s_initialized = false;
static RealtimeChart s_realtimeChart;
static auto s_startTime = std::chrono::steady_clock::now();
static auto s_lastSampleTime = std::chrono::steady_clock::now();

static DeviceState s_deviceState = DeviceState::Idle;
static bool s_autoTare = false;
static bool s_liveFeed = true;
static bool s_hardwareAcc = true;
static float s_sampleInterval = 250.0f; // ms
static std::string s_searchQuery = "";
static std::string s_nodeIdentifier = "worker-node-cluster-07";
static float s_throughputLimit = 120.0f; // MB/s
static bool s_confirmModalOpen = false;

struct JobRecord {
    int id;
    std::string taskName;
    std::string engine;
    double executionTimeMs;
    double memoryUsageMb;
    UiVariant status;
    const char* statusText;
};

static std::vector<JobRecord> s_jobRecords;

void Init() {
    if (s_initialized) return;
    s_initialized = true;

    UiTheme& theme = UiTheme::Get();
    theme.LoadFonts();
    theme.SetMode(ThemeMode::Dark);

    s_realtimeChart.SetXAxis("Elapsed Time", "s");
    s_realtimeChart.SetYAxis("Throughput", "MB/s");
    s_realtimeChart.SetLineThickness(2.2f);
    s_realtimeChart.SetHeadMarker(true);

    s_jobRecords = {
        { 1001, "Telemetry Ingestion Pipeline", "ThorEngine v4", 12.4, 256.0, UiVariant::Success, "COMPLETED" },
        { 1002, "Vector Matrix Transformation", "SIMD-AVX512",   48.2, 512.5, UiVariant::Success, "COMPLETED" },
        { 1003, "Neural Weight Quantization",  "CoreCompute",   118.0, 1024.0, UiVariant::Primary, "PROCESSING" },
        { 1004, "Cache Index Rebalancing",     "LSM-Store",       6.5, 128.2, UiVariant::Success, "COMPLETED" },
        { 1005, "Distributed Lock Heartbeat",  "Raft-Cluster",    2.1,  64.0, UiVariant::Warning, "LATENCY SPIKE" },
        { 1006, "TLS Key Exchange Handshake",  "CryptoLib",       4.8,  32.0, UiVariant::Success, "COMPLETED" },
        { 1007, "Batch Data Compaction",       "ZSTD-Parallel", 230.1, 780.0, UiVariant::Danger,  "FAILED" },
    };
}

void RenderUI(float main_scale) {
    if (!s_initialized) {
        Init();
    }

    UiTheme& theme = UiTheme::Get();
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - s_startTime).count();

    // Stream synthetic real-time telemetry curve data (e.g. Throughput vs Time)
    if (s_liveFeed) {
        double dt = std::chrono::duration<double>(now - s_lastSampleTime).count();
        if (dt >= 0.033) { // ~30 Hz telemetry stream
            s_lastSampleTime = now;
            // Synthetic wave: Baseline + Harmonic oscillation + Gaussian noise burst
            double t = elapsed;
            double baseline = 65.0;
            double harmonics = 25.0 * std::sin(t * 1.8) + 12.0 * std::cos(t * 3.4);
            double burst = 15.0 * std::sin(t * 0.4);
            double throughput = std::max(5.0, baseline + harmonics + burst);

            if (s_realtimeChart.SampleCount() > 250) {
                s_realtimeChart.Clear();
            }
            s_realtimeChart.AppendSample(elapsed, throughput, 0.0);
        }
    }

    // 1. TOP BRANDING & THEME CONTROLS
    {
        ImVec2 curPos = ImGui::GetCursorScreenPos();
        Icon(Icon::Target).DrawAt(ImGui::GetWindowDrawList(), ImVec2(curPos.x + 2, curPos.y + 4), 22.0f * main_scale, theme.palette.accent);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 32.0f * main_scale);
        ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "OmniKit");
        ImGui::SameLine();
        ImGui::TextColored(ImColor(theme.palette.accent).Value, "UI Design System & Widget Kit");
        ImGui::SameLine();
        Badge::Render("v2.0", UiVariant::Primary);

        float rightToolsWidth = 240.0f * main_scale;
        float rightPos = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - rightToolsWidth;
        if (rightPos > ImGui::GetCursorPosX() + 20.0f) {
            ImGui::SameLine(rightPos);
        }
        bool isDark = (theme.mode == ThemeMode::Dark);
        if (Button::Render(isDark ? "Light Mode" : "Dark Mode",
                           UiVariant::Secondary,
                           isDark ? Icon(Icon::Sun) : Icon(Icon::Moon),
                           UiSize::Small)) {
            theme.SetMode(isDark ? ThemeMode::Light : ThemeMode::Dark);
        }

        ImGui::SameLine();
        if (Button::Render("Reset", UiVariant::Danger, Icon(Icon::Refresh), UiSize::Small)) {
            s_confirmModalOpen = true;
        }
    }

    ImGui::Spacing();

    // 2. SYSTEM STATUS STRIP & REAL-TIME INDICATORS
    {
        // Compute service status banner
        DeviceStatus::Render("Data Pipeline Node 07",
                             s_deviceState,
                             s_deviceState == DeviceState::Running ? "STREAMING (100 Hz)" : "STANDBY",
                             s_deviceState == DeviceState::Running ? "Pause" : "Start",
                             []() {
                                 if (s_deviceState == DeviceState::Running) {
                                     s_deviceState = DeviceState::Idle;
                                     s_liveFeed = false;
                                 } else {
                                     s_deviceState = DeviceState::Running;
                                     s_liveFeed = true;
                                 }
                             });

        ImGui::SameLine();

        // High-precision live digital indicators
        double liveThroughput = s_liveFeed ? (84.6 + 4.8 * std::sin(elapsed * 2.2)) : 0.0;
        double liveLatency = s_liveFeed ? (14.2 + 2.1 * std::cos(elapsed * 1.5)) : 0.0;

        Indicator indThroughput("THROUGHPUT", liveThroughput, 1, "MB/s", 5, true);
        indThroughput.Render();

        ImGui::SameLine();
        Indicator indLatency("AVG LATENCY", liveLatency, 2, "ms", 5, true);
        indLatency.Render();

        ImGui::SameLine();
        Indicator indUptime("UPTIME", elapsed, 1, "s", 4, false);
        indUptime.Render();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 3. MAIN WORKSPACE: Two-column grid (Left: Form/Cards, Right: Real-time Curve & Table)
    float availW = ImGui::GetContentRegionAvail().x;
    float colLeftW = (availW - 16.0f * main_scale) * 0.44f;
    float colRightW = availW - colLeftW - 16.0f * main_scale;

    // LEFT COLUMN: Configuration Cards & Semantic Controls
    ImGui::BeginChild("LeftCol", ImVec2(colLeftW, 0), false, ImGuiWindowFlags_None);
    {
        // Card 1: Pipeline Configuration
        if (Card card("cfg_card", "Pipeline Configuration"); card) {
            InputField::Text("##node_name", "Cluster Node Identifier", s_nodeIdentifier);

            ImGui::Spacing();
            InputField::Float("##throughput_limit", "Throughput Ceiling", s_throughputLimit, "MB/s");

            ImGui::Spacing();
            // Toggle controls
            Toggle::Render("tog_tare", s_autoTare, "Auto-Zero Telemetry Baseline", "Zeros relative baseline counter on cycle start");
            Toggle::Render("tog_stream", s_liveFeed, "Real-Time Telemetry Feed", "Streams continuous high-frequency metrics");
            Toggle::Render("tog_acc", s_hardwareAcc, "Hardware Acceleration (AVX-512)", "Enables vectorized math operations");

            ImGui::Spacing();
            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Sampling Interval Preset:");
            static const std::vector<float> sampleOptions = { 50.0f, 100.0f, 250.0f, 500.0f };
            PresetGrid::Render(s_sampleInterval, sampleOptions, "ms", 0.0f, 4);

            ImGui::Spacing();
            if (card.Button("Deploy Pipeline Configuration", Icon(Icon::Check), UiVariant::Primary)) {
                s_deviceState = DeviceState::Running;
                s_liveFeed = true;
            }
        }

        ImGui::Spacing();

        // Card 2: Semantic Design Tokens & Button Gallery
        if (Card card2("tokens_card", "Semantic Design Tokens"); card2) {
            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Interactive Variants (UiVariant):");
            
            // FlowLayout: 12-column responsive layout (3 columns per row)
            FlowLayout flow(colLeftW - 32.0f * main_scale);
            if (auto col = flow.Col(::Col::Third()); col) {
                Button::Render("Primary", UiVariant::Primary, Icon(Icon::Play), UiSize::Small);
            }
            if (auto col = flow.Col(::Col::Third()); col) {
                Button::Render("Success", UiVariant::Success, Icon(Icon::Check), UiSize::Small);
            }
            if (auto col = flow.Col(::Col::Third()); col) {
                Button::Render("Warning", UiVariant::Warning, Icon(Icon::Pause), UiSize::Small);
            }
            if (auto col = flow.Col(::Col::Third()); col) {
                Button::Render("Danger", UiVariant::Danger, Icon(Icon::Close), UiSize::Small);
            }
            if (auto col = flow.Col(::Col::Third()); col) {
                Button::Render("Info", UiVariant::Info, Icon(Icon::Target), UiSize::Small);
            }
            if (auto col = flow.Col(::Col::Third()); col) {
                Button::Render("Secondary", UiVariant::Secondary, Icon(Icon::Cog), UiSize::Small);
            }

            ImGui::Spacing();
            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Status Badges & Metadata Tags:");
            Badge::Render("ONLINE", UiVariant::Success);
            ImGui::SameLine();
            Badge::Render("READY", UiVariant::Primary);
            ImGui::SameLine();
            Badge::Render("STANDBY", UiVariant::Secondary);
            ImGui::SameLine();
            Badge::Render("ALERT", UiVariant::Danger);
            ImGui::SameLine();
            Tag::Render("HTTP/3");
            ImGui::SameLine();
            Tag::Render("AVX-512");
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // RIGHT COLUMN: High-Speed Realtime Chart & Virtual Data Table
    ImGui::BeginChild("RightCol", ImVec2(colRightW, 0), false, ImGuiWindowFlags_None);
    {
        // 1. High-Performance Realtime Chart
        if (Card chartCard("chart_card", "Real-Time Telemetry Stream (Throughput vs Elapsed Time)"); chartCard) {
            float chartH = 260.0f * main_scale;
            s_realtimeChart.Render("realtime_chart_view", ImVec2(colRightW - 32.0f * main_scale, chartH));
        }

        ImGui::Spacing();

        // 2. Data Records Table
        if (Card tableCard("records_card", "Distributed Job Execution Queue"); tableCard) {
            SearchInput::Render("tbl_search", s_searchQuery, "Filter jobs...");
            ImGui::Spacing();

            if (ImGui::BeginTable("JobDataTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 60.0f * main_scale);
                ImGui::TableSetupColumn("Task Identifier");
                ImGui::TableSetupColumn("Execution Engine");
                ImGui::TableSetupColumn("Latency (ms)");
                ImGui::TableSetupColumn("Status");
                ImGui::TableHeadersRow();

                for (const auto& job : s_jobRecords) {
                    if (!s_searchQuery.empty() && 
                        job.taskName.find(s_searchQuery) == std::string::npos &&
                        job.engine.find(s_searchQuery) == std::string::npos) {
                        continue;
                    }
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("#%d", job.id);
                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", job.taskName.c_str());
                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "%s", job.engine.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%.1f ms", job.executionTimeMs);
                    ImGui::TableNextColumn();
                    Badge::Render(job.statusText, job.status);
                }
                ImGui::EndTable();
            }
        }
    }
    ImGui::EndChild();

    // Confirm Modal dialog
    if (s_confirmModalOpen) {
        if (ConfirmDialog::Render("ConfirmResetModal",
                                  s_confirmModalOpen,
                                  "Reset Pipeline Buffer",
                                  "Are you sure you want to flush the active queue buffer?",
                                  "All active telemetry points and cache buffers will be cleared.",
                                  "Flush Buffer",
                                  UiVariant::Danger)) {
            s_realtimeChart.Clear();
            s_startTime = std::chrono::steady_clock::now();
        }
    }
}

} // namespace OmniKitShowcase
