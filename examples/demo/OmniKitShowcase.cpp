#include "OmniKitShowcase.hpp"
#include "omnikit.hpp"
#include "imgui.h"

#include <cmath>
#include <chrono>
#include <string>
#include <vector>
#include <deque>

namespace OmniKitShowcase {

// Sidebar channels data model
struct ChannelInfo {
    std::string id;
    std::string name;
    std::string sublabel;
    std::string nodeId;
    ImU32 statusColor;
};

// Distributed job queue record
struct JobRecord {
    int id;
    std::string taskName;
    std::string engine;
    double executionTimeMs;
    double memoryUsageMb;
    UiVariant status;
    const char* statusText;
};

// Diagnostic log entry
struct LogEntry {
    std::string timeStr;
    std::string level;
    std::string message;
    UiVariant variant;
};

// Demonstration state
static bool s_initialized = false;
static RealtimeChart s_realtimeChart;
static auto s_startTime = std::chrono::steady_clock::now();
static auto s_lastSampleTime = std::chrono::steady_clock::now();
static auto s_lastLogTime = std::chrono::steady_clock::now();

static DeviceState s_deviceState = DeviceState::Running;
static bool s_autoTare = false;
static bool s_liveFeed = true;
static bool s_hardwareAcc = true;
static float s_sampleInterval = 250.0f; // ms
static std::string s_searchQuery = "";
static std::string s_nodeIdentifier = "worker-node-cluster-07";
static float s_throughputLimit = 120.0f; // MB/s
static bool s_confirmModalOpen = false;

static int s_currentNav = 0;
static int s_selectedChannel = 2; // Default to Worker-07

static std::vector<ChannelInfo> s_channels;
static std::vector<JobRecord> s_jobRecords;
static std::deque<LogEntry> s_diagnosticLogs;
static bool s_autoScrollLogs = true;

// Long settings form state (for Tab 3 scroll form demo)
static float s_tcpBufferSize = 64.0f; // KB
static int s_workerThreads = 16;
static float s_connectionTimeout = 5.0f; // s
static bool s_keepaliveEnabled = true;
static bool s_compressionZstd = true;
static float s_cacheQuotaMb = 2048.0f;
static float s_evictionThreshold = 85.0f; // %
static int s_replicationFactor = 3;
static std::string s_clusterRegion = "us-east-zone-b";

void Init() {
    if (s_initialized) return;
    s_initialized = true;

    UiTheme& theme = UiTheme::Get();
    theme.LoadFonts();
    theme.SetMode(ThemeMode::Dark);

    // Channels for sidebar navigation
    s_channels = {
        { "ch_01", "Node-01 East",     "100 Hz • Streaming",  "node-east-01",      theme.palette.success.solid },
        { "ch_02", "Node-02 Central",  "Batch 512 • Active",  "node-central-02",   theme.palette.primary.solid },
        { "ch_03", "Worker-07",        "Throughput 85 MB/s",  "worker-cluster-07", theme.palette.accent },
        { "ch_04", "Edge Gateway 04",  "Latency Spike • Warn","edge-gw-04",        theme.palette.warning.solid },
        { "ch_05", "Relay Node 05",    "Standby • Ready",     "relay-05",          theme.palette.textMuted },
        { "ch_06", "Storage Sink 09",  "Flushing Buffer",     "sink-nvme-09",      theme.palette.info.solid },
        { "ch_07", "Backup Ingestion", "Idle • Synced",       "backup-ingest-01",  theme.palette.textMuted },
        { "ch_08", "Neural Worker 03", "Quantizing Weights",  "neural-simd-03",    theme.palette.primary.solid },
    };

    // Configure RealtimeChart
    s_realtimeChart.Clear();
    s_realtimeChart.SetXAxis("Elapsed Time", "s");
    s_realtimeChart.SetYAxis("Throughput", "MB/s");
    s_realtimeChart.SetLineThickness(2.2f);
    s_realtimeChart.SetHeadMarker(true);

    s_startTime = std::chrono::steady_clock::now();
    s_lastSampleTime = s_startTime;
    s_lastLogTime = s_startTime;

    // Pre-populate with historical points across [0.0, 5.0]s
    // so the chart immediately displays a full horizontal curve
    for (int i = 0; i <= 65; ++i) {
        double t = i * 0.08;
        double baseline = 65.0;
        double harmonics = 22.0 * std::sin(t * 1.8) + 12.0 * std::cos(t * 3.4);
        double throughput = std::max(12.0, baseline + harmonics);
        s_realtimeChart.AppendPoint(t, throughput);
    }

    s_jobRecords = {
        { 1001, "Telemetry Ingestion Pipeline", "ThorEngine v4", 12.4, 256.0, UiVariant::Success, "COMPLETED" },
        { 1002, "Vector Matrix Transformation", "SIMD-AVX512",   48.2, 512.5, UiVariant::Success, "COMPLETED" },
        { 1003, "Neural Weight Quantization",  "CoreCompute",   118.0, 1024.0, UiVariant::Primary, "PROCESSING" },
        { 1004, "Cache Index Rebalancing",     "LSM-Store",       6.5, 128.2, UiVariant::Success, "COMPLETED" },
        { 1005, "Distributed Lock Heartbeat",  "Raft-Cluster",    2.1,  64.0, UiVariant::Warning, "LATENCY SPIKE" },
        { 1006, "TLS Key Exchange Handshake",  "CryptoLib",       4.8,  32.0, UiVariant::Success, "COMPLETED" },
        { 1007, "Batch Data Compaction",       "ZSTD-Parallel", 230.1, 780.0, UiVariant::Danger,  "FAILED" },
        { 1008, "Snapshot Checkpoint Sink",    "RocksDB-IO",     18.6, 320.0, UiVariant::Success, "COMPLETED" },
        { 1009, "Kafka Consumer Group Sync",   "StreamBridge",    3.4,  48.0, UiVariant::Success, "COMPLETED" },
    };

    s_diagnosticLogs = {
        { "00:01.02", "INFO",    "Cluster heartbeat synchronized across 8 active nodes", UiVariant::Success },
        { "00:01.45", "METRIC",  "Average telemetry ingestion rate stabilized at 84.6 MB/s", UiVariant::Info },
        { "00:02.10", "SUCCESS", "AVX-512 SIMD vector compute pipeline online and verified", UiVariant::Success },
        { "00:02.80", "WARN",    "Worker-07 reported transient latency excursion (+4.2 ms)", UiVariant::Warning },
        { "00:03.22", "INFO",    "Ring buffer flushed 10,240 records without frame drops", UiVariant::Success },
        { "00:04.05", "METRIC",  "Zero-copy socket transfer efficiency: 99.4%", UiVariant::Info },
        { "00:04.91", "INFO",    "TLS 1.3 session ticket refreshed for downstream gateway", UiVariant::Success },
    };
}

void RenderUI(float main_scale) {
    if (!s_initialized) {
        Init();
    }

    UiTheme& theme = UiTheme::Get();
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - s_startTime).count();

    // Stream synthetic real-time telemetry curve data (Throughput vs Elapsed Time)
    if (s_liveFeed && s_deviceState == DeviceState::Running) {
        double dt = std::chrono::duration<double>(now - s_lastSampleTime).count();
        if (dt >= 0.033) { // ~30 Hz telemetry rate
            s_lastSampleTime = now;
            double t = elapsed;
            double baseline = 65.0;
            double harmonics = 22.0 * std::sin(t * 1.8) + 12.0 * std::cos(t * 3.4);
            double burst = 14.0 * std::sin(t * 0.4);
            double throughput = std::max(8.0, baseline + harmonics + burst);

            // Cycle time window when points exceed 250 so it continuously sweeps
            if (s_realtimeChart.PointCount() > 250) {
                s_realtimeChart.Clear();
                s_startTime = now;
                elapsed = 0.0;
            }
            s_realtimeChart.AppendPoint(elapsed, throughput);
        }
    }

    // Stream synthetic diagnostic logs periodically
    {
        double logDt = std::chrono::duration<double>(now - s_lastLogTime).count();
        if (logDt >= 2.5) {
            s_lastLogTime = now;
            char timeBuf[32];
            snprintf(timeBuf, sizeof(timeBuf), "%02d:%05.2f", (int)(elapsed / 60.0), std::fmod(elapsed, 60.0));
            static int s_logCounter = 0;
            s_logCounter++;
            if (s_logCounter % 4 == 0) {
                s_diagnosticLogs.push_back({ timeBuf, "WARN", "Buffer pressure exceeded 75% on ingestion sink", UiVariant::Warning });
            } else if (s_logCounter % 3 == 0) {
                s_diagnosticLogs.push_back({ timeBuf, "METRIC", "Heartbeat roundtrip latency: 1.82 ms (P99: 3.4 ms)", UiVariant::Info });
            } else {
                s_diagnosticLogs.push_back({ timeBuf, "INFO", "Batch processed 4096 records in SIMD pipeline", UiVariant::Success });
            }
            if (s_diagnosticLogs.size() > 80) {
                s_diagnosticLogs.pop_front();
            }
        }
    }

    // ========================================================================
    // 1. TOP HEADER & BRANDING CONTROLS
    // ========================================================================
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
    ImGui::Separator();
    ImGui::Spacing();

    // ========================================================================
    // 2. TWO-COLUMN SPLIT: LEFT SIDEBAR + RIGHT MAIN WORKSPACE
    // ========================================================================
    float totalAvailW = ImGui::GetContentRegionAvail().x;
    float sidebarW = std::clamp(210.0f * main_scale, 180.0f, totalAvailW * 0.28f);
    float mainW = totalAvailW - sidebarW - 12.0f * main_scale;

    // LEFT COLUMN: SIDEBAR MENU
    ImGui::BeginChild("ShowcaseSidebar", ImVec2(sidebarW, 0.0f), false, ImGuiWindowFlags_None);
    {
        SidebarMenu sidebar;

        if (sidebar.Item("nav_overview", "Overview", Icon(Icon::Target), s_currentNav == 0)) s_currentNav = 0;
        if (sidebar.Item("nav_telemetry", "Telemetry", Icon(Icon::LineChart), s_currentNav == 1)) s_currentNav = 1;
        if (sidebar.Item("nav_pipeline", "Pipelines", Icon(Icon::Play), s_currentNav == 2)) s_currentNav = 2;
        if (sidebar.Item("nav_nodes", "Cluster Nodes", Icon(Icon::Database), s_currentNav == 3)) s_currentNav = 3;
        if (sidebar.Item("nav_settings", "Settings", Icon(Icon::Cog), s_currentNav == 4)) s_currentNav = 4;

        sidebar.Spacing(10.0f);
        sidebar.Separator();
        sidebar.SectionTitle("PIPELINE CHANNELS");

        if (sidebar.BeginScrollRegion("##ChannelsScroll")) {
            for (size_t i = 0; i < s_channels.size(); ++i) {
                const auto& ch = s_channels[i];
                if (sidebar.ItemEx(ch.id.c_str(), ch.name.c_str(), ch.sublabel.c_str(), ch.statusColor, s_selectedChannel == (int)i)) {
                    s_selectedChannel = (int)i;
                    s_nodeIdentifier = ch.nodeId;
                }
            }
            sidebar.EndScrollRegion();
        }
    }
    ImGui::EndChild();

    ImGui::SameLine(0.0f, 12.0f * main_scale);

    // RIGHT COLUMN: SYSTEM STATUS BANNER + SCROLLABLE TABS
    ImGui::BeginChild("ShowcaseMainArea", ImVec2(mainW, 0.0f), false, ImGuiWindowFlags_None);
    {
        // 2.1 STATUS BANNER & DIGITAL INDICATORS
        {
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

            double liveThroughput = (s_liveFeed && s_deviceState == DeviceState::Running)
                                  ? (84.6 + 4.8 * std::sin(elapsed * 2.2)) : 0.0;
            double liveLatency = (s_liveFeed && s_deviceState == DeviceState::Running)
                               ? (14.2 + 2.1 * std::cos(elapsed * 1.5)) : 0.0;

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

        // 2.2 TABS WRAPPED IN SCROLL
        // Flag ImGuiTabBarFlags_FittingPolicyScroll enables horizontal scroll buttons
        // when multiple tabs exceed available width!
        ImGuiTabBarFlags tabFlags = ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_TabListPopupButton;
        if (ImGui::BeginTabBar("OmniKitShowcaseTabBar", tabFlags)) {

            // ----------------------------------------------------------------
            // TAB 1: TELEMETRY & PIPELINE
            // ----------------------------------------------------------------
            if (ImGui::BeginTabItem("Telemetry & Pipeline")) {
                // Wrap tab content in scrollable child container
                ImGui::BeginChild("##TabScroll_Telemetry", ImVec2(0, 0), false, ImGuiWindowFlags_None);
                {
                    float availInsideTab = ImGui::GetContentRegionAvail().x;
                    float colLeftW = (availInsideTab - 16.0f * main_scale) * 0.44f;
                    float colRightW = availInsideTab - colLeftW - 16.0f * main_scale;

                    // Left sub-column: Configuration Card
                    ImGui::BeginChild("SubColLeft", ImVec2(colLeftW, 0), false, ImGuiWindowFlags_None);
                    {
                        if (Card cfgCard("cfg_card", "Pipeline Configuration"); cfgCard) {
                            InputField::Text("##node_name", "Cluster Node Identifier", s_nodeIdentifier);

                            ImGui::Spacing();
                            InputField::Float("##throughput_limit", "Throughput Ceiling", s_throughputLimit, "MB/s");

                            ImGui::Spacing();
                            Toggle::Render("tog_tare", s_autoTare, "Auto-Zero Telemetry Baseline", "Zeros relative baseline counter on cycle start");
                            Toggle::Render("tog_stream", s_liveFeed, "Real-Time Telemetry Feed", "Streams continuous high-frequency metrics");
                            Toggle::Render("tog_acc", s_hardwareAcc, "Hardware Acceleration (AVX-512)", "Enables vectorized math operations");

                            ImGui::Spacing();
                            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Sampling Interval Preset:");
                            static const std::vector<float> sampleOptions = { 50.0f, 100.0f, 250.0f, 500.0f };
                            PresetGrid::Render(s_sampleInterval, sampleOptions, "ms", 0.0f, 4);

                            ImGui::Spacing();
                            ImGui::Spacing();

                            // Deploy Button cleanly rendered below the preset grid with dedicated height and spacing
                            // Avoids overlap with earlier form inputs
                            if (Button::Render("Deploy Pipeline Configuration",
                                               UiVariant::Primary,
                                               ImVec2(ImGui::GetContentRegionAvail().x, 38.0f * main_scale),
                                               Icon(Icon::Check))) {
                                s_deviceState = DeviceState::Running;
                                s_liveFeed = true;
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    // Right sub-column: High-Speed Realtime Chart & Job Quick List
                    ImGui::BeginChild("SubColRight", ImVec2(colRightW, 0), false, ImGuiWindowFlags_None);
                    {
                        if (Card chartCard("chart_card", "Real-Time Telemetry Stream (Throughput vs Elapsed Time)"); chartCard) {
                            float chartH = 260.0f * main_scale;
                            s_realtimeChart.Render("realtime_chart_view", ImVec2(colRightW - 32.0f * main_scale, chartH));
                        }

                        ImGui::Spacing();

                        if (Card queueCard("queue_card", "Job Execution Queue Snapshot"); queueCard) {
                            SearchInput::Render("tbl_search_mini", s_searchQuery, "Filter active jobs...");
                            ImGui::Spacing();

                            if (ImGui::BeginTable("JobDataTableMini", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 55.0f * main_scale);
                                ImGui::TableSetupColumn("Task Identifier");
                                ImGui::TableSetupColumn("Engine");
                                ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 100.0f * main_scale);
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
                                    Badge::Render(job.statusText, job.status);
                                }
                                ImGui::EndTable();
                            }
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ----------------------------------------------------------------
            // TAB 2: DISTRIBUTED JOB QUEUE (FULL TABLE IN SCROLL)
            // ----------------------------------------------------------------
            if (ImGui::BeginTabItem("Job Execution Queue")) {
                ImGui::BeginChild("##TabScroll_JobQueue", ImVec2(0, 0), false, ImGuiWindowFlags_None);
                {
                    if (Card tableCard("records_card_full", "Distributed Job Execution Queue (Full Dataset)"); tableCard) {
                        SearchInput::Render("tbl_search_full", s_searchQuery, "Search jobs across cluster nodes...");
                        ImGui::Spacing();

                        if (ImGui::BeginTable("JobDataTableFull", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 65.0f * main_scale);
                            ImGui::TableSetupColumn("Task Identifier");
                            ImGui::TableSetupColumn("Execution Engine");
                            ImGui::TableSetupColumn("Latency (ms)");
                            ImGui::TableSetupColumn("Memory (MB)");
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
                                ImGui::Text("%.2f ms", job.executionTimeMs);
                                ImGui::TableNextColumn();
                                ImGui::Text("%.1f MB", job.memoryUsageMb);
                                ImGui::TableNextColumn();
                                Badge::Render(job.statusText, job.status);
                            }
                            ImGui::EndTable();
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ----------------------------------------------------------------
            // TAB 3: NODE PARAMETERS (LONG SCROLLABLE SETTINGS FORM)
            // ----------------------------------------------------------------
            if (ImGui::BeginTabItem("Node Parameters (Scroll Form)")) {
                // Demonstrates deep multi-section scrollable settings form wrapped inside tab
                ImGui::BeginChild("##TabScroll_SettingsForm", ImVec2(0, 0), false, ImGuiWindowFlags_None);
                {
                    if (Card netCard("net_card", "Network & Socket Subsystem"); netCard) {
                        InputField::Float("##tcp_buf", "TCP Socket Buffer Size", s_tcpBufferSize, "KB");
                        ImGui::Spacing();
                        InputField::Float("##conn_timeout", "Connection Keepalive Timeout", s_connectionTimeout, "s");
                        ImGui::Spacing();
                        Toggle::Render("tog_keepalive", s_keepaliveEnabled, "Enable TCP Keepalive Probes", "Periodically tests idle connection liveness");
                        Toggle::Render("tog_zstd", s_compressionZstd, "Payload Compression (Zstandard)", "Compresses wire packets above 4 KB");
                    }

                    ImGui::Spacing();

                    if (Card computeCard("compute_card", "Compute & Parallelism Architecture"); computeCard) {
                        InputField::Int("##worker_th", "Worker Thread Concurrency", s_workerThreads);
                        ImGui::Spacing();
                        InputField::Text("##region", "Cluster Deployment Zone", s_clusterRegion);
                        ImGui::Spacing();
                        Toggle::Render("tog_numa", s_hardwareAcc, "NUMA Node Pinning & Core Affinity", "Binds memory allocations to the active socket");
                    }

                    ImGui::Spacing();

                    if (Card cacheCard("cache_card", "In-Memory Storage & Cache Quota"); cacheCard) {
                        InputField::Float("##cache_quota", "Max L1 Memory Quota", s_cacheQuotaMb, "MB");
                        ImGui::Spacing();
                        InputField::Float("##eviction_th", "Eviction High-Watermark", s_evictionThreshold, "%");
                        ImGui::Spacing();
                        InputField::Int("##repl_factor", "Distributed Replication Factor", s_replicationFactor);
                    }

                    ImGui::Spacing();
                    if (Button::Render("Apply Node Parameters", UiVariant::Primary, ImVec2(220.0f * main_scale, 36.0f * main_scale), Icon(Icon::Check))) {
                        s_diagnosticLogs.push_back({ "NOW", "SUCCESS", "Node parameters reloaded and propagated", UiVariant::Success });
                    }
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ----------------------------------------------------------------
            // TAB 4: CLUSTER DIAGNOSTICS (SCROLLABLE LOG STREAM)
            // ----------------------------------------------------------------
            if (ImGui::BeginTabItem("Diagnostic Logs (Stream)")) {
                ImGui::BeginChild("##TabScroll_LogStream", ImVec2(0, 0), false, ImGuiWindowFlags_None);
                {
                    if (Card logCard("log_card", "Live Distributed Cluster Event Log"); logCard) {
                        ImGui::Checkbox("Auto-scroll to bottom", &s_autoScrollLogs);
                        ImGui::SameLine();
                        if (Button::Render("Clear Logs", UiVariant::Secondary, Icon(Icon::Refresh), UiSize::Small)) {
                            s_diagnosticLogs.clear();
                        }

                        ImGui::Spacing();

                        // Inner scrollable terminal log view
                        ImGui::BeginChild("LogTerminalRegion", ImVec2(0, 340.0f * main_scale), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                        {
                            for (const auto& entry : s_diagnosticLogs) {
                                ImGui::TextColored(ImColor(theme.palette.textMuted).Value, "[%s]", entry.timeStr.c_str());
                                ImGui::SameLine();
                                Badge::Render(entry.level.c_str(), entry.variant);
                                ImGui::SameLine();
                                ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", entry.message.c_str());
                            }
                            if (s_autoScrollLogs && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                                ImGui::SetScrollHereY(1.0f);
                            }
                        }
                        ImGui::EndChild();
                    }
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ----------------------------------------------------------------
            // TAB 5: SEMANTIC DESIGN TOKENS GALLERY
            // ----------------------------------------------------------------
            if (ImGui::BeginTabItem("Semantic Tokens Gallery")) {
                ImGui::BeginChild("##TabScroll_TokensGallery", ImVec2(0, 0), false, ImGuiWindowFlags_None);
                {
                    if (Card tokensCard("tokens_card_full", "Design System Interactive Variants"); tokensCard) {
                        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Interactive Variants (UiVariant):");
                        ImGui::Spacing();

                        FlowLayout flow(ImGui::GetContentRegionAvail().x);
                        if (auto col = flow.Col(::Col::Third()); col) {
                            Button::Render("Primary", UiVariant::Primary, Icon(Icon::Play), UiSize::Medium);
                        }
                        if (auto col = flow.Col(::Col::Third()); col) {
                            Button::Render("Success", UiVariant::Success, Icon(Icon::Check), UiSize::Medium);
                        }
                        if (auto col = flow.Col(::Col::Third()); col) {
                            Button::Render("Warning", UiVariant::Warning, Icon(Icon::Pause), UiSize::Medium);
                        }
                        if (auto col = flow.Col(::Col::Third()); col) {
                            Button::Render("Danger", UiVariant::Danger, Icon(Icon::Close), UiSize::Medium);
                        }
                        if (auto col = flow.Col(::Col::Third()); col) {
                            Button::Render("Info", UiVariant::Info, Icon(Icon::Target), UiSize::Medium);
                        }
                        if (auto col = flow.Col(::Col::Third()); col) {
                            Button::Render("Secondary", UiVariant::Secondary, Icon(Icon::Cog), UiSize::Medium);
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Status Badges & Tags:");
                        Badge::Render("ONLINE", UiVariant::Success);
                        ImGui::SameLine();
                        Badge::Render("READY", UiVariant::Primary);
                        ImGui::SameLine();
                        Badge::Render("STANDBY", UiVariant::Secondary);
                        ImGui::SameLine();
                        Badge::Render("ALERT", UiVariant::Danger);
                        ImGui::SameLine();
                        Badge::Render("SYNCING", UiVariant::Warning);
                        ImGui::SameLine();
                        Tag::Render("HTTP/3");
                        ImGui::SameLine();
                        Tag::Render("AVX-512");
                        ImGui::SameLine();
                        Tag::Render("ZERO-COPY");

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Editable Label Widget:");
                        static std::string s_sampleLabel = "Production Cluster Node 07";
                        static EditableLabel s_sampleEditableLabel;
                        s_sampleEditableLabel.Render("sample_edit_label", s_sampleLabel, 340.0f * main_scale);
                    }
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ----------------------------------------------------------------
            // TAB 6: MEMORY & BUFFERS
            // ----------------------------------------------------------------
            if (ImGui::BeginTabItem("Memory & Buffers")) {
                ImGui::BeginChild("##TabScroll_Memory", ImVec2(0, 0), false, ImGuiWindowFlags_None);
                {
                    if (Card memCard("mem_card", "Cluster Buffer Pool Allocator"); memCard) {
                        ValueDisplay::Render(14.8, "GB", 0, 48.0f * main_scale, "%.1f");
                        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Total Allocated Slab Cache");
                        ImGui::Spacing();
                        ValueDisplay::Render(99.4, "%", 0, 48.0f * main_scale, "%.1f");
                        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Cache Hit Rate (Past 10m)");
                    }
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            // ----------------------------------------------------------------
            // TAB 7: SECURITY & TLS
            // ----------------------------------------------------------------
            if (ImGui::BeginTabItem("Security & TLS")) {
                ImGui::BeginChild("##TabScroll_Security", ImVec2(0, 0), false, ImGuiWindowFlags_None);
                {
                    if (Card secCard("sec_card", "Cryptographic Protocol Status"); secCard) {
                        Badge::Render("TLS 1.3 ACTIVE", UiVariant::Success);
                        ImGui::SameLine();
                        Badge::Render("ECDHE-RSA-AES256-GCM", UiVariant::Primary);
                        ImGui::Spacing();
                        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Key Exchange Latency: 1.12 ms");
                        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "Certificate Fingerprint: SHA256:7f:8c:12:44:90:de:bc");
                    }
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
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
