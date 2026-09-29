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
static bool s_highPrecision = true;
static float s_speedValue = 10.0f;
static std::string s_searchQuery = "";
static std::string s_sampleName = "Specimen-Titanium-A4";
static float s_targetLoad = 75.0f;
static bool s_confirmModalOpen = false;

struct TestItem {
    int id;
    std::string name;
    std::string standard;
    double maxForce;
    double elongation;
    UiVariant status;
    const char* statusText;
};

static std::vector<TestItem> s_testItems;

void Init() {
    if (s_initialized) return;
    s_initialized = true;

    UiTheme& theme = UiTheme::Get();
    theme.LoadFonts();
    theme.SetMode(ThemeMode::Dark);

    s_realtimeChart.SetAxisPair(RealtimeAxisPair::Force_Displacement);
    s_realtimeChart.SetLineThickness(2.2f);
    s_realtimeChart.SetHeadMarker(true);

    s_testItems = {
        { 101, "Specimen-Ti-01", "ASTM E8", 124.5, 14.2, UiVariant::Success, "PASSED" },
        { 102, "Specimen-Ti-02", "ASTM E8", 126.1, 13.9, UiVariant::Success, "PASSED" },
        { 103, "Specimen-Al-7075", "ISO 6892-1", 88.4, 8.5, UiVariant::Warning, "MARGINAL" },
        { 104, "Specimen-CFRP-01", "ISO 527-4", 210.8, 2.1, UiVariant::Primary, "TESTING" },
        { 105, "Specimen-Steel-316L", "EN 10002", 95.2, 22.0, UiVariant::Success, "PASSED" },
        { 106, "Specimen-Polymer-09", "ASTM D638", 12.3, 45.0, UiVariant::Danger, "FAILED" },
    };
}

void RenderUI(float main_scale) {
    if (!s_initialized) {
        Init();
    }

    UiTheme& theme = UiTheme::Get();
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - s_startTime).count();

    // Stream real-time curve data
    if (s_liveFeed) {
        double dt = std::chrono::duration<double>(now - s_lastSampleTime).count();
        if (dt >= 0.03) { // ~30 Hz telemetry
            s_lastSampleTime = now;
            double d = fmod(elapsed * 2.5, 30.0);
            // Non-linear stress-strain curve
            double f = 140.0 * (1.0 - std::exp(-d / 4.0)) + 8.0 * std::sin(d * 1.5);
            if (d < 0.1) s_realtimeChart.Clear();
            s_realtimeChart.AppendSample(elapsed, f, d);
        }
    }

    // 1. TOP BRANDING & THEME CONTROLS
    {
        ImVec2 curPos = ImGui::GetCursorScreenPos();
        Icon(Icon::Target).DrawAt(ImGui::GetWindowDrawList(), ImVec2(curPos.x + 2, curPos.y + 4), 22.0f * main_scale, theme.palette.accent);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 32.0f * main_scale);
        ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "OmniKit");
        ImGui::SameLine();
        ImGui::TextColored(ImColor(theme.palette.accent).Value, "Design System & Widget Kit");
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

    // 2. DEVICE TELEMETRY STRIP & REALTIME INDICATORS
    {
        // Device status banner
        DeviceStatus::Render("Tension Tester Pro 50kN",
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
        double liveForce = s_liveFeed ? (82.4 + 3.2 * std::sin(elapsed * 2.0)) : 0.0;
        double liveDisp = s_liveFeed ? fmod(elapsed * 2.5, 30.0) : 0.0;

        Indicator indForce("LOAD CELL", liveForce, 2, "kN", 5, true);
        indForce.Render();

        ImGui::SameLine();
        Indicator indDisp("CROSSHEAD", liveDisp, 3, "mm", 6, true);
        indDisp.Render();

        ImGui::SameLine();
        Indicator indTime("TEST DURATION", elapsed, 1, "s", 4, false);
        indTime.Render();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 3. MAIN WORKSPACE: Two-column grid (Left: Form/Cards, Right: Real-time Curve & Table)
    float availW = ImGui::GetContentRegionAvail().x;
    float colLeftW = (availW - 16.0f * main_scale) * 0.44f;
    float colRightW = availW - colLeftW - 16.0f * main_scale;

    // LEFT COLUMN: Cards & Controls
    ImGui::BeginChild("LeftCol", ImVec2(colLeftW, 0), false, ImGuiWindowFlags_None);
    {
        // Card 1: Machine Settings & Test Configuration
        if (Card card("cfg_card", "Test Configuration"); card) {
            // Sample name input
            InputField::Text("##spec_name", "Specimen Identifier", s_sampleName);

            ImGui::Spacing();
            InputField::Float("##target_load", "Target Load Limit", s_targetLoad, "kN");

            ImGui::Spacing();
            // Toggle controls
            Toggle::Render("tog_tare", s_autoTare, "Automatic Tare on Start", "Zeros displacement before grip tensioning");
            Toggle::Render("tog_stream", s_liveFeed, "Telemetry Streaming", "Sends real-time high-rate samples to chart");
            Toggle::Render("tog_prec", s_highPrecision, "High Precision ADC Filter", "Enables 24-bit oversampling pipeline");

            ImGui::Spacing();
            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Testing Speed Preset:");
            static const std::vector<float> speedOptions = { 1.0f, 5.0f, 10.0f, 50.0f };
            PresetGrid::Render(s_speedValue, speedOptions, "mm/min", 0.0f, 4);

            ImGui::Spacing();
            if (card.Button("Execute Pre-Flight Check", Icon(Icon::Check), UiVariant::Primary)) {
                s_deviceState = DeviceState::Running;
                s_liveFeed = true;
            }
        }

        ImGui::Spacing();

        // Card 2: Semantic Design Tokens & Button Gallery
        if (Card card2("tokens_card", "Semantic Design Tokens"); card2) {
            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Color Variants (UiVariant):");
            
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
            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Badges & Metadata Tags:");
            Badge::Render("ACTIVE", UiVariant::Success);
            ImGui::SameLine();
            Badge::Render("CALIBRATED", UiVariant::Primary);
            ImGui::SameLine();
            Badge::Render("OFFLINE", UiVariant::Secondary);
            ImGui::SameLine();
            Badge::Render("OVERLOAD", UiVariant::Danger);
            ImGui::SameLine();
            Tag::Render("ISO-6892");
            ImGui::SameLine();
            Tag::Render("ASTM-E8");
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // RIGHT COLUMN: High-Speed Realtime Chart & Virtual Data Table
    ImGui::BeginChild("RightCol", ImVec2(colRightW, 0), false, ImGuiWindowFlags_None);
    {
        // 1. High-Performance Realtime Chart
        if (Card chartCard("chart_card", "Real-Time Telemetry Curve (Force vs Displacement)"); chartCard) {
            float chartH = 260.0f * main_scale;
            s_realtimeChart.Render("realtime_chart_view", ImVec2(colRightW - 32.0f * main_scale, chartH));
        }

        ImGui::Spacing();

        // 2. Data Records Table
        if (Card tableCard("records_card", "Archived Test Batches"); tableCard) {
            SearchInput::Render("tbl_search", s_searchQuery, "Search tests...");
            ImGui::Spacing();

            if (ImGui::BeginTable("TestDataTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 50.0f * main_scale);
                ImGui::TableSetupColumn("Specimen Name");
                ImGui::TableSetupColumn("Standard");
                ImGui::TableSetupColumn("Max Force (kN)");
                ImGui::TableSetupColumn("Status");
                ImGui::TableHeadersRow();

                for (const auto& item : s_testItems) {
                    if (!s_searchQuery.empty() && item.name.find(s_searchQuery) == std::string::npos) {
                        continue;
                    }
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", item.id);
                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", item.name.c_str());
                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "%s", item.standard.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%.1f kN", item.maxForce);
                    ImGui::TableNextColumn();
                    Badge::Render(item.statusText, item.status);
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
                                  "Reset Telemetry and Session Data",
                                  "Are you sure you want to clear all active telemetry points and buffer history?",
                                  "Current curve data will be discarded.")) {
            s_realtimeChart.Clear();
            s_startTime = std::chrono::steady_clock::now();
        }
    }
}

} // namespace OmniKitShowcase
