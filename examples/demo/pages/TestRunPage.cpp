#include "TestRunPage.hpp"
#include "omnikit.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace OmniKitShowcase {

namespace {

// ----------------------------------------------------------------------------
// Демо-сигналы: абстрактная телеметрия с шумом, генерируется на лету
// ----------------------------------------------------------------------------
struct Channel {
    const char* name;
    const char* unit;
    double (*sample)(double t, double noise);
};

constexpr double kWindowS = 30.0;   // длина окна графика; по его концу график начинается заново

const Channel kChannels[] = {
    {"Throughput", "MB/s", [](double t, double n) {
         return std::max(6.0, 68.0 + 22.0 * std::sin(t * 1.8) + 12.0 * std::cos(t * 3.4) + 14.0 * std::sin(t * 0.4) + n * 2.0);
     }},
    {"Latency", "ms", [](double t, double n) {
         return std::max(1.5, 12.0 + 4.5 * std::cos(t * 1.5) + 2.0 * std::sin(t * 4.2) + n * 0.4);
     }},
    {"Memory", "MB", [](double t, double n) { return 512.0 + 50.0 * std::sin(t * 0.8) + n * 3.0; }},
    {"Packets", "k/s", [](double t, double n) {
         return std::max(8.0, (68.0 + 22.0 * std::sin(t * 1.8) + 12.0 * std::cos(t * 3.4)) * 1.25 + n * 2.0);
     }},
};
constexpr int kChannelCount = static_cast<int>(sizeof(kChannels) / sizeof(kChannels[0]));

Session s_session;
RealtimeChart s_chart;
constexpr int s_channel = 0;  // фиксированный демо-канал
bool s_boost = false;        // кнопка удержания: пока нажата, выборка идёт в 4 раза чаще
double s_sampleAccS = 0.0;   // накопленное время до следующей точки
std::mt19937 s_rng{12345};
bool s_chartReady = false;

void ApplyChannel() {
    const Channel& ch = kChannels[s_channel];
    s_chart.Clear();
    s_chart.SetXAxis("Elapsed Time", "s");
    s_chart.SetYAxis(ch.name, ch.unit);
    s_chart.SetLineThickness(2.2f);
    s_chart.SetHeadMarker(true);
    s_session.runTimeS = 0.0;
    s_session.samples = 0;
    s_session.value = 0.0;
    s_sampleAccS = 0.0;
}

void EnsureChart() {
    if (!s_chartReady) {
        s_chartReady = true;
        ApplyChannel();
    }
}

// ----------------------------------------------------------------------------
// Вкладка «Console»: график реального времени + боковая панель управления
// ----------------------------------------------------------------------------
void RenderConsoleTab() {
    EnsureChart();
    Session& s = s_session;

    SplitView split({.sideWidth = 280.0f});

    // Левая область: живой график
    if (auto main = split.Main({.cardBackground = true})) {
        s_chart.Render();
    }

    // Правая область: управление прогоном
    if (auto panel = split.Side()) {
        const bool running = s.run == RunState::Running;
        const bool idle = s.run == RunState::Idle;

        if (Card card({.title = "Run Control"}); card) {
            if (auto row = card.Row(UiSize::Large)) {
                if (card.Button({.label = "START", .variant = UiVariant::Success, .icon = Icon::Play,
                                 .tooltip = !s.connected ? "Connect the device first"
                                                         : (idle ? nullptr : "A run is already in progress"),
                                 .col = Col::Half(), .size = UiSize::Large, .disabled = !s.connected || !idle})) {
                    StartRun();
                }
                if (card.Button({.label = "STOP", .variant = UiVariant::Danger, .icon = Icon::Square,
                                 .tooltip = idle ? "No active run" : nullptr,
                                 .col = Col::Half(), .size = UiSize::Large, .disabled = idle})) {
                    StopRun();
                }
            }
            if (card.Button({.label = running ? "PAUSE" : "RESUME", .variant = UiVariant::Warning,
                             .icon = running ? Icon::Pause : Icon::Play,
                             .tooltip = idle ? "No active run" : nullptr,
                             .col = Col::Full(), .disabled = idle})) {
                PauseRun(running);
            }
        }
        Spacer();

        if (Card card({.title = "Readout"}); card) {
            card.Value(s.value, {.label = "Value", .unit = kChannels[s_channel].unit, .format = "%.1f",
                                 .col = Col::Half(), .size = UiSize::Large});
            card.Value(s.runTimeS, {.label = "Elapsed", .unit = "s", .format = "%.1f",
                                    .col = Col::Half(), .size = UiSize::Large});
        }
        Spacer();

        if (Card card({.title = "Sampling Rate"}); card) {
            static const std::vector<float> rates = {10.0f, 20.0f, 50.0f, 100.0f, 200.0f};
            card.PresetGrid(s.sampleRateHz, rates, {.unit = "Hz", .col = Col::Full(), .columns = 5});
        }
        Spacer();

        if (Card card({.title = "Manual Boost"}); card) {
            s_boost = card.HoldButton({.label = "Hold to Boost", .variant = UiVariant::Info, .icon = Icon::Play,
                                       .tooltip = running ? "Samples 4x faster while held" : "Start a run to enable boost",
                                       .col = Col::Full(), .disabled = !running});
        }
    }
}

}  // namespace

Session& GetSession() { return s_session; }

void StartRun() {
    if (!s_session.connected || s_session.run != RunState::Idle) return;
    EnsureChart();
    ApplyChannel();
    s_session.run = RunState::Running;
}

void StopRun() {
    s_session.run = RunState::Idle;
    s_boost = false;
}

void PauseRun(bool pause) {
    if (s_session.run == RunState::Idle) return;
    s_session.run = pause ? RunState::Paused : RunState::Running;
}

void UpdateStream(double dtS) {
    if (s_session.run != RunState::Running) return;
    EnsureChart();

    const Channel& ch = kChannels[s_channel];
    s_session.runTimeS += dtS;
    s_sampleAccS += dtS;

    const double rate = static_cast<double>(s_session.sampleRateHz) * (s_boost ? 4.0 : 1.0);
    const double period = 1.0 / std::max(1.0, rate);
    std::normal_distribution<double> noise(0.0, 1.0);

    // Не больше 100 точек за кадр: после долгой паузы кадра не копим бесконечный хвост
    for (int guard = 0; s_sampleAccS >= period && guard < 100; ++guard) {
        s_sampleAccS -= period;
        const double t = s_session.runTimeS - s_sampleAccS;
        const double v = ch.sample(t, noise(s_rng));
        s_chart.AppendPoint(t, v);
        s_session.value = v;
        ++s_session.samples;
    }
    if (s_sampleAccS > period * 100.0) s_sampleAccS = 0.0;

    // Конец окна: график начинается заново, прогон продолжается
    if (s_session.runTimeS > kWindowS) {
        s_chart.Clear();
        s_session.runTimeS = 0.0;
        s_sampleAccS = 0.0;
    }
}

void RenderTestRunPage() {
    // Пока единственное содержимое — консоль; TabBar вернём, когда появятся остальные вкладки
    RenderConsoleTab();
}

}  // namespace OmniKitShowcase
