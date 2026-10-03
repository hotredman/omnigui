#include "OmniKitShowcase.hpp"
#include "omnikit.hpp"
#include <imgui.h>

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <string>
#include <vector>
#include <deque>
#include <ctime>
#include <algorithm>
#include <memory>

namespace OmniKitShowcase {

// Screen navigation enumeration (mirroring evo-machine-cpp NavScreen)
enum class NavScreen {
    RemoteControl = 0, // Live Stream & Pult controls
    Processing    = 1, // Offline Analysis & Metrics evaluation
    Machine       = 2, // System & Node Hardware Configuration
    Archive       = 3, // Data Archive
    Journal       = 4, // Event Journal
    Passport      = 5, // Node Passport / Specification
    Series        = 6  // Batch Analytics
};

// Test run record in sidebar
struct RecordedRun {
    std::string id;
    std::string title;
    std::string timestamp;
    double durationS;
    int pointCount;
    UiVariant status;
    const char* statusText;
    ImU32 ledColor;
    double peakThroughput;
    double avgLatency;
    double p99Latency;
    double efficiency;
    std::string anomaly;
};

// Journal entry record
struct JournalEntry {
    std::string timeStr;
    std::string level;
    std::string message;
    UiVariant variant;
};

// State variables
static bool s_initialized = false;
static NavScreen s_currentScreen = NavScreen::RemoteControl;
static int s_selectedRunIndex = 0;
static int s_indicatorsCount = 5;
static ConfirmModal s_resetConfirm({.title = "Reset stream?",
                                  .message = "Clears all live chart buffers and restarts the timeline.",
                                  .confirmLabel = "Reset"});
static bool s_scaleMenuOpen = false;
static bool s_countMenuOpen = false;

// Component instances (strictly decoupled, self-docking design system shell)
static SidebarMenu s_sidebarMenu;
static std::vector<std::unique_ptr<Indicator>> s_indicators;

// Charts
static RealtimeChart s_liveChart1;
static RealtimeChart s_liveChart2;
static RealtimeChart s_liveChart3;
static RealtimeChart s_liveChart4;
static int s_realtimeLayout = 0; // 0: 1 graph, 1: 2 graphs, 2: 3 graphs, 3: 4 graphs

static AnalysisChart s_offlineChart;
static bool s_offlineChartDirty = true;

// Timing & Streaming
static auto s_liveStartTime = std::chrono::steady_clock::now();
static auto s_lastLiveSampleTime = std::chrono::steady_clock::now();
static auto s_lastJournalTime = std::chrono::steady_clock::now();

static bool s_deviceConnected = true;
static bool s_boostHeld = false; // true while the "Hold to Boost" button is pressed
static bool s_deviceStreaming = true;
static DeviceState s_deviceState = DeviceState::Running;
static float s_streamRateJog = 100.0f; // Hz

static std::string s_sessionTitle = "Telemetry Ingestion Session - Zone Alpha";
static EditableLabel s_editableSessionTitle;

// Sidebar & Runs list
static std::vector<RecordedRun> s_runs;
static std::deque<JournalEntry> s_journalLogs;
static bool s_autoScrollJournal = true;
static std::string s_searchArchive = "";
static std::string s_searchJournal = "";

// Machine / Node configuration tabs
static int s_machineTab = 0;
static std::string s_bindAddress = "0.0.0.0";
static int s_bindPort = 9050;
static float s_tcpBufferSize = 64.0f; // KB
static bool s_keepaliveEnabled = true;
static bool s_compressionZstd = true;
static int s_workerThreads = 16;
static std::string s_clusterRegion = "us-east-zone-b";
static bool s_numaAffinity = true;
static int s_schedPolicy = 0;
static float s_cacheQuotaMb = 2048.0f;
static float s_evictionThreshold = 85.0f;
static int s_replicationFactor = 3;
static float s_pidKp = 1.25f;
static float s_pidKi = 0.08f;
static float s_pidKd = 0.35f;

// Node Passport data
static std::string s_passportNodeId = "NODE-AVX512-OCTA-07";
static std::string s_passportFwRev = "v2.8.4-RELEASE-PROD";
static std::string s_passportSimdArch = "AVX-512 F / CD / BW / DQ / VL";
static float s_passportMaxThroughput = 250.0f;
static float s_passportMinLatency = 0.85f;
static bool s_passportEccMemory = true;
static bool s_passportRdmaEnabled = true;
static bool s_machineAvx512Enabled = true;

// printf-style formatting into std::string (for TableGrid::CellText and similar)
static std::string Fmt(const char* format, ...) {
    char buf[128];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    return std::string(buf);
}

// Formatting helper for clock
static std::string FormatCurrentClock() {
    auto nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &nowTime);
#else
    localtime_r(&nowTime, &localTm);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", localTm.tm_hour, localTm.tm_min, localTm.tm_sec);
    return std::string(buf);
}

// Generates rich offline analytical curve with ramp-up, plateau, resonant peak, and breakdown
static void BuildOfflineAnalysisData(const RecordedRun& run) {
    s_offlineChart.Clear();
    s_offlineChart.SetXAxis("Elapsed Time", "s");
    s_offlineChart.SetYAxis("Throughput", "MB/s");
    s_offlineChart.SetPanEnabled(true);
    s_offlineChart.SetZoomEnabled(true);
    s_offlineChart.SetStickyZero(false);

    ChartSeries series;
    series.id = run.id;
    series.label = run.title;
    series.color = 0; // theme accent line
    series.thickness = 2.2f;
    series.style = LineStyle::Solid;
    series.points.reserve(600);

    double peakTarget = run.peakThroughput;
    double duration = run.durationS > 0.0 ? run.durationS : 45.0;

    for (int i = 0; i <= 600; ++i) {
        double t = (i / 600.0) * duration;
        double progress = t / duration;

        double val = 0.0;
        if (progress < 0.15) {
            // Smooth sigmoid ramp up
            double u = progress / 0.15;
            val = peakTarget * 0.75 * (3.0 * u * u - 2.0 * u * u * u);
        } else if (progress < 0.65) {
            // Stable plateau with harmonics
            double u = (progress - 0.15) / 0.50;
            double baseline = peakTarget * 0.85;
            double harmonic = 0.12 * peakTarget * std::sin(u * 12.0 * 3.14159);
            double noise = 0.03 * peakTarget * std::sin(u * 73.0);
            val = baseline + harmonic + noise;
        } else if (progress < 0.85) {
            // Resonant spike
            double u = (progress - 0.65) / 0.20;
            double peakRamp = std::sin(u * 3.14159);
            val = peakTarget * 0.85 + (peakTarget * 0.15) * peakRamp;
        } else {
            // Graceful shutdown decay
            double u = (progress - 0.85) / 0.15;
            val = peakTarget * (1.0 - u * u);
        }

        series.points.push_back({ t, std::max(0.0, val) });
    }
    s_offlineChart.AddSeries(series);

    // Baseline threshold line
    ChartLine thresholdLine;
    thresholdLine.id = "baseline_sla";
    thresholdLine.label = "Target Baseline (65 MB/s)";
    thresholdLine.x1 = 0.0;
    thresholdLine.y1 = 65.0;
    thresholdLine.x2 = duration;
    thresholdLine.y2 = 65.0;
    thresholdLine.style = LineStyle::Dotted;
    thresholdLine.thickness = 1.8f;
    s_offlineChart.AddLine(thresholdLine);

    // Peak limit line
    ChartLine slaLine;
    slaLine.id = "sla_line";
    slaLine.label = "SLA Ceiling (100 MB/s)";
    slaLine.x1 = 0.0;
    slaLine.y1 = 100.0;
    slaLine.x2 = duration;
    slaLine.y2 = 100.0;
    slaLine.style = LineStyle::Dashed;
    slaLine.thickness = 1.5f;
    s_offlineChart.AddLine(slaLine);

    s_offlineChartDirty = false;
}

void Init() {
    if (s_initialized) return;
    s_initialized = true;

    UiTheme& theme = UiTheme::Get();
    theme.LoadFonts();
    theme.SetMode(ThemeMode::Dark);

    // 1. Initial recorded runs for sidebar
    s_runs = {
        { "run_01", "Run #01 - High Rate Ingestion",    "10:14:22", 45.0, 1240, UiVariant::Success, "COMPLETED", theme.palette.success.solid, 118.4, 12.4, 28.5, 96.8, "Nominal" },
        { "run_02", "Run #02 - Vector AVX-512 Matrix",  "10:18:05", 35.0,  850, UiVariant::Success, "COMPLETED", theme.palette.success.solid, 142.0,  8.2, 14.0, 98.4, "Nominal" },
        { "run_03", "Run #03 - Stress Spike Latency",   "10:22:40", 50.0, 3400, UiVariant::Warning, "LATENCY SPIKE", theme.palette.warning.solid, 94.2, 38.6, 72.0, 84.1, "Jitter Anomaly" },
        { "run_04", "Run #04 - Neural Cache Quantize",  "10:28:11",  0.0,    0, UiVariant::Primary, "PROCESSING",    theme.palette.accent,        108.5, 14.8, 26.0, 94.5, "Nominal" },
        { "run_05", "Run #05 - Distributed Lock Sync",  "10:32:50", 40.0,  400, UiVariant::Success, "COMPLETED", theme.palette.success.solid,  64.0,  4.2,  8.1, 99.1, "Nominal" },
        { "run_06", "Run #06 - Socket Buffer Saturation","10:38:19",22.5, 6200, UiVariant::Danger,  "FAILED",    theme.palette.danger.solid,  156.0, 95.0, 180.0, 62.0, "Buffer Overflow" },
        { "run_07", "Run #07 - Standby Ingestion",      "10:44:00", 60.0,  600, UiVariant::Secondary, "IDLE",    theme.palette.textMuted,      45.0, 10.1, 18.0, 97.2, "Nominal" },
    };

    // 2. Setup Real-time Charts
    s_liveChart1.Clear();
    s_liveChart1.SetXAxis("Elapsed Time", "s");
    s_liveChart1.SetYAxis("Throughput", "MB/s");
    s_liveChart1.SetLineThickness(2.2f);
    s_liveChart1.SetHeadMarker(true);

    s_liveChart2.Clear();
    s_liveChart2.SetXAxis("Elapsed Time", "s");
    s_liveChart2.SetYAxis("Latency", "ms");
    s_liveChart2.SetLineThickness(2.2f);
    s_liveChart2.SetHeadMarker(true);

    s_liveChart3.Clear();
    s_liveChart3.SetXAxis("Elapsed Time", "s");
    s_liveChart3.SetYAxis("Memory", "MB");
    s_liveChart3.SetLineThickness(2.2f);
    s_liveChart3.SetHeadMarker(true);

    s_liveChart4.Clear();
    s_liveChart4.SetXAxis("Elapsed Time", "s");
    s_liveChart4.SetYAxis("Packets", "k/s");
    s_liveChart4.SetLineThickness(2.2f);
    s_liveChart4.SetHeadMarker(true);

    s_liveStartTime = std::chrono::steady_clock::now();
    s_lastLiveSampleTime = s_liveStartTime;
    s_lastJournalTime = s_liveStartTime;

    // Pre-populate online chart with 60 live points across [0, 5s]
    for (int i = 0; i <= 60; ++i) {
        double t = i * 0.08;
        double throughput = std::max(8.0, 68.0 + 22.0 * std::sin(t * 1.8) + 12.0 * std::cos(t * 3.4));
        double latency = std::max(2.0, 12.0 + 4.5 * std::cos(t * 1.5));
        double memory = 512.0 + 40.0 * std::sin(t * 0.8);
        double packets = throughput * 1.25;

        s_liveChart1.AppendPoint(t, throughput);
        s_liveChart2.AppendPoint(t, latency);
        s_liveChart3.AppendPoint(t, memory);
        s_liveChart4.AppendPoint(t, packets);
    }

    // 3. Setup Offline Analysis Chart
    BuildOfflineAnalysisData(s_runs[0]);

    // 4. Initial Journal Logs
    s_journalLogs = {
        { "10:14:00", "INFO",    "Application runtime initialized with ThorVG vector presentation backend", UiVariant::Success },
        { "10:14:02", "INFO",    "Cluster socket established to 10.0.4.12:9050 (TCP/IP)", UiVariant::Success },
        { "10:14:22", "SUCCESS", "Run #01 telemetry capture completed: 1,240 records committed", UiVariant::Success },
        { "10:18:05", "SUCCESS", "Run #02 SIMD AVX-512 matrix evaluation completed without errors", UiVariant::Success },
        { "10:22:40", "WARN",    "Run #03 observed packet queue spike at 13.6s (P99: 72 ms)", UiVariant::Warning },
        { "10:28:11", "INFO",    "Run #04 neural cache quantization streaming active", UiVariant::Primary },
    };

    // 5. Setup Persistent Indicators for Header Carousel
    s_indicators.clear();
    s_indicators.push_back(std::make_unique<Indicator>("THROUGHPUT", 0.0, 1, "MB/s", 5, true));
    s_indicators.push_back(std::make_unique<Indicator>("LATENCY", 0.0, 2, "ms", 5, true));
    s_indicators.push_back(std::make_unique<Indicator>("CPU LOAD", 0.0, 1, "%", 4, false));
    s_indicators.push_back(std::make_unique<Indicator>("UPTIME", 0.0, 1, "s", 4, false));
    s_indicators.push_back(std::make_unique<Indicator>("THREADS", 16.0, 0, "", 3, false));
    s_indicators.push_back(std::make_unique<Indicator>("MEMORY", 512.0, 0, "MB", 4, false));

    // Zero / Tare action resets live stream baseline
    s_indicators[0]->SetOnTare([]() {
        s_liveChart1.Clear();
        s_liveChart2.Clear();
        s_liveChart3.Clear();
        s_liveChart4.Clear();
        s_liveStartTime = std::chrono::steady_clock::now();
    });
}

// ----------------------------------------------------------------------------
// 1. TOP HEADER (MIRRORING evo-machine-cpp MainWindow::RenderHeader)
// ----------------------------------------------------------------------------
static void RenderHeader(double elapsed) {
    if (auto header = Header()) {
        const UiTheme& theme = UiTheme::Get();

        // 1.1 Left Zone: Font scale Aa, Sun/Moon theme toggle, and DeviceStatus
        if (auto left = header.Left()) {

            // Aa Scale button
            if (ToolButton({.icon = Icon::Aa, .size = UiSize::Medium, .tooltip = "Interface scale & DPI"})) {
                s_scaleMenuOpen = true;
            }
            if (ContextMenu menu(s_scaleMenuOpen); menu) {
                menu.Header("INTERFACE SCALE");
                menu.Separator();
                float scales[] = { 1.0f, 1.25f, 1.5f, 1.75f, 2.0f };
                for (float s : scales) {
                    char label[32];
                    std::snprintf(label, sizeof(label), "%.0f%%", s * 100.0f);
                    bool isCurrent = std::abs(theme.GetScale() - s) < 0.05f;
                    if (menu.Item(label, {.selected = isCurrent})) {
                        UiTheme::Get().SetScale(s);
                    }
                }
            }

            ImGui::SameLine(0.0f, theme.Scale(6.0f));

            // Theme toggle (Sun / Moon)
            const bool isDark = (theme.mode == ThemeMode::Dark);
            const Icon themeIcon = isDark ? Icon::Moon : Icon::Sun;
            const char* themeTooltip = isDark ? "Dark theme - click for light" : "Light theme - click for dark";
            if (ToolButton({.icon = themeIcon, .size = UiSize::Medium, .tooltip = themeTooltip, .key = "theme"})) {
                UiTheme::Get().SetMode(isDark ? ThemeMode::Light : ThemeMode::Dark);
            }

            ImGui::SameLine(0.0f, theme.Scale(10.0f));

            // Align cursor for DeviceStatus with header content baseline
            ImGui::SetCursorPosY(theme.Scale(theme.header.paddingY));

            const bool running = s_deviceConnected && s_deviceState == DeviceState::Running;
            if (DeviceStatus("Cluster Uplink 07",
                             s_deviceConnected ? s_deviceState : DeviceState::Disconnected,
                             {.status = s_deviceConnected ? (running ? "STREAMING (100 Hz)" : "STANDBY") : "DISCONNECTED",
                              .action = s_deviceConnected ? (running ? "Pause" : "Start") : "Connect"})) {
                if (!s_deviceConnected) {
                    s_deviceConnected = true;
                    s_deviceState = DeviceState::Running;
                    s_deviceStreaming = true;
                } else if (running) {
                    s_deviceState = DeviceState::Idle;
                    s_deviceStreaming = false;
                } else {
                    s_deviceState = DeviceState::Running;
                    s_deviceStreaming = true;
                }
            }
        }

        // 1.2 Right Zone: Indicator count selector (evaluated before Center so Center knows exact width)
        if (auto right = header.Right()) {
            char countTip[64];
            std::snprintf(countTip, sizeof(countTip), "Active Indicators (%d)", s_indicatorsCount);
            if (ToolButton({.icon = Icon::List, .size = UiSize::Medium, .tooltip = countTip})) {
                s_countMenuOpen = true;
            }
            if (ContextMenu menu(s_countMenuOpen); menu) {
                menu.Header("DIGITAL INDICATORS");
                menu.Separator();
                for (int n = 2; n <= 6; ++n) {
                    char label[32];
                    std::snprintf(label, sizeof(label), "%d indicators", n);
                    if (menu.Item(label, {.selected = s_indicatorsCount == n})) {
                        s_indicatorsCount = n;
                    }
                }
            }
        }

        // 1.3 Center Zone: Telemetry carousel
        if (auto center = header.Center()) {
            // Update live values in persistent indicators
            double liveTput = s_deviceStreaming ? (84.6 + 4.8 * std::sin(elapsed * 2.2)) : 0.0;
            double liveLat = s_deviceStreaming ? (14.2 + 2.1 * std::cos(elapsed * 1.5)) : 0.0;
            double liveCpu = s_deviceStreaming ? (24.8 + 6.2 * std::sin(elapsed * 1.1)) : 2.1;
            double liveMem = 512.0 + 35.0 * std::sin(elapsed * 0.7);

            if (s_indicators.size() >= 6) {
                s_indicators[0]->SetValue(liveTput);
                s_indicators[1]->SetValue(liveLat);
                s_indicators[2]->SetValue(liveCpu);
                s_indicators[3]->SetValue(elapsed);
                s_indicators[4]->SetValue(static_cast<double>(s_workerThreads));
                s_indicators[5]->SetValue(liveMem);
            }

            if (Carousel carousel("##HeaderTelemetryCarousel", ImVec2(center.Width(), center.Height())); carousel) {
                for (size_t i = 0; i < s_indicators.size() && i < static_cast<size_t>(s_indicatorsCount); ++i) {
                    if (i > 0) ImGui::SameLine();
                    s_indicators[i]->Render();
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// 2. SESSION / PROJECT TOOLBAR (MIRRORING evo-machine-cpp ProjectBar)
// ----------------------------------------------------------------------------
static void RenderToolbar() {
    if (auto toolbar = Toolbar()) {
        const UiTheme& theme = UiTheme::Get();

        // 2.1 Left Zone: "Session:" label
        if (auto left = toolbar.Left()) {
            left.Label("Session:");
        }

        // 2.2 Center / Fill Zone: Editable title
        if (auto fill = toolbar.Fill()) {
            s_editableSessionTitle.Render("##SessionTitleLabel", s_sessionTitle, fill.Width(), theme.fontBold);
        }

        // 2.3 Right Zone: "New Session" button
        if (auto right = toolbar.Right()) {
            if (right.Button({.label = "New Session", .variant = UiVariant::Primary, .icon = Icon::Plus})) {
                s_sessionTitle = "Telemetry Ingestion Session - " + FormatCurrentClock();
                s_liveChart1.Clear();
                s_liveStartTime = std::chrono::steady_clock::now();
            }
        }
    }
}

// ----------------------------------------------------------------------------
// 3. SIDEBAR NAVIGATION (MIRRORING evo-machine-cpp MainWindow::RenderSidebar)
// ----------------------------------------------------------------------------
static void RenderSidebar() {
    if (auto sidebar = Sidebar()) {
        // Fixed navigation items
        s_sidebarMenu.Item("Live Stream",      Icon::Gamepad,   NavScreen::RemoteControl, s_currentScreen);
        s_sidebarMenu.Item("Offline Analysis", Icon::LineChart, NavScreen::Processing,    s_currentScreen);
        s_sidebarMenu.Item("Machine & Node",   Icon::Cog,       NavScreen::Machine,       s_currentScreen);
        s_sidebarMenu.Item("Data Archive",     Icon::Database,  NavScreen::Archive,       s_currentScreen);
        s_sidebarMenu.Item("Event Journal",    Icon::List,      NavScreen::Journal,       s_currentScreen);
        s_sidebarMenu.Item("Node Passport",    Icon::Clipboard, NavScreen::Passport,      s_currentScreen);
        s_sidebarMenu.Item("Batch Analytics",  Icon::BarChart,  NavScreen::Series,        s_currentScreen);

        s_sidebarMenu.Spacing(8.0f);
        s_sidebarMenu.Separator();
        s_sidebarMenu.SectionTitle("RECORDED RUNS");

        // Scrollable region for recorded runs
        if (s_sidebarMenu.BeginScrollRegion("##RunsScrollRegion")) {
            for (size_t i = 0; i < s_runs.size(); ++i) {
                const auto& run = s_runs[i];
                bool isSelected = (s_currentScreen == NavScreen::Processing && s_selectedRunIndex == static_cast<int>(i));

                char sublabel[64];
                std::snprintf(sublabel, sizeof(sublabel), "%s • %d pts", run.timestamp.c_str(), run.pointCount);

                if (s_sidebarMenu.ItemEx(run.id.c_str(), run.title.c_str(), sublabel, run.ledColor, isSelected)) {
                    s_selectedRunIndex = static_cast<int>(i);
                    s_currentScreen = NavScreen::Processing;
                    BuildOfflineAnalysisData(s_runs[i]);
                }

                // Tooltip
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", run.title.c_str());
                    ImGui::TextDisabled("Started: %s | Duration: %.1f s", run.timestamp.c_str(), run.durationS);
                    ImGui::TextDisabled("Peak: %.1f MB/s | Latency: %.1f ms", run.peakThroughput, run.avgLatency);
                    Badge(run.statusText, {.variant = run.status});
                    ImGui::EndTooltip();
                }

                // Context menu
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Open in Analysis")) {
                        s_selectedRunIndex = static_cast<int>(i);
                        s_currentScreen = NavScreen::Processing;
                        BuildOfflineAnalysisData(s_runs[i]);
                    }
                    if (ImGui::MenuItem("Delete Run", nullptr, false, run.status != UiVariant::Primary)) {
                        s_runs.erase(s_runs.begin() + i);
                        if (s_selectedRunIndex >= static_cast<int>(s_runs.size())) {
                            s_selectedRunIndex = static_cast<int>(s_runs.size()) - 1;
                        }
                        if (s_selectedRunIndex >= 0) {
                            BuildOfflineAnalysisData(s_runs[s_selectedRunIndex]);
                        }
                        ImGui::EndPopup();
                        break;
                    }
                    ImGui::EndPopup();
                }
            }
            s_sidebarMenu.EndScrollRegion();
        }
    }
}

// ----------------------------------------------------------------------------
// 4. SCREEN 1: REMOTE CONTROL & REAL-TIME STREAM
// ----------------------------------------------------------------------------
static void RenderRemoteControl() {
    const UiTheme& theme = UiTheme::Get();
    float availW = ImGui::GetContentRegionAvail().x;
    float availH = ImGui::GetContentRegionAvail().y;

    float sideOccupiedW = SidePanel::CalcTotalWidth(260.0f, false);
    float graphAreaW = std::max(50.0f, availW - sideOccupiedW - theme.SpacingMedium());

    // 1. Left Area: Animated Real-time Chart
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(theme.card.colBg).Value);
    if (ImGui::BeginChild("##LiveGraphArea", ImVec2(graphAreaW, availH), true)) {
        // Single combo to select number of graphs to display: 1, 2, 3, 4
        const char* const graphCountOptions[] = {
            "1 Graph",
            "2 Graphs",
            "3 Graphs",
            "4 Graphs"
        };
        Combo(s_realtimeLayout, graphCountOptions, 4, {.width = 160.0f, .key = "graph_count"});

        ImGui::Spacing();

        ImVec2 graphSize = ImGui::GetContentRegionAvail();
        float spacing = theme.SpacingMedium();

        if (s_realtimeLayout == 0) {
            // 1 Graph: Single chart filling the entire available graph space (Throughput)
            s_liveChart1.Render("##LiveChart1", graphSize);
        } else if (s_realtimeLayout == 1) {
            // 2 Graphs: Dual stacked charts (Top: Throughput, Bottom: Latency)
            float halfH = std::max(50.0f, (graphSize.y - spacing) * 0.5f);
            s_liveChart1.Render("##LiveChart1", ImVec2(graphSize.x, halfH));
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
            s_liveChart2.Render("##LiveChart2", ImVec2(graphSize.x, halfH));
        } else if (s_realtimeLayout == 2) {
            // 3 Graphs: Top full width (Throughput), Bottom split dual (Left: Latency, Right: Memory)
            float halfW = std::max(50.0f, (graphSize.x - spacing) * 0.5f);
            float halfH = std::max(50.0f, (graphSize.y - spacing) * 0.5f);

            s_liveChart1.Render("##LiveChart1", ImVec2(graphSize.x, halfH));
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
            s_liveChart2.Render("##LiveChart2", ImVec2(halfW, halfH));
            ImGui::SameLine(0.0f, spacing);
            s_liveChart3.Render("##LiveChart3", ImVec2(halfW, halfH));
        } else {
            // 4 Graphs: Quad 2x2 grid
            // Top: Throughput (left) & Latency (right)
            // Bottom: Memory (left) & Packets (right)
            float halfW = std::max(50.0f, (graphSize.x - spacing) * 0.5f);
            float halfH = std::max(50.0f, (graphSize.y - spacing) * 0.5f);

            s_liveChart1.Render("##LiveChart1", ImVec2(halfW, halfH));
            ImGui::SameLine(0.0f, spacing);
            s_liveChart2.Render("##LiveChart2", ImVec2(halfW, halfH));

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
            s_liveChart3.Render("##LiveChart3", ImVec2(halfW, halfH));
            ImGui::SameLine(0.0f, spacing);
            s_liveChart4.Render("##LiveChart4", ImVec2(halfW, halfH));
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // 2. Right Area: SidePanel Remote Control actions
    if (SidePanel panel("##RemoteSidePanel", 260.0f, SidePanel::Side::Right); panel) {
        if (Card actionCard("Stream Actions"); actionCard) {
            if (Button({.label = "START STREAM", .variant = UiVariant::Success, .icon = Icon::Play, .size = UiSize::Large, .width = ButtonOptions::Fill})) {
                s_deviceState = DeviceState::Running;
                s_deviceStreaming = true;
            }
            ImGui::Spacing();
            if (Button({.label = "PAUSE FEED", .variant = UiVariant::Warning, .icon = Icon::Pause, .width = ButtonOptions::Fill})) {
                s_deviceState = DeviceState::Idle;
                s_deviceStreaming = false;
            }
            ImGui::Spacing();
            if (Button({.label = "STOP & RESET", .variant = UiVariant::Danger, .icon = Icon::Square, .width = ButtonOptions::Fill})) {
                s_deviceState = DeviceState::Idle;
                s_deviceStreaming = false;
                s_resetConfirm.Open();
            }
        }

        // Destructive action goes through a confirmation modal
        if (s_resetConfirm.Render()) {
            s_liveChart1.Clear();
            s_liveChart2.Clear();
            s_liveChart3.Clear();
            s_liveChart4.Clear();
            s_liveStartTime = std::chrono::steady_clock::now();
        }

        ImGui::Spacing();

        if (Card jogCard("Sampling Frequency"); jogCard) {
            static const std::vector<float> freqs = { 20.0f, 50.0f, 100.0f, 250.0f, 500.0f };
            PresetGrid(s_streamRateJog, freqs, {.unit = "Hz", .columns = 3});
        }

        ImGui::Spacing();

        if (Card calibCard("Baseline & Zero"); calibCard) {
            if (Button({.label = "Tare / Zero Baseline", .variant = UiVariant::Secondary, .icon = Icon::Zero, .width = ButtonOptions::Fill})) {
                s_liveChart1.Clear();
                s_liveChart2.Clear();
                s_liveChart3.Clear();
                s_liveChart4.Clear();
                s_liveStartTime = std::chrono::steady_clock::now();
            }

            // Hold button: true only while pressed (Card::HoldButton); streams ~4x faster while held
            ImGui::Spacing();
            s_boostHeld = calibCard.HoldButton({.label = "Hold to Boost Stream", .variant = UiVariant::Info, .icon = Icon::Play,
                                               .col = Col::Full(), .disabled = !s_deviceStreaming});
            Tooltip::OnLastItem(s_deviceStreaming ? "Samples ~4x faster while the button is held"
                                                  : "Start the stream to enable boost");
        }
    }
}

// ----------------------------------------------------------------------------
// 5. SCREEN 2: OFFLINE DATASET ANALYSIS (AnalysisChart with Zoom & Pan)
// ----------------------------------------------------------------------------
static void RenderProcessing() {
    const UiTheme& theme = UiTheme::Get();
    float availW = ImGui::GetContentRegionAvail().x;
    float availH = ImGui::GetContentRegionAvail().y;

    const auto& activeRun = s_runs[s_selectedRunIndex];
    float sideOccupiedW = SidePanel::CalcTotalWidth(280.0f, false);
    float chartAreaW = std::max(50.0f, availW - sideOccupiedW - theme.SpacingMedium());

    // 1. Left Area: Interactive Zoomable AnalysisChart
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(theme.card.colBg).Value);
    if (ImGui::BeginChild("##OfflineChartContainer", ImVec2(chartAreaW, availH), true)) {
        ImGui::TextColored(ImColor(theme.palette.accent).Value, "%s", activeRun.title.c_str());
        ImGui::SameLine();
        Badge(activeRun.statusText, {.variant = activeRun.status});

        ImGui::SameLine(chartAreaW - 240.0f * theme.GetScale());
        if (Button({.label = "Reset Zoom", .variant = UiVariant::Secondary, .icon = Icon::Refresh, .size = UiSize::Small})) {
            s_offlineChart.ResetZoom();
        }
        ImGui::SameLine();
        bool majGrid = s_offlineChart.GetOptions().majorGrid;
        if (Button({.label = majGrid ? "Grid: ON" : "Grid: OFF", .variant = UiVariant::Secondary, .size = UiSize::Small})) {
            s_offlineChart.SetMajorGrid(!majGrid);
            s_offlineChart.SetMinorGrid(!majGrid);
        }

        ImGui::Spacing();

        ImVec2 chartSz = ImGui::GetContentRegionAvail();
        s_offlineChart.Render("##OfflineDatasetAnalysisChart", chartSz);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // 2. Right Area: Metrics Evaluation & Run Metadata
    if (SidePanel sideP("##AnalysisSideMetrics", 280.0f, SidePanel::Side::Right); sideP) {
        if (Card metricsCard("Calculated Indicators"); metricsCard) {
            std::vector<TableGrid::Column> cols = {
                { "Indicator", ColumnWidthMode::Stretch, 1.0f },
                { "Value / Status", ColumnWidthMode::Fixed, 120.0f }
            };
            TableGrid::Options opts;
            opts.scrollY = false;
            opts.height = 170.0f;
            if (TableGrid grid("MetricsTable", cols, opts); grid) {
                grid.NextRow();
                grid.SetColumn(0); ImGui::Text("Peak Throughput");
                grid.SetColumn(1); ImGui::Text("%.1f MB/s", activeRun.peakThroughput);
                ImGui::SameLine(); TableCell::MetricStatus("ok");

                grid.NextRow();
                grid.SetColumn(0); ImGui::Text("Mean Latency");
                grid.SetColumn(1); ImGui::Text("%.1f ms", activeRun.avgLatency);
                ImGui::SameLine(); TableCell::MetricStatus("ok");

                grid.NextRow();
                grid.SetColumn(0); ImGui::Text("P99 Latency");
                grid.SetColumn(1); ImGui::Text("%.1f ms", activeRun.p99Latency);
                ImGui::SameLine(); TableCell::MetricStatus(activeRun.p99Latency > 30.0 ? "manual_needed" : "ok");

                grid.NextRow();
                grid.SetColumn(0); ImGui::Text("Efficiency Factor");
                grid.SetColumn(1); ImGui::Text("%.1f %%", activeRun.efficiency);
                ImGui::SameLine(); TableCell::MetricStatus("ok");

                grid.NextRow();
                grid.SetColumn(0); ImGui::Text("Anomaly Classifier");
                grid.SetColumn(1); ImGui::Text("%s", activeRun.anomaly.c_str());
                ImGui::SameLine(); TableCell::MetricStatus(activeRun.anomaly == "Nominal" ? "ok" : "empty");
            }
        }

        ImGui::Spacing();

        if (Card metaCard("Run Metadata"); metaCard) {
            ValueDisplay(static_cast<double>(activeRun.pointCount), {.unit = "points", .format = "%.0f", .compact = true});
            ImGui::Spacing();
            ValueDisplay(activeRun.durationS, {.unit = "s", .format = "%.1f", .compact = true});
        }
    }
}

// ----------------------------------------------------------------------------
// 6. SCREEN 3: SYSTEM & HARDWARE CONFIGURATION
// ----------------------------------------------------------------------------
static void RenderMachine() {
    const UiTheme& theme = UiTheme::Get();
    const char* machineTabs[] = {
        "Networking",
        "Compute Arch",
        "AVX-512 SIMD",
        "Buffer Pool",
        "NVMe Storage",
        "Rate Limiter & PID",
        "Diagnostics",
        "Export / Sync"
    };
    TabBar(machineTabs, s_machineTab);

    ImGui::Spacing();

    ImGui::BeginChild("##MachineTabScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_None);
    {
        switch (s_machineTab) {
            case 0: { // Networking
                if (Card netCard("Socket & Transport Configuration"); netCard) {
                    // Card grid: fields flow across a 12-column row
                    netCard.Text(s_bindAddress, {.label = "Listening Interface Address", .col = Col::TwoThirds()});
                    netCard.Int(s_bindPort, {.label = "Uplink TCP Port", .col = Col::Third()});
                    netCard.Float(s_tcpBufferSize, {.label = "Socket Ring Buffer Size", .unit = "KB", .format = "%.0f", .col = Col::Third()});
                    netCard.Toggle(s_keepaliveEnabled, {.label = "Enable TCP Keepalive Probes", .sublabel = "Verifies socket liveness every 15s"});
                    netCard.Toggle(s_compressionZstd, {.label = "Payload Compression (Zstandard)", .sublabel = "Compresses wire packets above 4 KB"});
                    netCard.Button({.label = "Apply Network Configuration", .variant = UiVariant::Primary, .icon = Icon::Check, .col = Col::Auto()});
                }
                break;
            }
            case 1: { // Compute Architecture
                if (Card compCard("Thread Concurrency & Core Allocation"); compCard) {
                    static const char* const schedPolicies[] = { "Round Robin", "Work Stealing", "Priority Queue" };
                    compCard.Int(s_workerThreads, {.label = "Active Worker Thread Count"});
                    compCard.Combo(s_schedPolicy, schedPolicies, {.label = "Scheduling Policy"});
                    compCard.Text(s_clusterRegion, {.label = "Cluster Deployment Zone", .col = Col::Full()});
                    compCard.Toggle(s_numaAffinity, {.label = "NUMA Socket Memory Pinning", .sublabel = "Allocates buffers strictly local to core", .col = Col::Full()});
                    compCard.Button({.label = "Apply Compute Settings", .variant = UiVariant::Primary, .icon = Icon::Check, .col = Col::Auto()});
                }
                break;
            }
            case 2: { // AVX-512 SIMD
                if (Card simdCard("Vectorization Engine Parameters"); simdCard) {
                    Toggle(s_machineAvx512Enabled, {.label = "Enable AVX-512 FPU Instructions", .sublabel = "512-bit wide vector matrix operations"});
                    ImGui::Spacing();
                    InputField(s_passportMaxThroughput, {.label = "Rated Maximum Bandwidth", .unit = "MB/s"});
                    ImGui::Spacing();
                    Button({.label = "Commit SIMD Pipeline", .variant = UiVariant::Primary, .icon = Icon::Check, .width = 240.0f});
                }
                break;
            }
            case 3: { // Buffer Pool
                if (Card bufCard("Slab Allocator Quotas"); bufCard) {
                    bufCard.Float(s_cacheQuotaMb, {.label = "Max L1 Memory Slab Size", .unit = "MB", .format = "%.0f"});
                    bufCard.Float(s_evictionThreshold, {.label = "Eviction High-Watermark", .unit = "%", .format = "%.0f"});
                    bufCard.Value(s_cacheQuotaMb * s_evictionThreshold / 100.0f, {.label = "Effective Watermark", .unit = "MB", .format = "%.0f", .size = UiSize::Large});
                    bufCard.Button({.label = "Flush & Reallocate Slabs", .variant = UiVariant::Warning, .icon = Icon::Refresh, .col = Col::Auto()});
                }
                break;
            }
            case 4: { // Storage NVMe
                if (Card storageCard("Zero-Copy Disk Writer"); storageCard) {
                    InputField(s_replicationFactor, {.label = "Replication Factor"});
                    ImGui::Spacing();
                    Button({.label = "Sync Storage Buffers", .variant = UiVariant::Primary, .icon = Icon::Check, .width = 240.0f});
                }
                break;
            }
            case 5: { // Rate Limiter & PID
                if (Card pidCard("Telemetry Stream PID Controller"); pidCard) {
                    if (auto row = pidCard.Row()) {
                        pidCard.Float(s_pidKp, {.label = "Proportional Gain (Kp)", .col = Col::Third()});
                        pidCard.Float(s_pidKi, {.label = "Integral Gain (Ki)", .col = Col::Third()});
                        pidCard.Float(s_pidKd, {.label = "Derivative Gain (Kd)", .col = Col::Third()});
                    }
                    pidCard.Button({.label = "Apply PID Calibration", .variant = UiVariant::Primary, .icon = Icon::Check, .col = Col::Auto()});
                }
                break;
            }
            default: { // Diagnostics & Export
                if (Card diagCard("Self-Test & Diagnostics"); diagCard) {
                    Button({.label = "Run Cluster Diagnostic Self-Test", .variant = UiVariant::Info, .icon = Icon::Target, .width = 260.0f});
                }
                break;
            }
        }
    }
    ImGui::EndChild();
}

// ----------------------------------------------------------------------------
// 7. SCREEN 4: DATA ARCHIVE (TableGrid & SearchInput)
// ----------------------------------------------------------------------------
static void RenderArchive() {
    const UiTheme& theme = UiTheme::Get();
    if (Card archiveCard("Telemetry Dataset Archive"); archiveCard) {
        SearchInput(s_searchArchive, {.hint = "Search datasets by name or run ID...", .key = "archive"});
        ImGui::Spacing();

        std::vector<TableGrid::Column> cols = {
            { "ID", ColumnWidthMode::Fixed, 65.0f },
            { "Dataset Name", ColumnWidthMode::Stretch, 1.0f },
            { "Timestamp", ColumnWidthMode::Fixed, 90.0f },
            { "Duration", ColumnWidthMode::Fixed, 80.0f },
            { "Samples", ColumnWidthMode::Fixed, 80.0f },
            { "Status", ColumnWidthMode::Fixed, 110.0f },
            { "Action", ColumnWidthMode::Fixed, 80.0f }
        };

        if (TableGrid grid("FullArchiveTable", cols); grid) {
            for (size_t i = 0; i < s_runs.size(); ++i) {
                const auto& run = s_runs[i];
                if (!s_searchArchive.empty() &&
                    run.title.find(s_searchArchive) == std::string::npos &&
                    run.id.find(s_searchArchive) == std::string::npos) {
                    continue;
                }
                grid.NextRow(static_cast<int>(i));
                grid.SetColumn(0); grid.CellText(run.id);
                grid.SetColumn(1); grid.CellText(run.title);
                grid.SetColumn(2); grid.CellText(run.timestamp, theme.palette.textMuted);
                grid.SetColumn(3); grid.CellText(Fmt("%.1f s", run.durationS));
                grid.SetColumn(4); grid.CellText(Fmt("%d", run.pointCount));
                grid.SetColumn(5); Badge(run.statusText, {.variant = run.status});
                grid.SetColumn(6);
                if (Button({.label = "View", .variant = UiVariant::Secondary, .size = UiSize::Mini, .width = 70.0f,
                            .tooltip = "Open this run in Offline Analysis"})) {
                    s_selectedRunIndex = static_cast<int>(i);
                    s_currentScreen = NavScreen::Processing;
                    BuildOfflineAnalysisData(s_runs[i]);
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// 8. SCREEN 5: EVENT JOURNAL (Terminal Log Stream)
// ----------------------------------------------------------------------------
static void RenderJournal() {
    const UiTheme& theme = UiTheme::Get();
    if (Card journalCard("Real-Time Event & Operator Journal"); journalCard) {
        ImGui::Checkbox("Auto-scroll", &s_autoScrollJournal);
        ImGui::SameLine();
        SearchInput(s_searchJournal, {.hint = "Filter log entries...", .width = 220, .key = "journal"});
        ImGui::SameLine();
        if (Button({.label = "Clear Journal", .variant = UiVariant::Secondary, .icon = Icon::Refresh, .size = UiSize::Small})) {
            s_journalLogs.clear();
        }

        ImGui::Spacing();

        ImGui::BeginChild("JournalTerminalShell", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        {
            for (const auto& entry : s_journalLogs) {
                if (!s_searchJournal.empty() && entry.message.find(s_searchJournal) == std::string::npos) {
                    continue;
                }
                ImGui::TextColored(ImColor(theme.palette.textMuted).Value, "[%s]", entry.timeStr.c_str());
                ImGui::SameLine();
                Badge(entry.level.c_str(), {.variant = entry.variant});
                ImGui::SameLine();
                ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", entry.message.c_str());
            }
            if (s_autoScrollJournal && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();
    }
}

// ----------------------------------------------------------------------------
// 9. SCREEN 6: NODE PASSPORT (Hardware Specs)
// ----------------------------------------------------------------------------
static void RenderPassport() {
    if (Card passportCard("Node Hardware Specification & Identity"); passportCard) {
        InputField(s_passportNodeId, {.label = "Node Unique Identifier"});
        ImGui::Spacing();
        InputField(s_passportFwRev, {.label = "Firmware / Runtime Revision"});
        ImGui::Spacing();
        InputField(s_passportSimdArch, {.label = "SIMD Architecture Extensions"});
        ImGui::Spacing();
        InputField(s_passportMaxThroughput, {.label = "Rated Maximum Bandwidth", .unit = "MB/s"});
        ImGui::Spacing();
        InputField(s_passportMinLatency, {.label = "Design Latency Lower Bound", .unit = "ms"});
        ImGui::Spacing();
        Toggle(s_passportEccMemory, {.label = "ECC Memory Scrubbing Active", .sublabel = "Hardware parity correction enabled"});
        Toggle(s_passportRdmaEnabled, {.label = "Direct Memory Access (RDMA)", .sublabel = "Kernel bypass zero-copy network buffers"});
    }
}

// ----------------------------------------------------------------------------
// 10. SCREEN 7: BATCH ANALYTICS (Series Evaluation Matrix)
// ----------------------------------------------------------------------------
static void RenderSeries() {
    const UiTheme& theme = UiTheme::Get();
    if (Card seriesCard("Cluster Nodes Comparative Analytics"); seriesCard) {
        std::vector<TableGrid::Column> cols = {
            { "Node Identifier", ColumnWidthMode::Stretch, 1.0f },
            { "Peak Tput (MB/s)", ColumnWidthMode::Fixed, 130.0f },
            { "Avg Latency (ms)", ColumnWidthMode::Fixed, 130.0f },
            { "P99 Tail (ms)", ColumnWidthMode::Fixed, 110.0f },
            { "Efficiency", ColumnWidthMode::Fixed, 100.0f },
            { "State", ColumnWidthMode::Fixed, 110.0f }
        };

        if (TableGrid grid("SeriesTable", cols); grid) {
            for (size_t i = 0; i < s_runs.size(); ++i) {
                const auto& run = s_runs[i];
                grid.NextRow(static_cast<int>(i));
                grid.SetColumn(0); ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", run.title.c_str());
                grid.SetColumn(1); ImGui::Text("%.1f", run.peakThroughput);
                grid.SetColumn(2); ImGui::Text("%.2f", run.avgLatency);
                grid.SetColumn(3); ImGui::Text("%.2f", run.p99Latency);
                grid.SetColumn(4); ImGui::Text("%.1f %%", run.efficiency);
                grid.SetColumn(5); Badge(run.statusText, {.variant = run.status});
            }
        }
    }
}

// ----------------------------------------------------------------------------
// 11. MAIN CONTENT ROUTER (ContentArea)
// ----------------------------------------------------------------------------
static void RenderContent() {
    if (auto content = ContentArea()) {
        switch (s_currentScreen) {
            case NavScreen::RemoteControl:
                RenderRemoteControl();
                break;
            case NavScreen::Processing:
                RenderProcessing();
                break;
            case NavScreen::Machine:
                RenderMachine();
                break;
            case NavScreen::Archive:
                RenderArchive();
                break;
            case NavScreen::Journal:
                RenderJournal();
                break;
            case NavScreen::Passport:
                RenderPassport();
                break;
            case NavScreen::Series:
                RenderSeries();
                break;
        }
    }
}

// ----------------------------------------------------------------------------
// 12. BOTTOM STATUS BAR (StatusBar)
// ----------------------------------------------------------------------------
static void RenderStatusBar() {
    if (auto status = StatusBar()) {
        status.Text("Uplink: 10.0.4.12:9050 (Active • 1.2 ms)", UiVariant::Success);
        status.Separator();

        std::string curMsg = (s_deviceState == DeviceState::Running)
                           ? "System nominal • Real-time pipeline active and streaming"
                           : "Standby • Stream paused by operator";
        status.Text(curMsg, s_deviceState == DeviceState::Running ? UiVariant::Default : UiVariant::Warning);

        status.Separator();
        status.Text("OmniGUI v1.0.7 | " + FormatCurrentClock(), UiVariant::Secondary);
    }
}

// ----------------------------------------------------------------------------
// MAIN SHOWCASE ENTRY POINT
// ----------------------------------------------------------------------------
void RenderUI(float main_scale) {
    if (!s_initialized) {
        Init();
    }

    UiTheme& theme = UiTheme::Get();
    if (theme.GetScale() > 0.0f) {
        main_scale = theme.GetScale();
    }
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - s_liveStartTime).count();

    // Stream synthetic real-time telemetry curve data (Throughput vs Elapsed Time)
    if (s_deviceStreaming && s_deviceConnected && s_deviceState == DeviceState::Running) {
        double dt = std::chrono::duration<double>(now - s_lastLiveSampleTime).count();
        const double minDt = s_boostHeld ? 0.008 : 0.033; // boost: ~4x telemetry rate
        if (dt >= minDt) { // ~30 Hz telemetry rate (120 Hz while boosted)
            s_lastLiveSampleTime = now;
            double t = elapsed;
            double baseline = 68.0;
            double harmonics = 22.0 * std::sin(t * 1.8) + 12.0 * std::cos(t * 3.4);
            double burst = 14.0 * std::sin(t * 0.4);
            double throughput = std::max(6.0, baseline + harmonics + burst);

            double latency = std::max(1.5, 12.0 + 4.5 * std::cos(t * 1.5) + 2.0 * std::sin(t * 4.2));
            double memory = 512.0 + 50.0 * std::sin(t * 0.8);
            double packets = throughput * 1.25;

            // Continuous sweep: refresh window when exceeding 250 samples
            if (s_liveChart1.PointCount() > 250) {
                s_liveChart1.Clear();
                s_liveChart2.Clear();
                s_liveChart3.Clear();
                s_liveChart4.Clear();
                s_liveStartTime = now;
                elapsed = 0.0;
            }

            s_liveChart1.AppendPoint(elapsed, throughput);
            s_liveChart2.AppendPoint(elapsed, latency);
            s_liveChart3.AppendPoint(elapsed, memory);
            s_liveChart4.AppendPoint(elapsed, packets);
        }
    }

    // Periodic live journal events
    {
        double logDt = std::chrono::duration<double>(now - s_lastJournalTime).count();
        if (logDt >= 4.0) {
            s_lastJournalTime = now;
            std::string curTime = FormatCurrentClock();
            static int s_logIdx = 0;
            s_logIdx++;
            if (s_logIdx % 4 == 0) {
                s_journalLogs.push_back({ curTime, "WARN", "Memory pressure watermark at 78% on L1 cache slab", UiVariant::Warning });
            } else if (s_logIdx % 3 == 0) {
                s_journalLogs.push_back({ curTime, "METRIC", "Heartbeat latency verified: 1.18 ms RTT", UiVariant::Info });
            } else {
                s_journalLogs.push_back({ curTime, "INFO", "Batch compacted 4,096 records in SIMD pipeline", UiVariant::Success });
            }
            if (s_journalLogs.size() > 100) {
                s_journalLogs.pop_front();
            }
        }
    }

    // ========================================================================
    // Five Component Architecture (Fully Tiling Viewport)
    // ========================================================================
    // 1. Top Header Bar
    RenderHeader(elapsed);

    // 2. Session / Project Toolbar
    RenderToolbar();

    // 3. Left Sidebar
    RenderSidebar();

    // 4. Main Content Area
    RenderContent();

    // 5. Bottom Status Bar
    RenderStatusBar();
}

} // namespace OmniKitShowcase
