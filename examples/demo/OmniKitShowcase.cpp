#include "OmniKitShowcase.hpp"
#include "omnikit.hpp"
#include "core/TimeUtil.hpp"
#include "pages/TestRunPage.hpp"
#include "pages/CalcAreaPage.hpp"
#include "pages/ImageViewerPage.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

// Демо построено как каркас прикладного приложения: Header, Sidebar, Content, StatusBar.
// Страницы лежат в pages/ и показывают возможности библиотеки на абстрактных данных.
namespace OmniKitShowcase {

namespace {

// Экраны навигации (пункты верхней части сайдбара)
enum class NavScreen {
    TestRun = 0, // Прогон: вкладки с пультом и графиком
    CalcArea,    // Расчет площади сечения образца (CalcAreaPage)
    Image,       // Просмотр изображений и гистограммы (ImageViewerPage)
};

// Узлы дерева сайдбара: проект → серия → прогон
struct RunNode {
    std::string id;
    std::string title;
    std::string timestamp;
    UiVariant   status;
};

struct SeriesNode {
    std::string          title;
    bool                 expanded;
    std::vector<RunNode> runs;
};

struct ProjectNode {
    std::string             title;
    bool                    expanded;
    std::vector<SeriesNode> series;
};

bool s_initialized = false;
NavScreen s_screen = NavScreen::TestRun;
std::string s_selectedRun;
std::vector<ProjectNode> s_projects;

bool s_scaleMenuOpen = false;
bool s_countMenuOpen = false;
int s_indicatorsCount = 4;
std::vector<std::unique_ptr<Indicator>> s_indicators;

EditableLabel s_demoEditableLabel;
std::string   s_demoLabelValue = "Experiment #42";

auto s_lastFrame = std::chrono::steady_clock::now();

std::string FormatCurrentClock() {
    auto nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm localTm{};
    LocalTime(nowTime, localTm);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", localTm.tm_hour, localTm.tm_min, localTm.tm_sec);
    return std::string(buf);
}

DeviceState DeviceStateOf(const Session& s) {
    if (!s.connected) return DeviceState::Disconnected;
    switch (s.run) {
        case RunState::Running: return DeviceState::Running;
        case RunState::Paused:  return DeviceState::Paused;
        case RunState::Idle:    break;
    }
    return DeviceState::Idle;
}

ImU32 StatusColor(UiVariant v) {
    const UiTheme& theme = UiTheme::Get();
    switch (v) {
        case UiVariant::Success: return theme.palette.success.solid;
        case UiVariant::Warning: return theme.palette.warning.solid;
        case UiVariant::Danger:  return theme.palette.danger.solid;
        case UiVariant::Primary: return theme.palette.accent;
        default:                 return theme.palette.textMuted;
    }
}

}  // namespace

void Init() {
    if (s_initialized) return;
    s_initialized = true;

    UiTheme& theme = UiTheme::Get();
    theme.LoadFonts();
    theme.SetMode(ThemeMode::Dark);

    s_projects = {
        {"Project Alpha", true, {
            {"Series A-1", true, {
                {"alpha_a1_r1", "Run 001", "10:14", UiVariant::Success},
                {"alpha_a1_r2", "Run 002", "10:18", UiVariant::Success},
                {"alpha_a1_r3", "Run 003", "10:22", UiVariant::Warning},
            }},
            {"Series A-2", false, {
                {"alpha_a2_r1", "Run 001", "11:02", UiVariant::Success},
                {"alpha_a2_r2", "Run 002", "11:09", UiVariant::Danger},
            }},
        }},
        {"Project Beta with a long descriptive name", false, {
            {"Series B-1", false, {
                {"beta_b1_r1", "Run 001", "12:30", UiVariant::Success},
            }},
        }},
    };
    s_selectedRun = "alpha_a1_r1";

    // Индикаторы заголовка: подпись, точность, единицы, число разрядов
    s_indicators.clear();
    s_indicators.push_back(std::make_unique<Indicator>(IndicatorOptions{.title = "VALUE", .precision = 1, .unit = "", .digits = 5}));
    s_indicators.push_back(std::make_unique<Indicator>(IndicatorOptions{.title = "ELAPSED", .precision = 1, .unit = "s", .digits = 4}));
    s_indicators.push_back(std::make_unique<Indicator>(IndicatorOptions{.title = "RATE", .value = 50.0, .precision = 0, .unit = "Hz", .digits = 3}));
    s_indicators.push_back(std::make_unique<Indicator>(IndicatorOptions{.title = "SAMPLES", .precision = 0, .digits = 5}));

    // Для автоматических проверок: сразу запускаем прогон
    if (const char* autostart = std::getenv("OMNIKIT_DEMO_AUTOSTART"); autostart && autostart[0] == '1') {
        StartRun();
    }
}

namespace {

// ----------------------------------------------------------------------------
// 1. Header: три зоны (слева — инструменты и статус устройства, по центру —
//    индикаторы, справа — настройка индикаторов)
// ----------------------------------------------------------------------------
void RenderHeader() {
    Session& session = GetSession();

    if (auto header = Header()) {
        const UiTheme& theme = UiTheme::Get();

        if (auto left = header.Left()) {
            // Масштаб интерфейса
            if (ToolButton({.icon = Icon::Aa, .size = UiSize::Medium, .tooltip = "Interface scale"})) {
                s_scaleMenuOpen = true;
            }
            if (ContextMenu menu(s_scaleMenuOpen); menu) {
                menu.Header("INTERFACE SCALE");
                menu.Separator();
                const float scales[] = {1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
                for (float s : scales) {
                    const bool isCurrent = std::abs(theme.GetScale() - s) < 0.05f;
                    if (menu.Item(Format("%.0f%%", s * 100.0f), {.selected = isCurrent})) {
                        UiTheme::Get().SetScale(s);
                    }
                }
            }

            SameLine(6);

            // Тема: солнце / луна
            const bool isDark = (theme.mode == ThemeMode::Dark);
            if (ToolButton({.icon = isDark ? Icon::Moon : Icon::Sun, .size = UiSize::Medium,
                            .tooltip = isDark ? "Dark theme - click for light" : "Light theme - click for dark",
                            .key = "theme"})) {
                UiTheme::Get().SetMode(isDark ? ThemeMode::Light : ThemeMode::Dark);
            }

            SameLine(10);

            // Статус устройства: подключение и питание
            const char* statusText = !session.connected ? "DISCONNECTED"
                                   : session.run == RunState::Running ? "RUNNING"
                                   : session.run == RunState::Paused  ? "PAUSED" : "READY";
            if (DeviceStatus("Device 01", DeviceStateOf(session),
                             {.status = statusText, .titleCandidates = {"Device 01"}})) {
                if (session.connected) {
                    StopRun();
                    session.connected = false;
                } else {
                    session.connected = true;
                }
            }
        }

        if (auto right = header.Right()) {
            if (ToolButton({.icon = Icon::List, .size = UiSize::Medium,
                            .tooltip = Format("Active indicators (%d)", s_indicatorsCount).c_str()})) {
                s_countMenuOpen = true;
            }
            if (ContextMenu menu(s_countMenuOpen); menu) {
                menu.Header("DIGITAL INDICATORS");
                menu.Separator();
                for (int n = 2; n <= 4; ++n) {
                    if (menu.Item(Format("%d indicators", n), {.selected = s_indicatorsCount == n})) {
                        s_indicatorsCount = n;
                    }
                }
            }
        }

        if (auto center = header.Center()) {
            s_indicators[0]->SetValue(session.value);
            s_indicators[1]->SetValue(session.runTimeS);
            s_indicators[2]->SetValue(static_cast<double>(session.sampleRateHz));
            s_indicators[3]->SetValue(static_cast<double>(session.samples));

            if (Carousel carousel({.widthPx = center.Width(), .heightPx = center.Height()}); carousel) {
                for (size_t i = 0; i < s_indicators.size() && i < static_cast<size_t>(s_indicatorsCount); ++i) {
                    if (i > 0) SameLine();
                    s_indicators[i]->Render();
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// 2. Toolbar: панель инструментов под Header с редактируемой меткой
// ----------------------------------------------------------------------------
void RenderToolbar() {
    if (auto toolbar = Toolbar()) {
        if (auto left = toolbar.Left()) {
            s_demoEditableLabel.Render(s_demoLabelValue, {
                .widthPx = -1.0f,
                .role = TextRole::Body,
                .defaultValue = [] { return std::string("Experiment #42"); }
            });
        }
    }
}

// ----------------------------------------------------------------------------
// 3. Sidebar: навигация по экранам и дерево проектов
// ----------------------------------------------------------------------------
void RenderSidebar() {
    const float topPx = UiTheme::Get().HeaderHeight() + UiTheme::Get().ToolbarHeight();

    if (auto sidebar = Sidebar({.posYPx = topPx})) {
        sidebar.Item({.label = "Test Run", .icon = Icon::Gamepad}, NavScreen::TestRun, s_screen);
        sidebar.Item({.label = "Calc Area", .icon = Icon::Clipboard}, NavScreen::CalcArea, s_screen);
        sidebar.Item({.label = "Image", .icon = Icon::LineChart}, NavScreen::Image, s_screen);

        sidebar.Spacer();
        sidebar.Separator();
        sidebar.SectionTitle("PROJECTS");

        if (auto list = sidebar.ScrollList()) {
            for (ProjectNode& project : s_projects) {
                size_t runCount = 0;
                for (const SeriesNode& series : project.series) runCount += series.runs.size();

                if (list.TreeNode({.label = project.title, .count = Format("%zu", runCount), .level = 0,
                                   .expanded = project.expanded, .key = project.title.c_str()}) != SidebarTreeClick::None) {
                    project.expanded = !project.expanded;
                }
                if (!project.expanded) continue;

                for (SeriesNode& series : project.series) {
                    if (list.TreeNode({.label = series.title, .count = Format("%zu", series.runs.size()), .level = 1,
                                       .expanded = series.expanded, .key = series.title.c_str()}) != SidebarTreeClick::None) {
                        series.expanded = !series.expanded;
                    }
                    if (!series.expanded) continue;

                    for (const RunNode& run : series.runs) {
                        if (list.Entry({.label = run.title, .sublabel = run.timestamp,
                                        .statusColor = StatusColor(run.status),
                                        .selected = (s_selectedRun == run.id),
                                        .key = run.id.c_str(), .level = 2})) {
                            s_selectedRun = run.id;
                        }
                    }
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// 4. Content: страница выбранного экрана
// ----------------------------------------------------------------------------
void RenderContent() {
    const float topPx = UiTheme::Get().HeaderHeight() + UiTheme::Get().ToolbarHeight();

    if (auto content = ContentArea({.posYPx = topPx})) {
        switch (s_screen) {
            case NavScreen::TestRun:
                RenderTestRunPage();
                break;
            case NavScreen::CalcArea:
                RenderCalcAreaPage();
                break;
            case NavScreen::Image:
                RenderImageViewerPage();
                break;
        }
    }
}

// ----------------------------------------------------------------------------
// 5. StatusBar: связь, сообщение оператора, время
// ----------------------------------------------------------------------------
void RenderStatusBar() {
    const Session& session = GetSession();

    if (auto status = StatusBar()) {
        status.Text(session.connected ? "Device 01: connected" : "Device 01: disconnected",
                    {.variant = session.connected ? UiVariant::Success : UiVariant::Danger});
        status.Separator();

        const char* message = !session.connected ? "No connection to the device"
                            : session.run == RunState::Running ? "Run in progress"
                            : session.run == RunState::Paused  ? "Run paused by operator"
                                                               : "Ready to start";
        status.Text(message, {.variant = session.run == RunState::Paused ? UiVariant::Warning : UiVariant::Default});

        status.Separator();
        status.Text("OmniGUI " + FormatCurrentClock(), {.variant = UiVariant::Secondary});
    }
}

}  // namespace

void RenderUI(float main_scale) {
    (void)main_scale;
    if (!s_initialized) {
        Init();
    }

    // Время кадра для потока демо-данных
    const auto now = std::chrono::steady_clock::now();
    const double dt = std::min(0.25, std::chrono::duration<double>(now - s_lastFrame).count());
    s_lastFrame = now;
    UpdateStream(dt);

    RenderHeader();
    RenderToolbar();
    RenderSidebar();
    RenderContent();
    RenderStatusBar();
}

}  // namespace OmniKitShowcase
