#include "CardsPage.hpp"
#include "omnikit.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace OmniKitShowcase {

using namespace OmniKit;

namespace {

// ============================================================================
// Top-Level Tab Navigation
// ============================================================================
enum class CardsTab : int {
    Archive = 0,
    Processing,
    Geometry,
};

static int s_activeTab = 0;
static const char* const kTabLabels[] = {
    "Archive",
    "Processing",
    "Section Geometry"
};

// ============================================================================
// Format Helpers
// ============================================================================
std::string FormatForceKn(double newtons) {
    if (newtons <= 0.0) return "—";
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f kN", newtons / 1000.0);
    return buffer;
}

std::string FormatDuration(double seconds) {
    if (seconds < 0.1) return "—";
    char buffer[32];
    const int total = static_cast<int>(seconds);
    if (total >= 60) {
        std::snprintf(buffer, sizeof(buffer), "%d min %02d s", total / 60, total % 60);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%.1f s", seconds);
    }
    return buffer;
}

inline bool CaseInsensitiveContains(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    auto it = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char ch1, char ch2) {
            return std::tolower(static_cast<unsigned char>(ch1)) ==
                   std::tolower(static_cast<unsigned char>(ch2));
        }
    );
    return it != haystack.end();
}

// ============================================================================
// TAB 1: Archive Screen (Direct Port of ArchiveScreen Architecture)
// ============================================================================
struct ArchiveRecord {
    uint32_t    id;
    std::string project;
    std::string series;
    std::string specimen;
    std::string startedAt;      // Display string "YYYY-MM-DD HH:MM"
    int64_t     startedTimestamp; // Unix seconds
    double      fMaxN;          // Maximum force in Newtons
    double      durationSeconds;
    std::string operatorName;
    std::string machineId;
    std::string standard;
};

enum class ArchivePeriod : int {
    Week = 0,
    Month,
    Quarter,
    Year,
    All
};

static const char* const kPeriodLabels[] = {
    "Week", "Month", "Quarter", "Year", "All Time"
};
constexpr int kPeriodCount = static_cast<int>(sizeof(kPeriodLabels) / sizeof(kPeriodLabels[0]));

enum ArchiveColumnIndex {
    ColStarted = 0,
    ColProject = 1,
    ColSeries = 2,
    ColSample = 3,
    ColForce = 4,
    ColDuration = 5,
    ColActions = 6
};

// Base widths of hideable columns in order of hiding (first hidden first)
struct HideableColumn {
    int column;
    float width;
};
constexpr int kArchiveHideableCount = 4;
constexpr HideableColumn kArchiveHideOrder[kArchiveHideableCount] = {
    {ColDuration, 120.0f},
    {ColForce, 110.0f},
    {ColSeries, 120.0f},
    {ColStarted, 150.0f},
};
constexpr float kArchiveLabelFontSize = 18.0f;
constexpr float kArchiveProjectMin = 160.0f;
constexpr float kArchiveSampleMin = 140.0f;
constexpr float kArchiveActionsWidth = 130.0f;

// Archive state
static std::vector<ArchiveRecord> s_archiveStore;
static bool s_archiveStoreInitialized = false;

static Table<uint32_t> s_archiveTable({.key = "ArchiveTable", .rowHeight = 36.0f});
static bool s_archiveTableInitialized = false;

static std::string s_filterProject;
static std::string s_filterSeries;
static std::string s_filterSample;
static int s_filterPeriod = static_cast<int>(ArchivePeriod::Month);

static bool s_archiveOpenModal = false;
static ArchiveRecord s_selectedRecord;

void InitArchiveStore() {
    if (s_archiveStoreInitialized) return;
    s_archiveStoreInitialized = true;

    // Unix epoch reference: 2026-10-04 12:00:00 UTC (~1791115200)
    const int64_t nowSec = 1791115200;

    s_archiveStore = {
        { 101, "Alloy Tensile QA",     "Batch-2026-A1", "Specimen #01", "2026-10-04 11:42", nowSec - 3600 * 2,    142500.0, 108.4, "J. Doe",    "UTM-600kN", "ASTM E8" },
        { 102, "Alloy Tensile QA",     "Batch-2026-A1", "Specimen #02", "2026-10-04 10:15", nowSec - 3600 * 4,    144200.0, 112.1, "J. Doe",    "UTM-600kN", "ASTM E8" },
        { 103, "Alloy Tensile QA",     "Batch-2026-A1", "Specimen #03", "2026-10-03 16:30", nowSec - 86400 * 1,   141900.0, 105.8, "M. Smith",  "UTM-600kN", "ASTM E8" },
        { 104, "Alloy Tensile QA",     "Batch-2026-A2", "Specimen #01", "2026-10-02 14:05", nowSec - 86400 * 2,   151300.0, 119.5, "M. Smith",  "UTM-600kN", "ASTM E8" },
        { 105, "Alloy Tensile QA",     "Batch-2026-A2", "Specimen #02", "2026-10-01 09:20", nowSec - 86400 * 3,   152800.0, 124.0, "J. Doe",    "UTM-600kN", "ASTM E8" },
        { 106, "Titanium Fatigue Study", "Series-Ti-01",  "Ti-Plate-01",  "2026-09-29 15:10", nowSec - 86400 * 5,    98400.0,  340.2, "A. Vance",  "UTM-250kN", "ISO 6892-1" },
        { 107, "Titanium Fatigue Study", "Series-Ti-01",  "Ti-Plate-02",  "2026-09-28 11:35", nowSec - 86400 * 6,    99100.0,  328.7, "A. Vance",  "UTM-250kN", "ISO 6892-1" },
        { 108, "Titanium Fatigue Study", "Series-Ti-02",  "Ti-Plate-03",  "2026-09-25 17:00", nowSec - 86400 * 9,    102300.0, 355.0, "A. Vance",  "UTM-250kN", "ISO 6892-1" },
        { 109, "Carbon Fiber Strip",   "CF-T700-UD",    "Coupon-C1",    "2026-09-20 13:45", nowSec - 86400 * 14,   38200.0,  54.6,  "K. Green",  "UTM-100kN", "ASTM D3039" },
        { 110, "Carbon Fiber Strip",   "CF-T700-UD",    "Coupon-C2",    "2026-09-19 10:20", nowSec - 86400 * 15,   39100.0,  58.2,  "K. Green",  "UTM-100kN", "ASTM D3039" },
        { 111, "Carbon Fiber Strip",   "CF-T700-Fabric","Coupon-F1",    "2026-09-15 16:50", nowSec - 86400 * 19,   44800.0,  72.0,  "K. Green",  "UTM-100kN", "ASTM D3039" },
        { 112, "Steel Rebar Tensile",  "Rebar-Grade60", "Bar-Ø25-01",   "2026-09-10 09:00", nowSec - 86400 * 24,   265000.0, 185.3, "D. Miller", "UTM-600kN", "ASTM A615" },
        { 113, "Steel Rebar Tensile",  "Rebar-Grade60", "Bar-Ø25-02",   "2026-09-08 14:25", nowSec - 86400 * 26,   268400.0, 192.0, "D. Miller", "UTM-600kN", "ASTM A615" },
        { 114, "Steel Rebar Tensile",  "Rebar-Grade60", "Bar-Ø25-03",   "2026-09-05 11:10", nowSec - 86400 * 29,   263900.0, 178.6, "D. Miller", "UTM-600kN", "ASTM A615" },
        { 115, "Automotive Axle Shear","Axle-Forging",  "Shaft-S01",    "2026-08-28 15:30", nowSec - 86400 * 37,   185600.0, 210.4, "R. Taylor", "UTM-600kN", "DIN 50125" },
        { 116, "Automotive Axle Shear","Axle-Forging",  "Shaft-S02",    "2026-08-20 10:40", nowSec - 86400 * 45,   187200.0, 215.1, "R. Taylor", "UTM-600kN", "DIN 50125" },
        { 117, "Cryogenic Weld Test",  "316L-Weld-77K", "Weld-Coupon-1","2026-08-05 14:15", nowSec - 86400 * 60,   118900.0, 145.0, "A. Vance",  "UTM-250kN", "ISO 15614" },
        { 118, "Cryogenic Weld Test",  "316L-Weld-77K", "Weld-Coupon-2","2026-07-25 16:00", nowSec - 86400 * 71,   121400.0, 150.2, "A. Vance",  "UTM-250kN", "ISO 15614" },
        { 119, "Polymer Tensile Mod",  "PEEK-Optima",   "Dogbone-P1",   "2026-07-10 11:20", nowSec - 86400 * 86,   14800.0,  88.5,  "M. Smith",  "UTM-100kN", "ASTM D638" },
        { 120, "High-Torque Coupling", "Coupling-HT",   "Sleeve-C1",    "2026-06-15 09:45", nowSec - 86400 * 111,  215000.0, 260.0, "R. Taylor", "UTM-600kN", "ISO 6892-1" },
        { 121, "High-Torque Coupling", "Coupling-HT",   "Sleeve-C2",    "2026-05-20 14:30", nowSec - 86400 * 137,  218400.0, 264.8, "R. Taylor", "UTM-600kN", "ISO 6892-1" },
        { 122, "Aluminum 7075-T6",     "Aero-Spar-Ext", "Rib-Ext-01",   "2026-04-12 10:00", nowSec - 86400 * 175,  84200.0,  95.4,  "J. Doe",    "UTM-250kN", "ASTM B557" },
        { 123, "Aluminum 7075-T6",     "Aero-Spar-Ext", "Rib-Ext-02",   "2026-03-01 15:15", nowSec - 86400 * 217,  85600.0,  98.1,  "J. Doe",    "UTM-250kN", "ASTM B557" },
        { 124, "Structural Fasteners", "Bolt-M16-10.9", "Bolt-Test-01", "2026-01-15 13:40", nowSec - 86400 * 262,  162000.0, 75.0,  "D. Miller", "UTM-600kN", "ISO 898-1" },
        { 125, "Structural Fasteners", "Bolt-M16-10.9", "Bolt-Test-02", "2025-11-20 11:05", nowSec - 86400 * 318,  164500.0, 78.4,  "D. Miller", "UTM-600kN", "ISO 898-1" },
    };
}

int64_t ArchivePeriodStart(ArchivePeriod period, int64_t nowSeconds) {
    int days = 0;
    switch (period) {
        case ArchivePeriod::Week:    days = 7;   break;
        case ArchivePeriod::Month:   days = 30;  break;
        case ArchivePeriod::Quarter: days = 90;  break;
        case ArchivePeriod::Year:    days = 365; break;
        case ArchivePeriod::All:     return 0;
    }
    const int64_t today = (nowSeconds / 86400) * 86400;
    return today - static_cast<int64_t>(days) * 86400;
}

void RefilterArchive() {
    InitArchiveStore();

    const int64_t nowSec = 1791115200;
    const int64_t minTimestamp = ArchivePeriodStart(static_cast<ArchivePeriod>(s_filterPeriod), nowSec);

    std::vector<uint32_t> matchedIndices;
    matchedIndices.reserve(s_archiveStore.size());

    for (uint32_t i = 0; i < static_cast<uint32_t>(s_archiveStore.size()); ++i) {
        const ArchiveRecord& r = s_archiveStore[i];

        if (minTimestamp > 0 && r.startedTimestamp < minTimestamp) continue;
        if (!CaseInsensitiveContains(r.project, s_filterProject)) continue;
        if (!CaseInsensitiveContains(r.series, s_filterSeries)) continue;
        if (!CaseInsensitiveContains(r.specimen, s_filterSample)) continue;

        matchedIndices.push_back(i);
    }

    s_archiveTable.SetItems(std::move(matchedIndices));
}

float ArchiveRequiredWidth(int hiddenCount) {
    const UiTheme& theme = UiTheme::Get();
    const float pad = theme.Scale(theme.table.cellPaddingX * 2.0f);
    float width = theme.Scale(kArchiveProjectMin) + theme.Scale(kArchiveSampleMin) +
                  theme.Scale(kArchiveActionsWidth) + pad * 3.0f;
    for (int i = hiddenCount; i < kArchiveHideableCount; ++i) {
        width += theme.Scale(kArchiveHideOrder[i].width) + pad;
    }
    return width;
}

void UpdateArchiveColumnVisibility(float availWidth) {
    if (availWidth <= 0.0f) {
        availWidth = ImGui::GetContentRegionAvail().x;
    }
    int hidden = 0;
    while (hidden < kArchiveHideableCount && availWidth < ArchiveRequiredWidth(hidden)) {
        ++hidden;
    }
    for (int i = 0; i < kArchiveHideableCount; ++i) {
        s_archiveTable.SetColumnVisible(static_cast<size_t>(kArchiveHideOrder[i].column), i >= hidden);
    }
    s_archiveTable.SetColumnVisible(ColProject, true);
    s_archiveTable.SetColumnVisible(ColSample, true);
    s_archiveTable.SetColumnVisible(ColActions, true);
}

void InitArchiveTable() {
    if (s_archiveTableInitialized) return;
    s_archiveTableInitialized = true;

    InitArchiveStore();

    const auto recOf = [](uint32_t idx) -> const ArchiveRecord& {
        return s_archiveStore[idx];
    };

    // 0. Started
    TableColumn<uint32_t> started;
    started.id = "started";
    started.header = "Started";
    started.width = 150.0f;
    started.widthMode = ColumnWidthMode::Fixed;
    started.resizable = false;
    started.comparator = [recOf](const uint32_t& a, const uint32_t& b) {
        const int64_t ta = recOf(a).startedTimestamp;
        const int64_t tb = recOf(b).startedTimestamp;
        return ta < tb ? 1 : (ta > tb ? -1 : 0); // newest first default
    };
    started.renderCell = [recOf](const uint32_t& row, int) {
        TableCell::Centered(recOf(row).startedAt, UiTheme::Get().table.cellFontSize,
                            UiTheme::Get().palette.textMuted);
    };
    s_archiveTable.AddColumn(std::move(started));

    // 1. Project
    TableColumn<uint32_t> project;
    project.id = "project";
    project.header = "Project";
    project.width = 1.4f;
    project.widthMode = ColumnWidthMode::Stretch;
    project.resizable = false;
    project.hideable = false;
    project.comparator = [recOf](const uint32_t& a, const uint32_t& b) {
        return recOf(a).project.compare(recOf(b).project);
    };
    project.renderCell = [recOf](const uint32_t& row, int) {
        TableCell::Centered(recOf(row).project, kArchiveLabelFontSize);
    };
    s_archiveTable.AddColumn(std::move(project));

    // 2. Series
    TableColumn<uint32_t> series;
    series.id = "series";
    series.header = "Series";
    series.width = 1.0f;
    series.widthMode = ColumnWidthMode::Stretch;
    series.resizable = false;
    series.comparator = [recOf](const uint32_t& a, const uint32_t& b) {
        return recOf(a).series.compare(recOf(b).series);
    };
    series.renderCell = [recOf](const uint32_t& row, int) {
        TableCell::Centered(recOf(row).series, kArchiveLabelFontSize);
    };
    s_archiveTable.AddColumn(std::move(series));

    // 3. Specimen
    TableColumn<uint32_t> sample;
    sample.id = "sample";
    sample.header = "Specimen";
    sample.width = 1.0f;
    sample.widthMode = ColumnWidthMode::Stretch;
    sample.resizable = false;
    sample.hideable = false;
    sample.comparator = [recOf](const uint32_t& a, const uint32_t& b) {
        return recOf(a).specimen.compare(recOf(b).specimen);
    };
    sample.renderCell = [recOf](const uint32_t& row, int) {
        TableCell::Centered(recOf(row).specimen, kArchiveLabelFontSize);
    };
    s_archiveTable.AddColumn(std::move(sample));

    // 4. Force (P_max)
    TableColumn<uint32_t> force;
    force.id = "force";
    force.header = "P_max";
    force.width = 110.0f;
    force.widthMode = ColumnWidthMode::Fixed;
    force.resizable = false;
    force.comparator = [recOf](const uint32_t& a, const uint32_t& b) {
        const double fa = recOf(a).fMaxN, fb = recOf(b).fMaxN;
        return fa < fb ? -1 : (fa > fb ? 1 : 0);
    };
    force.renderCell = [recOf](const uint32_t& row, int) {
        TableCell::Centered(FormatForceKn(recOf(row).fMaxN), UiTheme::Get().table.cellFontSize);
    };
    s_archiveTable.AddColumn(std::move(force));

    // 5. Duration
    TableColumn<uint32_t> duration;
    duration.id = "duration";
    duration.header = "Duration";
    duration.width = 120.0f;
    duration.widthMode = ColumnWidthMode::Fixed;
    duration.resizable = false;
    duration.comparator = [recOf](const uint32_t& a, const uint32_t& b) {
        const double da = recOf(a).durationSeconds, db = recOf(b).durationSeconds;
        return da < db ? -1 : (da > db ? 1 : 0);
    };
    duration.renderCell = [recOf](const uint32_t& row, int) {
        TableCell::Centered(FormatDuration(recOf(row).durationSeconds), UiTheme::Get().table.cellFontSize);
    };
    s_archiveTable.AddColumn(std::move(duration));

    // 6. Actions
    TableColumn<uint32_t> actions;
    actions.id = "actions";
    actions.header = "";
    actions.width = kArchiveActionsWidth;
    actions.widthMode = ColumnWidthMode::Fixed;
    actions.align = ColumnAlign::Right;
    actions.sortable = false;
    actions.resizable = false;
    actions.renderCell = [recOf](const uint32_t& row, int rowIndex) {
        (void)rowIndex;
        if (ActionGroup actionGroup; actionGroup) {
            if (actionGroup.Button({.label = "Open", .variant = UiVariant::Primary, .icon = Icon::ExternalLink})) {
                s_selectedRecord = recOf(row);
                s_archiveOpenModal = true;
            }
        }
    };
    s_archiveTable.AddColumn(std::move(actions));

    RefilterArchive();
}

void RenderArchiveTab() {
    InitArchiveTable();

    const UiTheme& theme = UiTheme::Get();
    const float avail = ImGui::GetContentRegionAvail().x;
    const float gap = theme.Scale(12.0f);
    const float periodWidth = theme.Scale(170.0f);
    const float fieldWidth = std::max(theme.Scale(120.0f), (avail - periodWidth - gap * 3.0f) / 3.0f);

    // Filter Bar
    bool queryChanged = false;
    queryChanged |= InputField(s_filterProject, {.label = "Project", .hint = " — ", .width = fieldWidth});
    ImGui::SameLine(0.0f, gap);
    queryChanged |= InputField(s_filterSeries,  {.label = "Series",  .hint = " — ", .width = fieldWidth});
    ImGui::SameLine(0.0f, gap);
    queryChanged |= InputField(s_filterSample,  {.label = "Specimen",.hint = " — ", .width = fieldWidth});
    ImGui::SameLine(0.0f, gap);
    queryChanged |= Combo(s_filterPeriod, kPeriodLabels, kPeriodCount, {.label = "Period", .width = periodWidth});

    if (queryChanged) {
        RefilterArchive();
    }

    // Summary Bar
    ImGui::Dummy(ImVec2(0.0f, theme.Scale(4.0f)));
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("Found: %zu of %zu", s_archiveTable.GetFilteredCount(), s_archiveStore.size());
    ImGui::SameLine(0.0f, theme.Scale(16.0f));

    if (Button({.label = "Refresh", .variant = UiVariant::Secondary, .icon = Icon::Refresh, .size = UiSize::Small})) {
        RefilterArchive();
    }
    ImGui::SameLine(0.0f, theme.Scale(8.0f));

    if (Button({.label = "Archive Folder", .variant = UiVariant::Secondary, .icon = Icon::Folder, .size = UiSize::Small})) {
        // Mock explorer folder action
    }

    ImGui::Dummy(ImVec2(0.0f, theme.Scale(4.0f)));

    // Adaptive Column Visibility & Virtualized Table Render
    const float availW = ImGui::GetContentRegionAvail().x;
    UpdateArchiveColumnVisibility(availW);
    s_archiveTable.Render();

    // Inspection Modal for Selected Record
    if (Modal modal(s_archiveOpenModal, {.title = "Test Session Details", .width = 540.0f, .height = 420.0f}); modal) {
        if (auto content = modal.Content()) {
            if (Card card({.title = "Test Passport & Execution Metadata", .key = "ArchiveInspectCard"}); card) {
                card.Display(s_selectedRecord.project,  {.label = "Project Name", .col = Col::Full()});
                card.Display(s_selectedRecord.series,   {.label = "Series Identifier", .col = Col::Half()});
                card.Display(s_selectedRecord.specimen, {.label = "Specimen Name", .col = Col::Half()});
                card.Display(s_selectedRecord.startedAt,{.label = "Date & Time", .col = Col::Half()});
                card.Display(s_selectedRecord.standard, {.label = "Testing Standard", .col = Col::Half()});
                card.Display(FormatForceKn(s_selectedRecord.fMaxN), {.label = "Peak Force (P_max)", .col = Col::Half()});
                card.Display(FormatDuration(s_selectedRecord.durationSeconds), {.label = "Total Duration", .col = Col::Half()});
                card.Display(s_selectedRecord.operatorName, {.label = "Operator", .col = Col::Half()});
                card.Display(s_selectedRecord.machineId,    {.label = "Test Rig / Machine", .col = Col::Half()});
            }
        }
        if (auto footer = modal.Footer()) {
            if (footer.Actions({.confirmLabel = "Close"})) {
                s_archiveOpenModal = false;
            }
        }
    }
}

// ============================================================================
// TAB 2: Processing Tab (Direct Port of ProcessingTab Architecture)
// ============================================================================
struct MetricConfig {
    std::string metricId;       // Unique ID
    std::string algorithmId;    // Empty for user-defined formula
    std::string name;           // Metric name
    std::string symbol;         // Formula symbol
    std::string unit;           // Engineering unit
    std::string formulaExpr;    // Calculation formula expression
    std::string description;
};

struct CatalogAlgorithm {
    std::string id;
    std::string name;
    std::string symbol;
    std::string unit;
    std::string description;
    std::string expression;
    std::vector<std::string> tags;
};

static const CatalogAlgorithm kCatalogAlgorithms[] = {
    {
        "p_max", "Peak Force (Max Load)", "F_max", "kN",
        "Highest tensile force registered during the continuous mechanical test run.",
        "max(Force)",
        {"force", "peak", "gost", "astm"}
    },
    {
        "yield_02", "Yield Strength (0.2% Proof)", "R_p0.2", "MPa",
        "Engineering stress corresponding to 0.2% non-proportional plastic elongation offset.",
        "Stress_at_plastic_strain(0.2%)",
        {"strength", "yield", "gost", "metals"}
    },
    {
        "tensile_rm", "Tensile Strength (UTS)", "R_m", "MPa",
        "Stress corresponding to the maximum engineering tensile force: R_m = F_max / S_0.",
        "F_max / S_0",
        {"strength", "ultimate", "fracture", "metals"}
    },
    {
        "youngs_modulus", "Modulus of Elasticity", "E", "GPa",
        "Slope of the initial linear elastic deformation region on the engineering stress-strain curve.",
        "delta_Stress / delta_Strain",
        {"elasticity", "slope", "modulus"}
    },
    {
        "elongation_a", "Percentage Elongation", "A", "%",
        "Permanent elongation of the gauge length after rupture relative to initial length L_0.",
        "(L_u - L_0) / L_0 * 100",
        {"ductility", "elongation", "fracture"}
    },
    {
        "reduction_z", "Percentage Reduction of Area", "Z", "%",
        "Maximum change in cross-sectional area at fracture necking relative to original S_0.",
        "(S_0 - S_u) / S_0 * 100",
        {"ductility", "necking"}
    },
    {
        "sg_filter", "Savitzky-Golay Smoothing", "SG", "—",
        "Digital polynomial regression filter that suppresses high-frequency transducer noise while preserving peaks.",
        "sgolay(Signal, window=15, order=3)",
        {"filter", "polynomial", "smoothing"}
    },
    {
        "bw_lp", "Butterworth Lowpass Filter", "BW_lp", "—",
        "IIR lowpass filter providing maximally flat frequency response in passband.",
        "butterworth_lp(Signal, cutoff=25.0, order=4)",
        {"filter", "iir", "lowpass"}
    },
    {
        "true_stress", "True Stress / True Strain", "sigma_true", "MPa",
        "Instantaneous true tensile stress accounting for continuous specimen necking.",
        "Stress * (1 + Strain)",
        {"plasticity", "true_stress"}
    },
    {
        "toughness_w", "Fracture Toughness Work", "W", "J",
        "Total mechanical energy absorbed by the specimen up to complete fracture (integral under F-L).",
        "trapz(Force, Displacement)",
        {"energy", "toughness", "integral"}
    }
};
constexpr size_t kCatalogAlgorithmCount = sizeof(kCatalogAlgorithms) / sizeof(kCatalogAlgorithms[0]);

// Processing Tab State
static std::vector<MetricConfig> s_activeMethod;
static bool s_methodInitialized = false;
static std::string s_procSearchQuery;

// Modals
static bool s_clearMethodModalOpen = false;

static bool s_metricEditModalOpen = false;
static MetricConfig s_editingMetric;
static bool s_isNewCustomMetric = false;

bool InMethod(const std::vector<MetricConfig>& method, const CatalogAlgorithm& a) {
    return std::any_of(method.begin(), method.end(), [&a](const MetricConfig& m) {
        return m.algorithmId == a.id && m.symbol == a.symbol;
    });
}

bool IsOverriddenFormula(const MetricConfig& m) {
    if (m.algorithmId.empty()) return false;
    for (size_t i = 0; i < kCatalogAlgorithmCount; ++i) {
        if (kCatalogAlgorithms[i].id == m.algorithmId) {
            return m.formulaExpr != kCatalogAlgorithms[i].expression;
        }
    }
    return false;
}

std::set<std::string> DuplicateSymbols(const std::vector<MetricConfig>& method) {
    std::map<std::string, int> counts;
    for (const MetricConfig& m : method) {
        if (!m.symbol.empty()) ++counts[m.symbol];
    }
    std::set<std::string> duplicates;
    for (const auto& [sym, count] : counts) {
        if (count > 1) duplicates.insert(sym);
    }
    return duplicates;
}

void InitProcessingPreset() {
    if (s_methodInitialized) return;
    s_methodInitialized = true;

    // Standard tensile method (metals according to ASTM E8 / ISO 6892-1 / GOST 1497)
    s_activeMethod = {
        { "m_1", "p_max",          "Peak Force (Max Load)",        "F_max",   "kN",  "max(Force)",                       "Highest tensile force registered." },
        { "m_2", "yield_02",       "Yield Strength (0.2% Proof)",  "R_p0.2",  "MPa", "Stress_at_plastic_strain(0.2%)",   "Proof stress at 0.2% plastic strain." },
        { "m_3", "tensile_rm",     "Tensile Strength (UTS)",       "R_m",     "MPa", "F_max / S_0",                      "Ultimate tensile strength." },
        { "m_4", "youngs_modulus", "Modulus of Elasticity",        "E",       "GPa", "delta_Stress / delta_Strain",      "Young's modulus slope." },
        { "m_5", "elongation_a",   "Percentage Elongation",        "A",       "%",   "(L_u - L_0) / L_0 * 100",          "Permanent elongation." },
        { "m_6", "reduction_z",    "Percentage Reduction of Area", "Z",       "%",   "(S_0 - S_u) / S_0 * 100",          "Reduction of area at fracture." }
    };
}

void RenderCatalogCard(const std::vector<MetricConfig>& method) {
    if (Card card({.title = "Algorithm & Formula Catalog", .key = "ProcCatalogCard", .stretchY = true}); card) {
        SearchInput(s_procSearchQuery, {.hint = "Search by name, symbol, formula or tag (case-insensitive)...", .width = Fill});
        card.NextRow();

        const bool hasQuery = s_procSearchQuery.find_first_not_of(" \t") != std::string::npos;

        std::vector<const CatalogAlgorithm*> filtered;
        for (size_t i = 0; i < kCatalogAlgorithmCount; ++i) {
            const CatalogAlgorithm& a = kCatalogAlgorithms[i];
            if (!hasQuery) {
                filtered.push_back(&a);
                continue;
            }
            if (CaseInsensitiveContains(a.name, s_procSearchQuery) ||
                CaseInsensitiveContains(a.symbol, s_procSearchQuery) ||
                CaseInsensitiveContains(a.expression, s_procSearchQuery)) {
                filtered.push_back(&a);
                continue;
            }
            bool tagMatch = false;
            for (const auto& tag : a.tags) {
                if (CaseInsensitiveContains(tag, s_procSearchQuery)) {
                    tagMatch = true;
                    break;
                }
            }
            if (tagMatch) {
                filtered.push_back(&a);
            }
        }

        if (hasQuery) {
            const bool canAddAny = std::any_of(filtered.begin(), filtered.end(),
                [&method](const CatalogAlgorithm* a) { return !InMethod(method, *a); });

            Text(Format("Found: %zu", filtered.size()));
            SameLine(10.0f);

            if (Button({.label = "Add all found", .variant = UiVariant::Secondary,
                        .icon = Icon::ChevronRight, .size = UiSize::Small, .disabled = !canAddAny})) {
                for (const CatalogAlgorithm* a : filtered) {
                    if (!InMethod(s_activeMethod, *a)) {
                        MetricConfig m;
                        m.metricId = Format("m_%u", static_cast<uint32_t>(s_activeMethod.size() + 100));
                        m.algorithmId = a->id;
                        m.name = a->name;
                        m.symbol = a->symbol;
                        m.unit = a->unit;
                        m.formulaExpr = a->expression;
                        m.description = a->description;
                        s_activeMethod.push_back(std::move(m));
                    }
                }
            }
            Spacer(6);
        }

        if (filtered.empty()) {
            Text("No algorithms found matching your search query.");
            return;
        }

        for (const CatalogAlgorithm* a : filtered) {
            if (ItemRow item({.key = a->id.c_str(), .actions = 1}); item) {
                item.Title(a->name, {.symbol = a->symbol.c_str(), .unit = a->unit.c_str()});

                const bool inMethod = InMethod(method, *a);
                if (inMethod) {
                    item.Badge("In Method", {.variant = UiVariant::Success});
                }

                const char* tooltip = inMethod ? "Already in method — add another instance" : "Add to method";
                if (item.Action({.icon = Icon::ChevronRight, .tooltip = tooltip, .variant = UiVariant::Secondary})) {
                    MetricConfig m;
                    m.metricId = Format("m_%u", static_cast<uint32_t>(s_activeMethod.size() + 100));
                    m.algorithmId = a->id;
                    m.name = a->name;
                    m.symbol = a->symbol;
                    m.unit = a->unit;
                    m.formulaExpr = a->expression;
                    m.description = a->description;
                    s_activeMethod.push_back(std::move(m));
                }

                item.Description(a->description);
                if (!a->expression.empty()) {
                    item.Formula(a->symbol, a->expression);
                }

                auto clickedTag = item.Tags(a->tags);
                if (clickedTag) {
                    s_procSearchQuery = *clickedTag;
                }
            }
            Spacer(4);
        }
    }
}

void RenderMethodCard(const std::vector<MetricConfig>& method) {
    const std::set<std::string> duplicates = DuplicateSymbols(method);

    if (Card card({.title = "Test Processing Method", .key = "ProcMethodCard", .stretchY = true}); card) {
        if (Button({.label = "Custom Formula", .variant = UiVariant::Secondary, .icon = Icon::Plus, .size = UiSize::Small})) {
            s_editingMetric = {};
            s_editingMetric.metricId = Format("custom_%u", static_cast<uint32_t>(s_activeMethod.size() + 1));
            s_editingMetric.name = "Custom Metric";
            s_editingMetric.symbol = "X";
            s_editingMetric.unit = "MPa";
            s_editingMetric.formulaExpr = "F_max / 100.0";
            s_isNewCustomMetric = true;
            s_metricEditModalOpen = true;
        }

        AlignRight(140.0f);

        if (Button({.label = "Clear Method", .variant = UiVariant::Danger, .icon = Icon::Trash,
                    .size = UiSize::Small, .disabled = method.empty()})) {
            s_clearMethodModalOpen = true;
        }
        Spacer(8);

        if (method.empty()) {
            Text("No metrics in method.");
            Text("Add required algorithms and formulas from the catalog on the left.");
            return;
        }

        int removeIndex = -1;

        for (size_t i = 0; i < method.size(); ++i) {
            const MetricConfig& m = method[i];

            if (ItemRow item({.key = m.metricId.c_str(), .actions = 2}); item) {
                item.Title(m.name, {.symbol = m.symbol.c_str(), .unit = m.unit.c_str()});

                if (m.algorithmId.empty()) {
                    item.Badge("Custom", {.variant = UiVariant::Warning});
                } else if (IsOverriddenFormula(m)) {
                    item.Badge("Custom Expression", {.variant = UiVariant::Info});
                }
                if (duplicates.count(m.symbol) > 0) {
                    item.Badge("Duplicate", {.variant = UiVariant::Danger});
                }

                if (item.Action({.icon = Icon::Pencil, .tooltip = "Configure"})) {
                    s_editingMetric = m;
                    s_isNewCustomMetric = false;
                    s_metricEditModalOpen = true;
                }
                if (item.Action({.icon = Icon::Trash, .tooltip = "Remove from method", .variant = UiVariant::Danger})) {
                    removeIndex = static_cast<int>(i);
                }

                if (!m.formulaExpr.empty()) {
                    item.Formula(m.symbol, m.formulaExpr);
                }
            }
            Spacer(4);
        }

        if (removeIndex >= 0) {
            s_activeMethod.erase(s_activeMethod.begin() + removeIndex);
        }
    }
}

void RenderProcessingTab() {
    InitProcessingPreset();

    ColumnLayout flow;

    // Left Column (50%): Catalog
    if (auto leftCol = flow.Col(Col::Half())) {
        RenderCatalogCard(s_activeMethod);
    }

    // Right Column (50%): Method
    if (auto rightCol = flow.Col(Col::Half())) {
        RenderMethodCard(s_activeMethod);
    }

    // Clear Method Confirmation Modal
    if (Modal clearModal(s_clearMethodModalOpen, {.title = "Clear Method?", .width = 460.0f, .height = 200.0f}); clearModal) {
        if (auto content = clearModal.Content()) {
            Text("Delete all algorithms and formulas from the project method?");
            Text("Test metrics will be recalculated.");
        }
        if (auto footer = clearModal.Footer()) {
            if (footer.Actions({.confirmLabel = "Confirm", .cancelLabel = "Cancel",
                                .variant = UiVariant::Danger, .icon = Icon::None})) {
                s_activeMethod.clear();
                s_clearMethodModalOpen = false;
            }
        }
    }

    // Metric Edit Modal
    if (Modal editModal(s_metricEditModalOpen, {.title = s_isNewCustomMetric ? "New Custom Metric" : "Configure Metric",
                                               .width = 560.0f, .height = 420.0f}); editModal) {
        if (auto content = editModal.Content()) {
            if (Card card({.title = "Metric Identity & Parameters", .key = "EditModalCard"}); card) {
                card.Text(s_editingMetric.name,        {.label = "Metric Name", .col = Col::Full()});
                card.Text(s_editingMetric.symbol,      {.label = "Symbol",      .col = Col::Half()});
                card.Text(s_editingMetric.unit,        {.label = "Unit",        .col = Col::Half()});
                card.Text(s_editingMetric.formulaExpr, {.label = "Calculation Expression / Formula", .col = Col::Full()});
            }
        }
        if (auto footer = editModal.Footer()) {
            if (footer.Actions({.confirmLabel = "Done"})) {
                if (s_isNewCustomMetric) {
                    s_activeMethod.push_back(s_editingMetric);
                } else {
                    for (auto& m : s_activeMethod) {
                        if (m.metricId == s_editingMetric.metricId) {
                            m = s_editingMetric;
                            break;
                        }
                    }
                }
                s_metricEditModalOpen = false;
            }
        }
    }
}

// ============================================================================
// TAB 3: Section Geometry (Parametric Engineering Section Calculator)
// ============================================================================
enum class SectionProfile : uint8_t {
    Rectangle = 0,
    Round,
    Tube,
    IBeam,
    Count
};

static const char* const kSectionProfileLabels[] = {
    "Rectangle (b x h)",
    "Round Bar (Ø d)",
    "Hollow Tube (Ø D x t)",
    "Structural I-Beam"
};

enum class SectionMaterial : uint8_t {
    Steel = 0,
    Aluminum,
    Titanium,
    Brass,
    CarbonFiber,
    Count
};

static const char* const kSectionMaterialLabels[] = {
    "Structural Steel (7.85 g/cm³)",
    "Aluminum 6061-T6 (2.70 g/cm³)",
    "Titanium Gr5 (4.43 g/cm³)",
    "Brass (8.50 g/cm³)",
    "Carbon Fiber (1.60 g/cm³)"
};

static const float kSectionMaterialDensities[] = {
    7.85f, 2.70f, 4.43f, 8.50f, 1.60f
};

inline constexpr double kPi = 3.14159265358979323846;

struct SectionGeometryState {
    SectionProfile  profile = SectionProfile::Rectangle;
    SectionMaterial material = SectionMaterial::Steel;

    float width     = 40.0f;  // b (mm)
    float height    = 60.0f;  // h (mm)
    float diameter  = 30.0f;  // d (mm)
    float outerDia  = 50.0f;  // D (mm)
    float wallThick = 4.0f;   // t (mm)
    float tf        = 6.0f;   // flange thickness (mm)
    float tw        = 4.0f;   // web thickness (mm)
    float lengthM   = 2.5f;   // beam span length (m)

    double areaMm2      = 0.0;
    double perimeterMm  = 0.0;
    double linearMassKg = 0.0; // kg/m
    double totalMassKg  = 0.0; // kg
    double ixCm4        = 0.0; // cm⁴
    double iyCm4        = 0.0; // cm⁴
    double wxCm3        = 0.0; // cm³
    double wyCm3        = 0.0; // cm³
    double rxMm         = 0.0; // mm
    double ryMm         = 0.0; // mm

    void Recalculate() {
        const double rho = static_cast<double>(kSectionMaterialDensities[static_cast<int>(material)]);

        switch (profile) {
            case SectionProfile::Rectangle: {
                const double b = std::max(0.1f, width);
                const double h = std::max(0.1f, height);
                areaMm2     = b * h;
                perimeterMm = 2.0 * (b + h);
                const double ixMm4 = (b * h * h * h) / 12.0;
                const double iyMm4 = (h * b * b * b) / 12.0;
                ixCm4 = ixMm4 * 1e-4;
                iyCm4 = iyMm4 * 1e-4;
                wxCm3 = ((b * h * h) / 6.0) * 1e-3;
                wyCm3 = ((h * b * b) / 6.0) * 1e-3;
                rxMm  = h / std::sqrt(12.0);
                ryMm  = b / std::sqrt(12.0);
                break;
            }
            case SectionProfile::Round: {
                const double d = std::max(0.1f, diameter);
                areaMm2     = (kPi * d * d) / 4.0;
                perimeterMm = kPi * d;
                const double iMm4 = (kPi * d * d * d * d) / 64.0;
                ixCm4 = iyCm4 = iMm4 * 1e-4;
                wxCm3 = wyCm3 = ((kPi * d * d * d) / 32.0) * 1e-3;
                rxMm  = ryMm  = d / 4.0;
                break;
            }
            case SectionProfile::Tube: {
                const double D = std::max(0.2f, outerDia);
                const double t = std::clamp(static_cast<double>(wallThick), 0.05, D * 0.49);
                const double d = D - 2.0 * t;
                areaMm2     = (kPi * (D * D - d * d)) / 4.0;
                perimeterMm = kPi * D;
                const double iMm4 = (kPi * (D * D * D * D - d * d * d * d)) / 64.0;
                ixCm4 = iyCm4 = iMm4 * 1e-4;
                wxCm3 = wyCm3 = ((kPi * (D * D * D * D - d * d * d * d)) / (32.0 * D)) * 1e-3;
                rxMm  = ryMm  = std::sqrt(D * D + d * d) / 4.0;
                break;
            }
            case SectionProfile::IBeam: {
                const double b = std::max(1.0f, width);
                const double h = std::max(2.0f, height);
                const double f = std::clamp(static_cast<double>(tf), 0.5, h * 0.45);
                const double w = std::clamp(static_cast<double>(tw), 0.5, b * 0.9);
                const double hInner = h - 2.0 * f;
                areaMm2     = 2.0 * b * f + hInner * w;
                perimeterMm = 4.0 * b + 2.0 * h - 2.0 * w;
                const double ixMm4 = (b * h * h * h - (b - w) * (hInner * hInner * hInner)) / 12.0;
                const double iyMm4 = (2.0 * f * b * b * b + hInner * w * w * w) / 12.0;
                ixCm4 = ixMm4 * 1e-4;
                iyCm4 = iyMm4 * 1e-4;
                wxCm3 = (2.0 * ixMm4 / h) * 1e-3;
                wyCm3 = (2.0 * iyMm4 / b) * 1e-3;
                rxMm  = std::sqrt(std::max(0.0, ixMm4 / areaMm2));
                ryMm  = std::sqrt(std::max(0.0, iyMm4 / areaMm2));
                break;
            }
            default:
                break;
        }

        linearMassKg = areaMm2 * rho * 1e-3;
        totalMassKg  = linearMassKg * std::max(0.01f, lengthM);
    }
};

static SectionGeometryState s_sectionState;
static bool s_sectionStateInitialized = false;

void RenderSectionGeometryTab() {
    if (!s_sectionStateInitialized) {
        s_sectionStateInitialized = true;
        s_sectionState.Recalculate();
    }

    const auto onEdit = [&](bool edited) {
        if (edited) {
            s_sectionState.Recalculate();
        }
    };

    ColumnLayout flow;

    // Left Column (50%): Geometric Inputs & Material Specs
    if (auto leftCol = flow.Col(Col::Half())) {
        if (Card card({.title = "Profile Geometry & Dimensions", .key = "GeoDimCard"}); card) {
            onEdit(card.Combo(s_sectionState.profile, kSectionProfileLabels, {.label = "Section Profile", .col = Col::Full()}));

            switch (s_sectionState.profile) {
                case SectionProfile::Rectangle:
                    onEdit(card.Float(s_sectionState.width,  {.label = "Width (b)",  .unit = "mm", .format = "%.1f", .col = Col::Half()}));
                    onEdit(card.Float(s_sectionState.height, {.label = "Height (h)", .unit = "mm", .format = "%.1f", .col = Col::Half()}));
                    break;

                case SectionProfile::Round:
                    onEdit(card.Float(s_sectionState.diameter, {.label = "Outer Diameter (d)", .unit = "mm", .format = "%.1f", .col = Col::Full()}));
                    break;

                case SectionProfile::Tube:
                    onEdit(card.Float(s_sectionState.outerDia,  {.label = "Outer Diameter (D)", .unit = "mm", .format = "%.1f", .col = Col::Half()}));
                    onEdit(card.Float(s_sectionState.wallThick, {.label = "Wall Thickness (t)", .unit = "mm", .format = "%.1f", .col = Col::Half()}));
                    break;

                case SectionProfile::IBeam:
                    onEdit(card.Float(s_sectionState.width,     {.label = "Flange Width (b)",     .unit = "mm", .format = "%.1f", .col = Col::Half()}));
                    onEdit(card.Float(s_sectionState.height,    {.label = "Total Height (h)",     .unit = "mm", .format = "%.1f", .col = Col::Half()}));
                    onEdit(card.Float(s_sectionState.tf,        {.label = "Flange Thickness (tf)",.unit = "mm", .format = "%.1f", .col = Col::Half()}));
                    onEdit(card.Float(s_sectionState.tw,        {.label = "Web Thickness (tw)",   .unit = "mm", .format = "%.1f", .col = Col::Half()}));
                    break;

                default:
                    break;
            }
        }

        Spacer(12);

        if (Card card({.title = "Material & Beam Span", .key = "GeoMatCard"}); card) {
            onEdit(card.Combo(s_sectionState.material, kSectionMaterialLabels, {.label = "Material Alloy", .col = Col::Full()}));
            onEdit(card.Float(s_sectionState.lengthM, {.label = "Beam Span Length (L)", .unit = "m", .format = "%.2f", .col = Col::Full()}));
        }
    }

    // Right Column (50%): Hero Readouts and Engineering Moduli
    if (auto rightCol = flow.Col(Col::Half())) {
        if (Card card({.title = "Primary Cross-Section Metrics", .key = "GeoHeroCard"}); card) {
            // Large Hero Readout for Area
            card.Value(s_sectionState.areaMm2, {.label = "Cross-Section Area (A)", .unit = "mm²", .format = "%.1f",
                                                .variant = UiVariant::Primary, .col = Col::Full(), .size = UiSize::Large});

            // Medium Value Displays for Mass
            card.Value(s_sectionState.linearMassKg, {.label = "Linear Mass", .unit = "kg/m", .format = "%.2f",
                                                    .col = Col::Half(), .size = UiSize::Medium});
            card.Value(s_sectionState.totalMassKg,  {.label = "Total Beam Mass", .unit = "kg", .format = "%.2f",
                                                    .col = Col::Half(), .size = UiSize::Medium});
        }

        Spacer(12);

        if (Card card({.title = "Structural & Bending Properties", .key = "GeoPropsCard"}); card) {
            card.Value(s_sectionState.ixCm4, {.label = "Moment of Inertia (Ix)", .unit = "cm⁴", .format = "%.2f",
                                             .col = Col::Half(), .size = UiSize::Small});
            card.Value(s_sectionState.iyCm4, {.label = "Moment of Inertia (Iy)", .unit = "cm⁴", .format = "%.2f",
                                             .col = Col::Half(), .size = UiSize::Small});

            card.Value(s_sectionState.wxCm3, {.label = "Section Modulus (Wx)", .unit = "cm³", .format = "%.2f",
                                             .col = Col::Half(), .size = UiSize::Small});
            card.Value(s_sectionState.wyCm3, {.label = "Section Modulus (Wy)", .unit = "cm³", .format = "%.2f",
                                             .col = Col::Half(), .size = UiSize::Small});

            card.Value(s_sectionState.rxMm,  {.label = "Radius of Gyration (rx)", .unit = "mm", .format = "%.1f",
                                             .col = Col::Half(), .size = UiSize::Small});
            card.Value(s_sectionState.ryMm,  {.label = "Radius of Gyration (ry)", .unit = "mm", .format = "%.1f",
                                             .col = Col::Half(), .size = UiSize::Small});

            card.Value(s_sectionState.perimeterMm, {.label = "Outer Perimeter (P)", .unit = "mm", .format = "%.1f",
                                                   .col = Col::Full(), .size = UiSize::Small});
        }
    }
}

}  // namespace

// ============================================================================
// Main Cards Page Entry Point
// ============================================================================
void RenderCardsPage() {
    TabBar(kTabLabels, s_activeTab);
    Spacer(10);

    switch (static_cast<CardsTab>(s_activeTab)) {
        case CardsTab::Archive:
            RenderArchiveTab();
            break;
        case CardsTab::Processing:
            RenderProcessingTab();
            break;
        case CardsTab::Geometry:
            RenderSectionGeometryTab();
            break;
    }
}

}  // namespace OmniKitShowcase
