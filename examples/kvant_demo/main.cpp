#include <SDL3/SDL.h>
#include <iostream>
#include <string>
#include <vector>
#include "kvant/Context.hpp"
#include "kvant/backend/ThorCanvas.hpp"
#include "kvant/core/FrameMetrics.hpp"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL3: " << SDL_GetError() << std::endl;
        return 1;
    }

    int winWidth = 1024;
    int winHeight = 720;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    const char* winTitle = "Kvant Vector GUI Demo (C++20 Immediate Mode + ThorVG)";

    if (!SDL_CreateWindowAndRenderer(winTitle, winWidth, winHeight,
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
                                     &window, &renderer)) {
        std::cerr << "Failed to create SDL window & renderer: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetRenderVSync(renderer, 1);
    SDL_StartTextInput(window);

    kvant::Context ui;
    kvant::ThorCanvas canvas;

    std::vector<uint32_t> pixelBuffer(winWidth * winHeight, 0);
    canvas.SetTarget(pixelBuffer.data(), winWidth, winHeight, winWidth);

    SDL_Texture* screenTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        winWidth,
        winHeight
    );

    int counter = 0;
    int activeTabIndex = 0;
    bool running = true;
    bool vsyncEnabled = true;
    bool reactiveMode = true;
    bool darkMode = true;
    bool showTelemetry = false;
    bool formMode = true;
    float speedDrop = 50.0f;
    std::string sampleId = "Specimen-2026-X";
    size_t baudIndex = 1;
    std::string profileMode = "Standard Performance";
    size_t heartbeatIndex = 2; // Default: 500 ms (2 Hz)
    const std::vector<int> heartbeatMsValues = {-1, 250, 500, 1000, 2000};
    const std::vector<std::string_view> heartbeatLabels = {
        "Heartbeat: Off",
        "Heartbeat: 4 Hz",
        "Heartbeat: 2 Hz",
        "Heartbeat: 1 Hz",
        "Heartbeat: 0.5 Hz"
    };
    bool enableSensors = true;
    float maxTravel = 150.0f;
    float maxVelocity = 25.0f;
    size_t sensorChannel = 0;
    bool hwEstop = true;
    size_t interlockMode = 1;
    float rampDown = 250.0f;
    float forceLimit = 85.0f;
    float yieldRatio = 0.2f;
    float pGain = 1.25f;
    float iGain = 0.05f;
    float dGain = 0.01f;
    bool diagLogging = true;
    kvant::TabBarStyle tabStyle = kvant::TabBarStyle::Underline;
    kvant::FrameMetrics metrics;

    ui.SetThemeMode(darkMode ? kvant::ThemeMode::Dark : kvant::ThemeMode::Light);
    ui.SetIdleHeartbeatMs(heartbeatMsValues[heartbeatIndex]);

    auto handleEvent = [&](const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                winWidth = event.window.data1;
                winHeight = event.window.data2;
                pixelBuffer.resize(winWidth * winHeight, 0);
                canvas.SetTarget(pixelBuffer.data(), winWidth, winHeight, winWidth);
                if (screenTexture) {
                    SDL_DestroyTexture(screenTexture);
                }
                screenTexture = SDL_CreateTexture(
                    renderer,
                    SDL_PIXELFORMAT_ABGR8888,
                    SDL_TEXTUREACCESS_STREAMING,
                    winWidth,
                    winHeight
                );
                ui.RequestRepaint(3);
                break;
            }
            case SDL_EVENT_MOUSE_MOTION:
                ui.OnMouseMove({event.motion.x, event.motion.y});
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                ui.OnMouseDown(event.button.button);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                ui.OnMouseUp(event.button.button);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                ui.OnMouseWheel(event.wheel.x, event.wheel.y);
                break;
            case SDL_EVENT_TEXT_INPUT:
                ui.OnTextInput(event.text.text);
                break;
            case SDL_EVENT_KEY_DOWN: {
                kvant::Key key = kvant::Key::None;
                switch (event.key.key) {
                    case SDLK_BACKSPACE: key = kvant::Key::Backspace; break;
                    case SDLK_DELETE:    key = kvant::Key::Delete; break;
                    case SDLK_RETURN:    key = kvant::Key::Enter; break;
                    case SDLK_ESCAPE:    key = kvant::Key::Escape; break;
                    case SDLK_TAB:       key = kvant::Key::Tab; break;
                    case SDLK_LEFT:      key = kvant::Key::Left; break;
                    case SDLK_RIGHT:     key = kvant::Key::Right; break;
                    case SDLK_UP:        key = kvant::Key::Up; break;
                    case SDLK_DOWN:      key = kvant::Key::Down; break;
                    case SDLK_HOME:      key = kvant::Key::Home; break;
                    case SDLK_END:       key = kvant::Key::End; break;
                    default: break;
                }
                if (key != kvant::Key::None) {
                    ui.OnKeyDown(key);
                }
                break;
            }
            default:
                break;
        }
    };

    while (running) {
        if (reactiveMode && !ui.NeedsRepaint()) {
            const int timeoutMs = ui.GetRecommendedSleepTimeoutMs();
            SDL_Event event;
            if (SDL_WaitEventTimeout(&event, timeoutMs)) {
                handleEvent(event);
                while (SDL_PollEvent(&event)) {
                    handleEvent(event);
                }
            } else {
                // Timeout expired with no events.
                // If timeoutMs <= 0 (disabled periodic heartbeat / deep sleep), continue waiting.
                // If timeoutMs > 0, the periodic heartbeat fired: proceed to render 1 frame to refresh metrics.
                if (timeoutMs <= 0 && !ui.NeedsRepaint()) {
                    continue;
                }
            }
        } else {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                handleEvent(event);
            }
        }

        if (!running) {
            break;
        }

        const auto frameWorkStart = std::chrono::steady_clock::now();
        metrics.BeginFrame();

        // --- Begin Immediate Mode Kvant Frame ---
        ui.BeginFrame({static_cast<float>(winWidth), static_cast<float>(winHeight)});
        {
            if (auto mainCol = ui.Column()) {
                // Header Bar
                if (auto header = ui.Row({.align = kvant::Alignment::Center})) {
                    ui.Title("KVANT GUI", kvant::UiVariant::Info);
                    ui.Caption("| Autonomous Vector GUI Engine");
                    ui.Spacer(); // Push build badge to right
                    ui.Caption("C++20 Immediate + ThorVG");
                }

                // Engine HUD Toolbar
                if (auto tb = ui.Toolbar({.fixedHeight = 42.0f})) {
                    // Density Preset Toggle
                    const char* densityLabel = (ui.GetDensity() == kvant::Density::Standard) ? "Density: Standard" :
                                              ((ui.GetDensity() == kvant::Density::Compact) ? "Density: Compact" : "Density: Touch");
                    if (tb.ToolButton(densityLabel, {
                        .icon = kvant::Icon::Settings(),
                        .labelVariants = {"Density: Standard", "Density: Compact", "Density: Touch"},
                        .active = true
                    })) {
                        if (ui.GetDensity() == kvant::Density::Standard) {
                            ui.SetDensity(kvant::Density::Compact);
                        } else if (ui.GetDensity() == kvant::Density::Compact) {
                            ui.SetDensity(kvant::Density::Touch);
                        } else {
                            ui.SetDensity(kvant::Density::Standard);
                        }
                    }

                    tb.Separator();

                    // Theme Mode Toggle
                    if (tb.ToolButton(darkMode ? "Theme: Dark" : "Theme: Light", {
                        .icon = darkMode ? kvant::Icon::Eye() : kvant::Icon::EyeOff(),
                        .labelVariants = {"Theme: Dark", "Theme: Light"},
                        .active = darkMode
                    })) {
                        darkMode = !darkMode;
                        ui.SetThemeMode(darkMode ? kvant::ThemeMode::Dark : kvant::ThemeMode::Light);
                        ui.InvalidateAll();
                    }

                    tb.Separator();

                    // VSync Toggle
                    if (tb.ToolButton(vsyncEnabled ? "VSync: ON" : "VSync: OFF", {
                        .icon = kvant::Icon::Refresh(),
                        .variant = vsyncEnabled ? kvant::UiVariant::Success : kvant::UiVariant::Ghost,
                        .labelVariants = {"VSync: ON", "VSync: OFF"},
                        .active = vsyncEnabled
                    })) {
                        vsyncEnabled = !vsyncEnabled;
                        SDL_SetRenderVSync(renderer, vsyncEnabled ? 1 : 0);
                    }

                    // Reactive Mode Toggle
                    if (tb.ToolButton(reactiveMode ? "Reactive: ON" : "Reactive: OFF", {
                        .icon = reactiveMode ? kvant::Icon::Play() : kvant::Icon::Pause(),
                        .variant = reactiveMode ? kvant::UiVariant::Primary : kvant::UiVariant::Ghost,
                        .labelVariants = {"Reactive: ON", "Reactive: OFF"},
                        .active = reactiveMode
                    })) {
                        reactiveMode = !reactiveMode;
                        if (!reactiveMode) {
                            ui.InvalidateAll();
                        }
                    }

                    // Dirty Rect Toggle
                    const bool dirtyActive = ui.IsDirtyRectEnabled();
                    if (tb.ToolButton(dirtyActive ? "Dirty: ON" : "Dirty: OFF", {
                        .icon = kvant::Icon::Settings(),
                        .variant = dirtyActive ? kvant::UiVariant::Info : kvant::UiVariant::Ghost,
                        .labelVariants = {"Dirty: ON", "Dirty: OFF"},
                        .active = dirtyActive
                    })) {
                        const bool nextDirty = !dirtyActive;
                        ui.SetDirtyRectEnabled(nextDirty);
                        if (!nextDirty) {
                            ui.InvalidateAll();
                        }
                    }

                    // Idle Heartbeat Presets Cycle
                    if (tb.ToolButton(heartbeatLabels[heartbeatIndex], {
                        .icon = kvant::Icon::Plus(),
                        .labelVariants = heartbeatLabels,
                        .disabled = !reactiveMode
                    })) {
                        heartbeatIndex = (heartbeatIndex + 1) % heartbeatLabels.size();
                        ui.SetIdleHeartbeatMs(heartbeatMsValues[heartbeatIndex]);
                    }

                    tb.Spacer();

                    // TabBar Style Toggle
                    if (tb.ToolButton(tabStyle == kvant::TabBarStyle::Underline ? "Tabs: Underline" : "Tabs: Pill", {
                        .icon = kvant::Icon::Folder(),
                        .labelVariants = {"Tabs: Underline", "Tabs: Pill"}
                    })) {
                        tabStyle = (tabStyle == kvant::TabBarStyle::Underline)
                            ? kvant::TabBarStyle::Pill
                            : kvant::TabBarStyle::Underline;
                    }
                }

                // First-class Vector TabBar
                const std::vector<kvant::TabItem> navTabs = {
                    {.label = "Overview", .icon = kvant::Icon::Eye()},
                    {.label = "Flexbox Demo", .icon = kvant::Icon::Settings()},
                    {.label = "Industrial Dashboard", .icon = kvant::Icon::Folder(), .badge = "Live"}
                };
                ui.TabBar(activeTabIndex, navTabs, {.style = tabStyle});

                if (activeTabIndex == 2) {
                    // Industrial Dashboard: responsive 2-column layout inside a dedicated ScrollArea
                    if (auto scroll = ui.ScrollArea()) {
                        if (auto dashboardGrid = ui.CellGrid()) {
                            // Left Column: wraps to full width when narrow
                            if (auto colLeft = dashboardGrid.Cell6()) {
                                if (auto card1 = ui.Card()) {
                                    ui.Heading("Primary Stop Conditions", "Sensory tripwire thresholds & limits");
                                    if (auto grid1 = ui.CellGrid()) {
                                        if (auto c = grid1.Cell12()) {
                                            ui.Switch("Enable Sensor Drop Limits", &enableSensors, {
                                                .description = "Immediate stop on threshold breach"
                                            });
                                        }
                                        if (auto c = grid1.Cell6()) {
                                            ui.InputNumber("Max Travel", maxTravel, "mm", {.format = "%.1f"});
                                        }
                                        if (auto c = grid1.Cell6()) {
                                            ui.InputNumber("Max Velocity", maxVelocity, "mm/s", {.format = "%.1f"});
                                        }
                                        if (auto c = grid1.Cell12()) {
                                            ui.Select("Primary Sensor Channel", sensorChannel, {
                                                "Channel A (Load Cell 50kN)",
                                                "Channel B (Optical Extensometer)",
                                                "Channel C (Laser Interferometer)"
                                            }, {
                                                .description = "Active physical transducer bus",
                                                .labelPlacement = kvant::Placement::Stacked
                                            });
                                        }
                                    }
                                }

                                if (auto card2 = ui.Card()) {
                                    ui.Heading("Emergency Interlocks", "Hardware line supervision & failsafes", {.variant = kvant::UiVariant::Danger});
                                    if (auto grid2 = ui.CellGrid()) {
                                        if (auto c = grid2.Cell12()) {
                                            ui.Switch("Hardware E-Stop Line", &hwEstop, {
                                                .description = "Continuous 24V loop monitoring",
                                                .variant = kvant::UiVariant::Danger
                                            });
                                        }
                                        if (auto c = grid2.Cell6()) {
                                            ui.Select("Interlock Action", interlockMode, {
                                                "Immediate Cutoff",
                                                "Controlled Ramp-down",
                                                "Hold Position"
                                            });
                                        }
                                        if (auto c = grid2.Cell6()) {
                                            ui.InputNumber("Ramp Duration", rampDown, "ms", {.format = "%.0f"});
                                        }
                                    }
                                }
                            }

                            // Right Column: wraps to full width when narrow
                            if (auto colRight = dashboardGrid.Cell6()) {
                                if (auto card3 = ui.Card()) {
                                    ui.Heading("Secondary Criteria & PID", "Dynamic closed-loop feedback parameters");
                                    if (auto grid3 = ui.CellGrid()) {
                                        if (auto c = grid3.Cell6()) {
                                            ui.InputNumber("Force Drop Limit", forceLimit, "%", {.format = "%.1f"});
                                        }
                                        if (auto c = grid3.Cell6()) {
                                            ui.InputNumber("Yield Ratio", yieldRatio, "%", {.format = "%.2f"});
                                        }
                                        if (auto c = grid3.Cell4()) {
                                            ui.InputNumber("P-Gain", pGain, {}, {.format = "%.2f"});
                                        }
                                        if (auto c = grid3.Cell4()) {
                                            ui.InputNumber("I-Gain", iGain, {}, {.format = "%.3f"});
                                        }
                                        if (auto c = grid3.Cell4()) {
                                            ui.InputNumber("D-Gain", dGain, {}, {.format = "%.3f"});
                                        }
                                        if (auto c = grid3.Cell12()) {
                                            ui.Switch("Log High-Speed Telemetry", &diagLogging, {
                                                .description = "Record 10 kHz buffer to internal flash"
                                            });
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    // Content Panel Card for Tabs 0 and 1
                    if (auto card = ui.Card()) {
                        if (activeTabIndex == 0) {
                            // Overview Tab: Interactive Counter
                            ui.Heading("Interactive Reactive State", "Pure immediate mode with zero external widget IDs and instant response.");

                            if (auto actionRow = ui.Wrap({.align = kvant::Alignment::Center})) {
                                if (ui.Button("Increment (+1)", {.icon = kvant::Icon::Plus(), .variant = kvant::UiVariant::Success})) {
                                    ++counter;
                                }
                                if (ui.Button("Decrement (-1)", {.icon = kvant::Icon::Minus(), .variant = kvant::UiVariant::Warning})) {
                                    --counter;
                                }
                                if (ui.Button("Reset", {.icon = kvant::Icon::Refresh(), .variant = kvant::UiVariant::Danger})) {
                                    counter = 0;
                                }

                                ui.Text(ui.Format("Value: {}", counter), {.role = kvant::TextRole::HeadlineMedium, .variant = kvant::UiVariant::Info});
                            }

                            if (auto togglesRow = ui.Wrap({.gap = kvant::UiSize::Lg, .align = kvant::Alignment::Center})) {
                                ui.Checkbox("Show Telemetry", &showTelemetry, {.variant = kvant::UiVariant::Info});
                                if (ui.Switch("Form Mode", &formMode, {
                                    .description = "Toggle stacked vs default label layout",
                                    .labelPlacement = kvant::Placement::Stacked
                                    }).changed) {
                                    ui.InvalidateAll();
                                }
                                ui.Checkbox("Auto-Scale", true, {.disabled = true});
                            }

                            if (showTelemetry) {
                                ui.Heading({
                                    .title = "Telemetry Stream",
                                    .subtitle = "1000 samples/sec | Jitter < 0.2ms | Hardware loop nominal",
                                    .icon = kvant::Icon::Check(),
                                    .badge = "ACTIVE",
                                    .variant = kvant::UiVariant::Success
                                });
                            }

                            const kvant::Placement activePlacement = formMode ? kvant::Placement::Stacked : kvant::Placement::Leading;

                            if (auto fieldsGrid = ui.CellGrid()) {
                                if (auto c1 = fieldsGrid.Cell3()) {
                                    ui.InputField("Speed Drop dF/dt", speedDrop, {
                                        .description = "Automatic stop of testing on load drop",
                                        .suffix = "%",
                                        .format = "%.0f",
                                        .labelPlacement = activePlacement
                                    });
                                }
                                if (auto c2 = fieldsGrid.Cell3()) {
                                    ui.InputField("Sample Identifier", sampleId, {
                                        .description = "Unique test specimen code",
                                        .placeholder = "Enter sample code...",
                                        .labelPlacement = activePlacement
                                    });
                                }
                                if (auto c3 = fieldsGrid.Cell3()) {
                                    ui.Select("Baud Rate", baudIndex, {"9600 bps", "19200 bps", "38400 bps", "115200 bps"}, {
                                        .description = "Serial communication bus rate",
                                        .labelPlacement = activePlacement
                                    });
                                }
                                if (auto c4 = fieldsGrid.Cell3()) {
                                    ui.Select("Operation Profile", profileMode, {"Eco Saver", "Standard Performance", "Industrial High-Speed", "Extreme Calibration"}, {
                                        .description = "Active runtime sensor & compute profile",
                                        .labelPlacement = activePlacement
                                    });
                                }
                            }
                        } else {
                            // Flexbox Demo Tab: Equal Stretch & Alignment
                            ui.Heading("Dynamic 3-Way Equal Stretch Layout", "Each button automatically receives exactly 33.3% available width via stretch = true.");

                            if (auto stretchRow = ui.Row()) {
                                ui.Button("Col 1 (Stretch)", {.stretch = true});
                                ui.Button("Col 2 (Stretch)", {.stretch = true});
                                ui.Button("Col 3 (Stretch)", {.stretch = true});
                            }
                        }
                    }
                }

                // Vertical Spacer pushing the footer down (only for non-stretching tabs)
                if (activeTabIndex != 2) {
                    ui.Spacer();
                }

                // Sticky Footer Bar
                if (auto footer = ui.Row({.align = kvant::Alignment::Center})) {
                    ui.Caption(metrics.GetStatusString(), kvant::UiVariant::Muted);
                    ui.Spacer();
                    if (ui.Button("Exit", {.icon = kvant::Icon::Close(), .variant = kvant::UiVariant::Ghost})) {
                        running = false;
                    }
                }
            }
        }
        ui.EndFrame(canvas);
        metrics.MarkCpuDone();

        const bool shouldPresent = ui.HasDirtyDamage() || !reactiveMode;
        if (shouldPresent) {
            metrics.MarkGpuStart();
            if (ui.HasDirtyDamage()) {
                if (ui.IsFullRepaint()) {
                    SDL_UpdateTexture(screenTexture, nullptr, pixelBuffer.data(), winWidth * sizeof(uint32_t));
                } else {
                    const kvant::Rect r = ui.GetDirtyRect();
                    const SDL_Rect sdlR = {
                        static_cast<int>(r.x),
                        static_cast<int>(r.y),
                        static_cast<int>(r.w),
                        static_cast<int>(r.h)
                    };
                    const uint32_t* src = pixelBuffer.data() + sdlR.y * winWidth + sdlR.x;
                    SDL_UpdateTexture(screenTexture, &sdlR, src, winWidth * sizeof(uint32_t));
                }
            }
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, screenTexture, nullptr, nullptr);
            metrics.MarkGpuDone();

            SDL_RenderPresent(renderer);
            metrics.MarkFramePresented();
        }

        // Frame rate limiter and pacing:
        // Ensures smooth 60 FPS pacing and prevents unconstrained spinning when presenting is skipped or VSync is off
        const auto frameEnd = std::chrono::steady_clock::now();
        const double frameElapsedMs = std::chrono::duration<double, std::milli>(frameEnd - frameWorkStart).count();
        const double targetFrameMs = 1000.0 / 60.0; // 16.67 ms (60 FPS)
        if (frameElapsedMs < targetFrameMs) {
            const double remainingMs = targetFrameMs - frameElapsedMs;
            if (!reactiveMode && !vsyncEnabled) {
                SDL_Delay(static_cast<uint32_t>(remainingMs));
            } else if (!shouldPresent) {
                if (remainingMs >= 1.0) {
                    SDL_Delay(static_cast<uint32_t>(remainingMs));
                }
            }
        }

        metrics.EndFrame(60.0f);
    }

    if (screenTexture) {
        SDL_DestroyTexture(screenTexture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
