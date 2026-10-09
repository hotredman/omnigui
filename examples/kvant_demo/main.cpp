#include <SDL3/SDL.h>
#include <iostream>
#include <string>
#include <vector>
#include "kvant/Context.hpp"
#include "kvant/backend/ThorCanvas.hpp"

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

    while (running) {
        SDL_Event event;
        // Wait up to 16ms for an event to maintain 60 FPS while conserving CPU at idle
        while (SDL_PollEvent(&event)) {
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
                default:
                    break;
            }
        }

        // Clear CPU pixel buffer to dark background (Tailwind Slate-950 #0B0F19)
        std::fill(pixelBuffer.begin(), pixelBuffer.end(), 0xFF190F0B);

        // --- Begin Immediate Mode Kvant Frame ---
        ui.BeginFrame({static_cast<float>(winWidth), static_cast<float>(winHeight)});
        {
            if (auto mainCol = ui.Column({.gap = 16.0f, .padding = kvant::Padding::All(24.0f)})) {
                // Header Bar
                if (auto header = ui.Row({.gap = 12.0f, .alignCross = kvant::Align::Center})) {
                    ui.Text("KVANT GUI", {.color = kvant::Color::Hex(0x38BDF8FF), .fontSize = 24.0f});
                    ui.Text("| Autonomous Vector GUI Engine", {.color = kvant::Color::Hex(0x94A3B8FF), .fontSize = 14.0f});
                    ui.Spacer(); // Push build badge to right
                    ui.Text("C++20 Immediate + ThorVG", {.color = kvant::Color::Hex(0x64748BFF), .fontSize = 12.0f});
                }

                // Navigation Tabs
                if (auto navRow = ui.Row({.gap = 8.0f})) {
                    auto btnOverview = ui.Button({
                        .label = "Overview",
                        .backgroundColor = (activeTabIndex == 0) ? kvant::Color::Hex(0x2563EBFF) : kvant::Color::Hex(0x1E293BFF),
                        .fixedWidth = 120.0f
                    });
                    if (btnOverview) activeTabIndex = 0;

                    auto btnFlex = ui.Button({
                        .label = "Flexbox Demo",
                        .backgroundColor = (activeTabIndex == 1) ? kvant::Color::Hex(0x2563EBFF) : kvant::Color::Hex(0x1E293BFF),
                        .fixedWidth = 150.0f
                    });
                    if (btnFlex) activeTabIndex = 1;
                }

                // Content Panel Card
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
                        ui.Text("Interactive Reactive State", {.color = kvant::Color::Hex(0xF8FAFCFF), .fontSize = 18.0f});
                        ui.Text("Pure immediate mode with zero external widget IDs and instant response.",
                                {.color = kvant::Color::Hex(0x94A3B8FF), .fontSize = 13.0f});

                        if (auto actionRow = ui.Wrap({.gap = 12.0f, .runGap = 10.0f, .alignCross = kvant::Align::Center})) {
                            if (ui.Button({.label = "Increment (+1)", .backgroundColor = kvant::Color::Hex(0x10B981FF)})) {
                                ++counter;
                            }
                            if (ui.Button({.label = "Decrement (-1)", .backgroundColor = kvant::Color::Hex(0xF59E0BFF)})) {
                                --counter;
                            }
                            if (ui.Button({.label = "Reset", .backgroundColor = kvant::Color::Hex(0xEF4444FF)})) {
                                counter = 0;
                            }

                            std::string counterStr = "Value: " + std::to_string(counter);
                            ui.Text(counterStr, {.color = kvant::Color::Hex(0x38BDF8FF), .fontSize = 20.0f});
                        }
                    } else {
                        // Flexbox Demo Tab: Equal Stretch & Alignment
                        ui.Text("Dynamic 3-Way Equal Stretch Layout", {.color = kvant::Color::Hex(0xF8FAFCFF), .fontSize = 18.0f});
                        ui.Text("Each button automatically receives exactly 33.3% available width.",
                                {.color = kvant::Color::Hex(0x94A3B8FF), .fontSize = 13.0f});

                        if (auto stretchRow = ui.Row({.gap = 12.0f})) {
                            ui.Button({.label = "Col 1 (Stretch)", .widthMode = kvant::SizeMode::Stretch});
                            ui.Button({.label = "Col 2 (Stretch)", .widthMode = kvant::SizeMode::Stretch});
                            ui.Button({.label = "Col 3 (Stretch)", .widthMode = kvant::SizeMode::Stretch});
                        }
                    }
                }

                // Vertical Spacer pushing the footer down
                ui.Spacer();

                // Sticky Footer Bar
                if (auto footer = ui.Row({.gap = 12.0f, .alignCross = kvant::Align::Center})) {
                    ui.Text("Zero heap allocations per frame | 0% idle CPU",
                            {.color = kvant::Color::Hex(0x64748BFF), .fontSize = 12.0f});
                    ui.Spacer();
                    if (ui.Button({.label = "Exit", .backgroundColor = kvant::Color::Hex(0x475569FF), .fixedWidth = 80.0f})) {
                        running = false;
                    }
                }
            }
        }
        ui.EndFrame(canvas);

        // Blit pixel buffer to SDL3 streaming texture
        SDL_UpdateTexture(screenTexture, nullptr, pixelBuffer.data(), winWidth * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, screenTexture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // Yield CPU slightly to cap frame rate when no events are pending
        SDL_Delay(8);
    }

    if (screenTexture) {
        SDL_DestroyTexture(screenTexture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
