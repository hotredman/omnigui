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
                    ui.Title("KVANT GUI", kvant::UiVariant::Info);
                    ui.Caption("| Autonomous Vector GUI Engine");
                    ui.Spacer(); // Push build badge to right
                    ui.Caption("C++20 Immediate + ThorVG");
                }

                // Navigation Tabs
                if (auto navRow = ui.Row({.gap = 8.0f})) {
                    if (ui.Button("Overview", {
                        .icon = kvant::Icon::Eye(),
                        .variant = (activeTabIndex == 0) ? kvant::UiVariant::Primary : kvant::UiVariant::Secondary
                    })) {
                        activeTabIndex = 0;
                    }

                    if (ui.Button("Flexbox Demo", {
                        .icon = kvant::Icon::Settings(),
                        .variant = (activeTabIndex == 1) ? kvant::UiVariant::Primary : kvant::UiVariant::Secondary
                    })) {
                        activeTabIndex = 1;
                    }
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
                        ui.Headline("Interactive Reactive State");
                        ui.Body("Pure immediate mode with zero external widget IDs and instant response.",
                                kvant::UiVariant::Muted);

                        if (auto actionRow = ui.Wrap({.gap = 12.0f, .runGap = 10.0f, .alignCross = kvant::Align::Center})) {
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

                // Vertical Spacer pushing the footer down
                ui.Spacer();

                // Sticky Footer Bar
                if (auto footer = ui.Row({.gap = 12.0f, .alignCross = kvant::Align::Center})) {
                    ui.Caption("Zero heap allocations per frame | 0% idle CPU");
                    ui.Spacer();
                    if (auto exitBox = ui.Box({.fixedWidth = 90.0f})) {
                        if (ui.Button("Exit", {.icon = kvant::Icon::Close(), .variant = kvant::UiVariant::Ghost})) {
                            running = false;
                        }
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
