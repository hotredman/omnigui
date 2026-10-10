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
    bool toolPlay = false;
    bool toolPause = false;
    kvant::FrameMetrics metrics;

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
            if (auto mainCol = ui.Column({.gap = 16.0f, .padding = kvant::Padding::All(24.0f)})) {
                // Header Bar
                if (auto header = ui.Row({.gap = 12.0f, .align = kvant::Alignment::Center})) {
                    ui.Title("KVANT GUI", kvant::UiVariant::Info);
                    ui.Caption("| Autonomous Vector GUI Engine");
                    ui.Spacer(); // Push build badge to right
                    ui.Caption("C++20 Immediate + ThorVG");
                }

                // Toolbar
                if (auto tb = ui.Toolbar({.fixedHeight = 42.0f})) {
                    if (tb.ToolButton("Start", {.icon = kvant::Icon::Play(), .variant = kvant::UiVariant::Success, .active = toolPlay})) {
                        toolPlay = true;
                        toolPause = false;
                        enableSensors = true;
                    }
                    if (tb.ToolButton("Pause", {.icon = kvant::Icon::Pause(), .variant = kvant::UiVariant::Warning, .active = toolPause})) {
                        toolPause = true;
                        toolPlay = false;
                    }
                    if (tb.ToolButton("Stop", {.icon = kvant::Icon::Stop(), .variant = kvant::UiVariant::Danger})) {
                        toolPlay = false;
                        toolPause = false;
                        enableSensors = false;
                    }
                    tb.Separator();
                    if (tb.ToolButton(tabStyle == kvant::TabBarStyle::Underline ? "Tabs: Underline" : "Tabs: Pill",
                                      {.icon = kvant::Icon::Settings()})) {
                        tabStyle = (tabStyle == kvant::TabBarStyle::Underline)
                            ? kvant::TabBarStyle::Pill
                            : kvant::TabBarStyle::Underline;
                    }
                    tb.Spacer();
                    if (tb.ToolButton("Reset Defaults", {.icon = kvant::Icon::Refresh()})) {
                        activeTabIndex = 0;
                        enableSensors = true;
                        toolPlay = false;
                        toolPause = false;
                        maxTravel = 150.0f;
                        maxVelocity = 25.0f;
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
                        if (auto dashboardGrid = ui.Grid({.gap = 16.0f, .runGap = 16.0f})) {
                            // Left Column: wraps to full width if < 360px
                            if (auto colLeft = ui.Cell6({.minWidth = 360.0f, .gap = 16.0f})) {
                                if (auto card1 = ui.Card("Primary Stop Conditions", {.subtitle = "Sensory tripwire thresholds & limits"})) {
                                    if (auto c = ui.Cell12()) {
                                        ui.Switch("Enable Sensor Drop Limits", &enableSensors, {
                                            .description = "Immediate stop on threshold breach"
                                        });
                                    }
                                    if (auto c = ui.Cell6({.minWidth = 140.0f})) {
                                        ui.InputNumber("Max Travel", maxTravel, "mm", {.format = "%.1f"});
                                    }
                                    if (auto c = ui.Cell6({.minWidth = 140.0f})) {
                                        ui.InputNumber("Max Velocity", maxVelocity, "mm/s", {.format = "%.1f"});
                                    }
                                    if (auto c = ui.Cell12()) {
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

                                if (auto card2 = ui.Card("Emergency Interlocks", {.subtitle = "Hardware line supervision & failsafes"})) {
                                    if (auto c = ui.Cell12()) {
                                        ui.Switch("Hardware E-Stop Line", &hwEstop, {
                                            .description = "Continuous 24V loop monitoring",
                                            .variant = kvant::UiVariant::Danger
                                        });
                                    }
                                    if (auto c = ui.Cell6({.minWidth = 140.0f})) {
                                        ui.Select("Interlock Action", interlockMode, {
                                            "Immediate Cutoff",
                                            "Controlled Ramp-down",
                                            "Hold Position"
                                        });
                                    }
                                    if (auto c = ui.Cell6({.minWidth = 140.0f})) {
                                        ui.InputNumber("Ramp Duration", rampDown, "ms", {.format = "%.0f"});
                                    }
                                }
                            }

                            // Right Column: wraps to full width if < 360px
                            if (auto colRight = ui.Cell6({.minWidth = 360.0f, .gap = 16.0f})) {
                                if (auto card3 = ui.Card("Secondary Criteria & PID", {.subtitle = "Dynamic closed-loop feedback parameters"})) {
                                    if (auto c = ui.Cell6({.minWidth = 140.0f})) {
                                        ui.InputNumber("Force Drop Limit", forceLimit, "%", {.format = "%.1f"});
                                    }
                                    if (auto c = ui.Cell6({.minWidth = 140.0f})) {
                                        ui.InputNumber("Yield Ratio", yieldRatio, "%", {.format = "%.2f"});
                                    }
                                    if (auto c = ui.Cell4({.minWidth = 90.0f})) {
                                        ui.InputNumber("P-Gain", pGain, {}, {.format = "%.2f"});
                                    }
                                    if (auto c = ui.Cell4({.minWidth = 90.0f})) {
                                        ui.InputNumber("I-Gain", iGain, {}, {.format = "%.3f"});
                                    }
                                    if (auto c = ui.Cell4({.minWidth = 90.0f})) {
                                        ui.InputNumber("D-Gain", dGain, {}, {.format = "%.3f"});
                                    }
                                    if (auto c = ui.Cell12()) {
                                        ui.Switch("Log High-Speed Telemetry", &diagLogging, {
                                            .description = "Record 10 kHz buffer to internal flash"
                                        });
                                    }
                                }
                            }
                        }
                    }
                } else {
                    // Content Panel Card for Tabs 0 and 1
                    if (auto card = ui.Box({
                        .gap = 16.0f,
                        .padding = kvant::Padding::All(20.0f),
                        .backgroundColor = kvant::Color::Hex(0x1E293BFF), // Slate-800
                        .borderColor = kvant::Color::Hex(0x334155FF),     // Slate-700
                        .borderWidth = 1.0f,
                        .cornerRadius = 8.0f,
                        .clipChildren = true
                    })) {
                        if (activeTabIndex == 0) {
                            // Overview Tab: Interactive Counter
                            ui.Headline("Interactive Reactive State");
                            ui.Body("Pure immediate mode with zero external widget IDs and instant response.",
                                    kvant::UiVariant::Muted);

                            if (auto actionRow = ui.Wrap({.gap = 12.0f, .runGap = 10.0f, .align = kvant::Alignment::Center})) {
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

                            if (auto togglesRow = ui.Wrap({.gap = 20.0f, .runGap = 16.0f, .align = kvant::Alignment::Center})) {
                                if (ui.Switch("Dark Theme", &darkMode).changed) {
                                    ui.InvalidateAll();
                                }
                                if (ui.Switch("VSync (60 FPS)", &vsyncEnabled, {
                                    .description = "Locks rendering rate to display refresh to eliminate tearing",
                                    .variant = kvant::UiVariant::Success
                                }).changed) {
                                    SDL_SetRenderVSync(renderer, vsyncEnabled ? 1 : 0);
                                }
                                ui.Switch("Reactive Mode", &reactiveMode, {
                                    .description = "Zero-CPU idle: sleeps when inactive, wakes up on input events",
                                    .variant = kvant::UiVariant::Primary
                                });
                                bool dirtyRectEnabled = ui.IsDirtyRectEnabled();
                                if (ui.Switch("Dirty Rect", &dirtyRectEnabled, {
                                    .description = "Redraw & upload only changed damage regions instead of full 1080p frame",
                                    .variant = kvant::UiVariant::Success
                                }).changed) {
                                    ui.SetDirtyRectEnabled(dirtyRectEnabled);
                                    if (!dirtyRectEnabled) {
                                        ui.InvalidateAll();
                                    }
                                }
                                ui.Checkbox("Show Telemetry", &showTelemetry, {.variant = kvant::UiVariant::Info});
                                if (ui.Switch("Form Mode", &formMode, {
                                    .description = "Stacked label & description layout",
                                    .labelPlacement = kvant::Placement::Stacked
                                }).changed) {
                                    ui.InvalidateAll();
                                }
                                ui.Checkbox("Auto-Scale", true, {.disabled = true});
                            }

                            if (auto fieldsRow = ui.Wrap({.gap = 24.0f, .runGap = 16.0f})) {
                                if (auto f1 = ui.Box({.fixedWidth = 260.0f})) {
                                    ui.InputField("Speed Drop dF/dt", speedDrop, {
                                        .description = "Automatic stop of testing on load drop",
                                        .suffix = "%",
                                        .format = "%.0f"
                                    });
                                }
                                if (auto f2 = ui.Box({.fixedWidth = 260.0f})) {
                                    ui.InputField("Sample Identifier", sampleId, {
                                        .description = "Unique test specimen code",
                                        .placeholder = "Enter sample code..."
                                    });
                                }
                                if (auto f3 = ui.Box({.fixedWidth = 260.0f})) {
                                    ui.Select("Baud Rate", baudIndex, {"9600 bps", "19200 bps", "38400 bps", "115200 bps"}, {
                                        .description = "Serial communication bus rate",
                                        .labelPlacement = kvant::Placement::Stacked
                                    });
                                }
                                if (auto f4 = ui.Box({.fixedWidth = 260.0f})) {
                                    ui.Select("Operation Profile", profileMode, {"Eco Saver", "Standard Performance", "Industrial High-Speed", "Extreme Calibration"}, {
                                        .description = "Active runtime sensor & compute profile",
                                        .labelPlacement = kvant::Placement::Stacked
                                    });
                                }
                                if (auto fHeartbeat = ui.Box({.fixedWidth = 260.0f})) {
                                    if (ui.Select("Idle Heartbeat", heartbeatIndex, {
                                        "Off (Deep Sleep)",
                                        "250 ms (4 Hz)",
                                        "500 ms (2 Hz - Default)",
                                        "1000 ms (1 Hz)",
                                        "2000 ms (0.5 Hz)"
                                    }, {
                                        .description = "Telemetry refresh rate in idle mode",
                                        .labelPlacement = kvant::Placement::Stacked,
                                        .disabled = !reactiveMode
                                    }).changed) {
                                        ui.SetIdleHeartbeatMs(heartbeatMsValues[heartbeatIndex]);
                                    }
                                }
                            }
                        } else {
                            // Flexbox Demo Tab: Equal Stretch & Alignment
                            ui.Headline("Dynamic 3-Way Equal Stretch Layout");
                            ui.Body("Each button automatically receives exactly 33.3% available width.",
                                    kvant::UiVariant::Muted);

                            if (auto stretchRow = ui.Row({.gap = 12.0f})) {
                                if (auto col1 = ui.Box({.widthMode = kvant::SizeMode::Stretch})) {
                                    ui.Button("Col 1 (Stretch)");
                                }
                                if (auto col2 = ui.Box({.widthMode = kvant::SizeMode::Stretch})) {
                                    ui.Button("Col 2 (Stretch)");
                                }
                                if (auto col3 = ui.Box({.widthMode = kvant::SizeMode::Stretch})) {
                                    ui.Button("Col 3 (Stretch)");
                                }
                            }
                        }
                    }
                }

                // Vertical Spacer pushing the footer down (only for non-stretching tabs)
                if (activeTabIndex != 2) {
                    ui.Spacer();
                }

                // Sticky Footer Bar
                if (auto footer = ui.Row({.gap = 12.0f, .align = kvant::Alignment::Center})) {
                    ui.Caption(metrics.GetStatusString(), kvant::UiVariant::Muted);
                    ui.Spacer();
                    if (auto exitBox = ui.Box({.fixedWidth = 80.0f})) {
                        if (ui.Button("Exit", {.icon = kvant::Icon::Close(), .variant = kvant::UiVariant::Ghost})) {
                            running = false;
                        }
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
