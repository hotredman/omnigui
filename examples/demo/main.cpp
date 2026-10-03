#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <iomanip>
#include <cmath>

#include "core/Assets.hpp"
#include "core/ProcessStats.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "imgui_ext/event_loop.h"
#include "imgui_ext/frame_dedup.h"
#include "imgui_ext/recorder.h"
#include "imgui_ext/renderer.h"
#include "imgui_ext/oscilloscope.h"
#include "imgui_dom/imgui_dom.h"
#include "OmniKitShowcase.hpp"
#include "omnikit.hpp"

struct PerformanceMetrics {
    double cpu_usage_percent = 0.0;
    double ram_working_set_mb = 0.0;
    double ram_peak_working_set_mb = 0.0;
    double cpu_work_time_ms = 0.0;
    double cpu_frame_time_ms = 0.0;
    double gpu_frame_time_ms = 0.0;
    double fps = 0.0;

    double display_fps = 0.0;
    double display_cpu_frame_ms = 0.0;
    double display_cpu_work_ms = 0.0;
    double display_gpu_frame_ms = 0.0;
    uint64_t display_skipped_frames = 0;
    std::chrono::steady_clock::time_point last_display_update_time{};

    std::chrono::steady_clock::time_point last_cpu_check_time{};
    double last_process_cpu_sec = 0.0;
    int num_processors = 1;

    void Init() {
        last_display_update_time = std::chrono::steady_clock::now();
        num_processors = ProcessStats::ProcessorCount();
        last_process_cpu_sec = ProcessStats::ProcessCpuSeconds();
        last_cpu_check_time = std::chrono::steady_clock::now();
    }

    void Update(double frame_time_ms, double work_time_ms, double gpu_time_ms, double current_fps) {
        cpu_frame_time_ms = frame_time_ms;
        cpu_work_time_ms = work_time_ms;
        gpu_frame_time_ms = gpu_time_ms;
        fps = current_fps;

        auto now = std::chrono::steady_clock::now();
        double display_elapsed = std::chrono::duration<double>(now - last_display_update_time).count();
        if (display_elapsed >= 0.25 || display_fps == 0.0) {
            display_fps = current_fps;
            display_cpu_frame_ms = frame_time_ms;
            display_cpu_work_ms = work_time_ms;
            display_gpu_frame_ms = gpu_time_ms;
            display_skipped_frames = ImGuiExt::FrameDeduplicator::Instance().GetTotalFramesSkipped();
            last_display_update_time = now;

            static auto last_print = std::chrono::steady_clock::now();
            if (std::chrono::duration<double>(now - last_print).count() >= 1.0) {
                last_print = now;
                std::cout << "[Live Metrics] FPS: " << display_fps << ", CPU Work: " << display_cpu_work_ms
                          << " ms, Total Frame: " << display_cpu_frame_ms << " ms, Present/GPU: " << display_gpu_frame_ms << " ms\n";
            }
        }

        const ProcessStats::Memory mem = ProcessStats::GetMemory();
        ram_working_set_mb = mem.workingSetMb;
        ram_peak_working_set_mb = mem.peakWorkingSetMb;

        double elapsed_sec = std::chrono::duration<double>(now - last_cpu_check_time).count();
        if (elapsed_sec >= 0.5) {
            const double cur_cpu_sec = ProcessStats::ProcessCpuSeconds();
            if (cur_cpu_sec >= 0.0) {
                cpu_usage_percent = ((cur_cpu_sec - last_process_cpu_sec) / (elapsed_sec * num_processors)) * 100.0;
                last_process_cpu_sec = cur_cpu_sec;
                last_cpu_check_time = now;
            }
        }
    }
};

struct BenchmarkSample {
    bool is_render = false;
    double cpu_usage = 0.0;
    double ram_mb = 0.0;
    double cpu_work_time_ms = 0.0;
    double cpu_total_time_ms = 0.0;
    double gpu_frame_time_ms = 0.0;
    double fps = 0.0;
};

struct BenchmarkResult {
    std::string name;
    size_t frames_rendered = 0;
    double duration_sec = 0.0;
    double avg_fps = 0.0;
    double avg_worktime_ms = 0.0;
    double max_worktime_ms = 0.0;
    double total_frame_time_ms = 0.0;
    double thread_cpu_time_ms = 0.0;
    double thread_cpu_percent = 0.0;
    double process_cpu_percent = 0.0;
    double avg_gpu_time_ms = 0.0;
    double ram_mb = 0.0;
};

struct BenchmarkSession {
    bool running = false;
    std::string name;
    double duration_sec = 5.0;
    std::chrono::steady_clock::time_point start_time;
    std::vector<BenchmarkSample> samples;
    std::vector<BenchmarkResult> all_results;

    double start_thread_cpu_sec = -1.0;

    void Start(const std::string& session_name, double duration) {
        name = session_name;
        duration_sec = duration;
        samples.clear();
        samples.reserve((size_t)(duration * 200));
        start_thread_cpu_sec = ProcessStats::ThreadCpuSeconds();
        start_time = std::chrono::steady_clock::now();
        running = true;
        std::cout << "\n>>> Starting benchmark: " << name << " (" << duration_sec << "s) <<<\n";
    }

    void RecordRender(const PerformanceMetrics& m, double cpu_work_time_ms) {
        if (!running) return;
        samples.push_back({true, m.cpu_usage_percent, m.ram_working_set_mb, cpu_work_time_ms, m.cpu_frame_time_ms, m.gpu_frame_time_ms, m.fps});
        CheckElapsed();
    }

    void RecordIdle(const PerformanceMetrics& m) {
        if (!running) return;
        samples.push_back({false, m.cpu_usage_percent, m.ram_working_set_mb, 0.0, m.cpu_frame_time_ms, 0.0, 0.0});
        CheckElapsed();
    }

    void CheckElapsed() {
        if (!running) return;
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();
        if (elapsed >= duration_sec) {
            Finish();
        }
    }

    void Finish() {
        running = false;
        if (samples.empty()) return;

        auto now = std::chrono::steady_clock::now();
        double actual_duration = std::chrono::duration<double>(now - start_time).count();

        double thread_cpu_time_ms = 0.0;
        double thread_cpu_percent = 0.0;
        const double cur_thread_cpu_sec = ProcessStats::ThreadCpuSeconds();
        if (cur_thread_cpu_sec >= 0.0 && start_thread_cpu_sec >= 0.0) {
            thread_cpu_time_ms = (cur_thread_cpu_sec - start_thread_cpu_sec) * 1000.0;
            thread_cpu_percent = (thread_cpu_time_ms / (actual_duration * 1000.0)) * 100.0;
        }

        size_t rendered_frames = 0;
        double sum_cpu = 0, sum_ram = 0, sum_worktime = 0, sum_totaltime = 0, sum_gputime = 0;
        double max_worktime = 0;
        for (const auto& s : samples) {
            sum_cpu += s.cpu_usage;
            sum_ram += s.ram_mb;
            sum_totaltime += s.cpu_total_time_ms;
            if (s.is_render) {
                rendered_frames++;
                sum_worktime += s.cpu_work_time_ms;
                sum_gputime += s.gpu_frame_time_ms;
                if (s.cpu_work_time_ms > max_worktime) max_worktime = s.cpu_work_time_ms;
            }
        }
        size_t n = samples.size();
        double avg_fps = (double)rendered_frames / actual_duration;
        double avg_worktime = (rendered_frames > 0) ? (sum_worktime / rendered_frames) : 0.0;
        double avg_gputime = (rendered_frames > 0) ? (sum_gputime / rendered_frames) : 0.0;
        double avg_totaltime = (n > 0) ? (sum_totaltime / n) : 0.0;
        double avg_cpu = (n > 0) ? (sum_cpu / n) : 0.0;
        double avg_ram = (n > 0) ? (sum_ram / n) : 0.0;

        BenchmarkResult res;
        res.name = name;
        res.frames_rendered = rendered_frames;
        res.duration_sec = actual_duration;
        res.avg_fps = avg_fps;
        res.avg_worktime_ms = avg_worktime;
        res.max_worktime_ms = max_worktime;
        res.total_frame_time_ms = avg_totaltime;
        res.thread_cpu_time_ms = thread_cpu_time_ms;
        res.thread_cpu_percent = thread_cpu_percent;
        res.process_cpu_percent = avg_cpu;
        res.avg_gpu_time_ms = avg_gputime;
        res.ram_mb = avg_ram;
        all_results.push_back(res);

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n=========================================\n";
        std::cout << " BENCHMARK RESULTS: " << name << "\n";
        std::cout << " Frames Rendered:   " << rendered_frames << " frames in " << actual_duration << "s\n";
        std::cout << " Average FPS:       " << avg_fps << "\n";
        std::cout << " CPU Work Time:     " << avg_worktime << " ms/frame (Max: " << max_worktime << " ms)\n";
        std::cout << " Total Frame Time:  " << avg_totaltime << " ms/frame\n";
        std::cout << " Thread CPU Time:   " << thread_cpu_time_ms << " ms total (" << thread_cpu_percent << "% of 1 core)\n";
        std::cout << " Process CPU Usage: " << avg_cpu << " % (All cores)\n";
        std::cout << " Render/Present:    " << avg_gputime << " ms\n";
        std::cout << " RAM Working Set:   " << avg_ram << " MB\n";
        std::cout << "=========================================\n" << std::endl;
    }

    void PrintSummaryMarkdown() {
        if (all_results.empty()) return;
        std::cout << "\n### Comparative Benchmark Summary\n\n";
        std::cout << "| Mode / Scenario | Frames (5s) | FPS | CPU Work (ms/frame) | CPU Thread Time | 1 Core Load | Render/Present | RAM |\n";
        std::cout << "| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |\n";
        for (const auto& r : all_results) {
            std::cout << "| " << r.name << " | **" << r.frames_rendered << "** | "
                      << std::fixed << std::setprecision(1) << r.avg_fps << " | "
                      << std::fixed << std::setprecision(2) << r.avg_worktime_ms << " ms | "
                      << std::fixed << std::setprecision(1) << r.thread_cpu_time_ms << " ms | "
                      << std::fixed << std::setprecision(2) << r.thread_cpu_percent << " % | "
                      << std::fixed << std::setprecision(2) << r.avg_gpu_time_ms << " ms | "
                      << std::fixed << std::setprecision(1) << r.ram_mb << " MB |\n";
        }
        std::cout << "\n" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::cout.setf(std::ios::unitbuf);
    bool auto_benchmark = false;
    double auto_exit_seconds = 0.0;
    bool use_dom_server = true;
    int dom_port = 8080;
    bool open_browser = false;
    bool headless = false;
    bool use_vector_backend = true;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--benchmark") {
            auto_benchmark = true;
        } else if ((arg == "--profile" || arg == "--auto-exit") && i + 1 < argc) {
            auto_exit_seconds = std::atof(argv[++i]);
        } else if (arg == "--software") {
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
        } else if (arg == "--backend=thorvg" || arg == "--thorvg" || arg == "--vector") {
            use_vector_backend = true;
        } else if (arg == "--backend=sdl" || arg == "--sdl" || arg == "--raster") {
            use_vector_backend = false;
        } else if (arg == "--backend=dom" || arg == "--dom" || arg == "--web" || arg == "--headless" || arg == "--web-only") {
            headless = true;
            use_dom_server = true;
        } else if (arg == "--no-web") {
            use_dom_server = false;
        } else if (arg == "--port" && i + 1 < argc) {
            dom_port = std::atoi(argv[++i]);
        } else if (arg == "--open-browser" || arg == "--browser" || arg == "-b") {
            open_browser = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "OmniGUI Demo - Multi-Backend Runner\n\n"
                      << "Backends:\n"
                      << "  --backend=thorvg, --thorvg   Launch Desktop with ThorVG Vector Renderer (default)\n"
                      << "  --backend=sdl,    --sdl      Launch Desktop with standard SDL3 Renderer (raster triangles)\n"
                      << "  --backend=dom,    --dom      Launch Headless Web DOM Server\n\n"
                      << "Options:\n"
                      << "  --open-browser,   -b         Open web browser automatically\n"
                      << "  --port <port>                Web DOM server port (default: 8080)\n"
                      << "  --no-web                     Disable Web DOM server in desktop mode\n"
                      << "  --software                   Force SDL software rendering driver\n"
                      << "  --benchmark                  Run automated performance benchmark\n"
                      << "  --auto-exit <sec>            Exit after N seconds\n"
                      << "  --help,           -h         Show this help message\n";
            return 0;
        }
    }

    if (headless) {
        SDL_Init(0);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(1280, 720);
        io.DeltaTime = 1.0f / 60.0f;
        io.IniFilename = nullptr;

        unsigned char* pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        ImGuiExt::OscilloscopeWidget oscilloscope;

        ImGuiDom::StartServer(dom_port);
        if (open_browser) {
            std::string url = "http://localhost:" + std::to_string(dom_port);
            SDL_OpenURL(url.c_str());
        }

        std::cout << "\n========================================================\n";
        std::cout << "  Dear ImGui Web DOM Server (Headless Mode)\n";
        std::cout << "  URL: http://localhost:" << dom_port << "\n";
        std::cout << "========================================================\n\n";

        uint64_t dom_frame = 0;
        auto app_start = std::chrono::steady_clock::now();
        while (true) {
            if (auto_exit_seconds > 0.0) {
                double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - app_start).count();
                if (elapsed >= auto_exit_seconds) break;
            }

            dom_frame++;
            ImGuiDom::BeginFrame(dom_frame);
            ImGui::NewFrame();

            float pad = 12.0f;
            float total_w = io.DisplaySize.x;
            float total_h = io.DisplaySize.y;

            float hud_w = 480.0f;
            float hud_h = total_h - pad * 2.0f;
            float demo_x = pad + hud_w + pad;
            float demo_w = total_w - demo_x - pad;
            float demo_h = total_h - pad * 2.0f;

            float half_h = (demo_h - pad) * 0.48f;
            float osc_h = demo_h - half_h - pad;

            ImGui::SetNextWindowPos(ImVec2(demo_x, pad), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(demo_w, half_h), ImGuiCond_FirstUseEver);
            ImGui::ShowDemoWindow();

            ImGui::SetNextWindowPos(ImVec2(demo_x, pad + half_h + pad), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(demo_w, osc_h), ImGuiCond_FirstUseEver);
            oscilloscope.RenderUI();

            ImGui::SetNextWindowPos(ImVec2(pad, pad), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(hud_w, hud_h), ImGuiCond_FirstUseEver);
            ImGuiDom::Begin("ImGui Web DOM Backend - Server Controls", nullptr);
            ImGuiDom::Text("Dear ImGui %s (Headless Web Server)", IMGUI_VERSION);
            ImGuiDom::Separator();
            if (ImGuiDom::BeginTabBar("HudTabs")) {
                if (ImGuiDom::BeginTabItem("Status")) {
                    ImGuiDom::TextColored(ImVec4(0.2f, 0.9f, 1.0f, 1.0f), "Web Client URL: http://localhost:%d", dom_port);
                    ImGuiDom::Text("Status: Streaming DOM & Canvas @ 60 FPS");
                    ImGuiDom::EndTabItem();
                }

                if (ImGuiDom::BeginTabItem("Element Gallery")) {
                    ImGuiDom::Text("Canonical Widget Gallery & State Matrix:");
                    ImGuiDom::Separator();

                    // Buttons
                    if (ImGuiDom::Button("Gallery Button")) {}
                    ImGuiDom::SameLine();
                    ImGui::BeginDisabled(true);
                    ImGuiDom::Button("Disabled Button");
                    ImGui::EndDisabled();

                    // Checkboxes
                    static bool gal_c1 = false;
                    static bool gal_c2 = true;
                    ImGuiDom::Checkbox("Unchecked", &gal_c1);
                    ImGuiDom::SameLine();
                    ImGuiDom::Checkbox("Checked", &gal_c2);
                    ImGuiDom::SameLine();
                    ImGui::BeginDisabled(true);
                    ImGuiDom::Checkbox("Disabled Check", &gal_c2);
                    ImGui::EndDisabled();

                    // Radio
                    static int gal_r = 1;
                    ImGuiDom::RadioButton("Radio 0", &gal_r, 0);
                    ImGuiDom::SameLine();
                    ImGuiDom::RadioButton("Radio 1", &gal_r, 1);
                    ImGuiDom::SameLine();
                    ImGui::BeginDisabled(true);
                    ImGuiDom::RadioButton("Radio Dis", &gal_r, 1);
                    ImGui::EndDisabled();

                    // Slider
                    static float gal_slider = 50.0f;
                    ImGuiDom::SliderFloat("Slider 50%", &gal_slider, 0.0f, 100.0f);
                    ImGui::BeginDisabled(true);
                    ImGuiDom::SliderFloat("Disabled Slider", &gal_slider, 0.0f, 100.0f);
                    ImGui::EndDisabled();

                    // Input
                    static char gal_text[64] = "Editable text";
                    ImGuiDom::InputText("Input Text", gal_text, sizeof(gal_text));
                    ImGui::BeginDisabled(true);
                    ImGuiDom::InputText("Disabled Input", gal_text, sizeof(gal_text));
                    ImGui::EndDisabled();

                    // Combo
                    static int gal_combo = 0;
                    static const char* gal_items[] = { "Option Alpha", "Option Beta", "Option Gamma" };
                    ImGuiDom::Combo("Select Combo", &gal_combo, gal_items, 3);

                    // ProgressBar
                    ImGuiDom::ProgressBar(0.70f, ImVec2(-1, 0), "70%");

                    // CollapsingHeader
                    if (ImGui::CollapsingHeader("Gallery Collapsing Header")) {
                        ImGuiDom::Text("Inside Collapsing Header");
                        if (ImGui::TreeNode("Gallery Tree Node")) {
                            ImGuiDom::Text("Tree node content");
                            ImGui::TreePop();
                        }
                    }

                    ImGuiDom::EndTabItem();
                }

                ImGuiDom::EndTabBar();
            }
            ImGuiDom::End();

            ImGui::Render();
            ImGuiDom::EndFrame();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        ImGuiDom::StopServer();
        ImGui::DestroyContext();
        SDL_Quit();
        return 0;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_DisplayID display_id = SDL_GetPrimaryDisplay();
    float main_scale = SDL_GetDisplayContentScale(display_id);
    if (main_scale <= 0.0f) main_scale = 1.0f;

    SDL_Rect work_area{0, 0, 1280, 720};
    SDL_GetDisplayUsableBounds(display_id, &work_area);

    int mon_w = work_area.w > 0 ? work_area.w : 1280;
    int mon_h = work_area.h > 0 ? work_area.h : 720;

    int base_w = (std::min)(mon_w - 40, (int)(1360 * (main_scale > 1.25f ? 1.0f : main_scale)));
    int base_h = (std::min)(mon_h - 60, (int)(800 * (main_scale > 1.25f ? 1.0f : main_scale)));
    if (base_w < 1024) base_w = (std::min)(1024, mon_w - 20);
    if (base_h < 640)  base_h = (std::min)(640, mon_h - 40);

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    const char* win_title = use_vector_backend
        ? "OmniGUI - ImGui + ThorVG Vector Backend (SDL3)"
        : "OmniGUI - ImGui + Standard SDL3 Renderer";
    if (!SDL_CreateWindowAndRenderer(win_title,
                                     base_w, base_h,
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
                                     &window, &renderer)) {
        std::cerr << "Failed to create SDL window and renderer: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetRenderVSync(renderer, 1); // Enable vsync (60Hz baseline)

    float window_scale = SDL_GetWindowDisplayScale(window);
    if (window_scale > 0.0f) main_scale = window_scale;

    std::cout << "[Demo] Content Scale: " << main_scale << ", Window Size: " << base_w << "x" << base_h
              << " | Presentation Driver: " << (renderer ? SDL_GetRendererName(renderer) : "None")
              << " | Active Renderer: " << (use_vector_backend ? "ThorVG Vector Rasterizer" : "Standard SDL3 Triangles") << "\n";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // Ensure demo always starts with pristine, properly proportioned layout

    ImGui::StyleColorsDark();

    // Scale UI style according to monitor DPI
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);

    // Load clean TrueType font supporting Cyrillic + Latin glyph ranges
    const std::string font_path = Assets::Resolve("fonts/Roboto-Regular.ttf");
    float font_size = 19.0f * main_scale;
    ImFontConfig cfg;
    cfg.OversampleH = 1;
    cfg.OversampleV = 1;
    cfg.PixelSnapH = false;
    ImFont* font = font_path.empty() ? nullptr
        : io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    if (!font) {
        font = io.Fonts->AddFontDefault();
    }

    // Initialize design system theme scale and fonts
    UiTheme::Get().SetScale(main_scale);
    UiTheme::Get().LoadFonts();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Initialize Event Loop manager
    ImGuiExt::InitEventLoop(window);

    // Initialize Web DOM Server if enabled
    if (use_dom_server) {
        ImGuiDom::DomContext::Instance().SetEventCallback([]() {
            ImGuiExt::RequestRepaint(3);
        });
        ImGuiDom::StartServer(dom_port);
        if (open_browser) {
            std::string url = "http://localhost:" + std::to_string(dom_port);
            SDL_OpenURL(url.c_str());
        }
    }

    ImGuiExt::IRenderer* vector_renderer = ImGuiExt::CreateThorVGRenderer();
    int init_fb_w = 0, init_fb_h = 0;
    SDL_GetRenderOutputSize(renderer, &init_fb_w, &init_fb_h);
    vector_renderer->Init(init_fb_w, init_fb_h);
    if (font && !font_path.empty()) {
        vector_renderer->LoadFontFile(font_path.c_str());
    }
    ImGuiExt::SetVectorInterception(use_vector_backend);

    SDL_Texture* vector_texture = nullptr;
    int texture_w = 0, texture_h = 0;

    PerformanceMetrics metrics;
    metrics.Init();
    BenchmarkSession bench;

    ImGuiExt::OscilloscopeWidget oscilloscope;
    if (auto_benchmark) {
        oscilloscope.GetSignal().SetPaused(true);
    }

    int auto_bench_stage = auto_benchmark ? 1 : 0;
    auto bench_stage_timer = std::chrono::steady_clock::now();

    auto app_start_time = std::chrono::steady_clock::now();
    while (!ImGuiExt::EventLoop::Instance().ShouldClose()) {
        auto frame_start = std::chrono::steady_clock::now();

        if (auto_exit_seconds > 0.0) {
            double run_time = std::chrono::duration<double>(frame_start - app_start_time).count();
            if (run_time >= auto_exit_seconds) {
                std::cout << "[Profile] Exiting after " << run_time << " s\n";
                ImGuiExt::EventLoop::Instance().SetShouldClose(true);
                break;
            }
        }

        // Automated benchmark sequencer
        if (auto_benchmark) {
            auto now = std::chrono::steady_clock::now();
            double stage_elapsed = std::chrono::duration<double>(now - bench_stage_timer).count();

            if (auto_bench_stage == 1 && stage_elapsed > 1.0) {
                // Stage 0: Continuous Idle
                ImGuiExt::SetReactiveMode(false);
                bench.Start("Stage 0: Continuous - Idle (5s)", 5.0);
                auto_bench_stage = 2;
            } else if (auto_bench_stage == 2 && !bench.running) {
                // Stage 0: Continuous Active
                bench.Start("Stage 0: Continuous - Mouse Motion (5s)", 5.0);
                bench_stage_timer = std::chrono::steady_clock::now();
                auto_bench_stage = 3;
            } else if (auto_bench_stage == 3) {
                double t = std::chrono::duration<double>(now - bench_stage_timer).count() * 4.0;
                double mx = (640.0 + 300.0 * std::sin(t)) * main_scale;
                double my = (360.0 + 200.0 * std::cos(t)) * main_scale;
                SDL_WarpMouseInWindow(window, (float)mx, (float)my);
                ImGuiExt::RequestRepaint(3);
                if (!bench.running) {
                    // Transition to Stage 1
                    ImGuiExt::SetReactiveMode(true);
                    bench.Start("Stage 1: Reactive - Idle (5s)", 5.0);
                    bench_stage_timer = std::chrono::steady_clock::now();
                    auto_bench_stage = 4;
                }
            } else if (auto_bench_stage == 4 && !bench.running) {
                // Stage 1: Reactive Active
                bench.Start("Stage 1: Reactive - Mouse Motion (5s)", 5.0);
                bench_stage_timer = std::chrono::steady_clock::now();
                auto_bench_stage = 5;
            } else if (auto_bench_stage == 5) {
                double t = std::chrono::duration<double>(now - bench_stage_timer).count() * 4.0;
                double mx = (640.0 + 300.0 * std::sin(t)) * main_scale;
                double my = (360.0 + 200.0 * std::cos(t)) * main_scale;
                SDL_WarpMouseInWindow(window, (float)mx, (float)my);
                ImGuiExt::RequestRepaint(3);
                if (!bench.running) {
                    // Transition to Stage 2 (Reactive + Dedup)
                    ImGuiExt::SetReactiveMode(true);
                    ImGuiExt::SetFrameDeduplication(true);
                    bench.Start("Stage 2: Reactive+Dedup - Idle (5s)", 5.0);
                    bench_stage_timer = std::chrono::steady_clock::now();
                    auto_bench_stage = 6;
                }
            } else if (auto_bench_stage == 6 && !bench.running) {
                bench.Start("Stage 2: Reactive+Dedup - Mouse Motion (5s)", 5.0);
                bench_stage_timer = std::chrono::steady_clock::now();
                auto_bench_stage = 7;
            } else if (auto_bench_stage == 7) {
                double t = std::chrono::duration<double>(now - bench_stage_timer).count() * 4.0;
                double mx = (640.0 + 300.0 * std::sin(t)) * main_scale;
                double my = (360.0 + 200.0 * std::cos(t)) * main_scale;
                SDL_WarpMouseInWindow(window, (float)mx, (float)my);
                ImGuiExt::RequestRepaint(3);
                if (!bench.running) {
                    // Transition to Stage 3 (ThorVG Vector Backend - Idle)
                    use_vector_backend = true;
                    ImGuiExt::SetVectorInterception(true);
                    bench.Start("Stage 3: ThorVG Vector - Idle (5s)", 5.0);
                    bench_stage_timer = std::chrono::steady_clock::now();
                    auto_bench_stage = 8;
                }
            } else if (auto_bench_stage == 8 && !bench.running) {
                // Stage 3 (ThorVG Vector Backend - Mouse Motion)
                bench.Start("Stage 3: ThorVG Vector - Mouse Motion (5s)", 5.0);
                bench_stage_timer = std::chrono::steady_clock::now();
                auto_bench_stage = 9;
            } else if (auto_bench_stage == 9) {
                double t = std::chrono::duration<double>(now - bench_stage_timer).count() * 4.0;
                double mx = (640.0 + 300.0 * std::sin(t)) * main_scale;
                double my = (360.0 + 200.0 * std::cos(t)) * main_scale;
                SDL_WarpMouseInWindow(window, (float)mx, (float)my);
                ImGuiExt::RequestRepaint(3);
                if (!bench.running) {
                    auto_bench_stage = 10;
                    std::cout << "All automated benchmarks complete!\n";
                    bench.PrintSummaryMarkdown();
                    ImGuiExt::EventLoop::Instance().SetShouldClose(true);
                }
            }
        }

        // Wait / Poll events according to event loop configuration
        bool want_text = io.WantTextInput;
        bool should_render = ImGuiExt::EventLoop::Instance().StepBeforeWait(want_text, false, bench.running);

        if (ImGuiExt::EventLoop::Instance().ShouldClose()) {
            break;
        }

        if (!should_render) {
            auto now = std::chrono::steady_clock::now();
            double frame_time_ms = std::chrono::duration<double, std::milli>(now - frame_start).count();
            metrics.Update(frame_time_ms, 0.0, 0.0, 0.0);
            bench.RecordIdle(metrics);
            continue;
        }

        auto cpu_work_start = std::chrono::steady_clock::now();

        static uint64_t dom_frame = 0;
        dom_frame++;
        ImGuiDom::BeginFrame(dom_frame);

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // ====================================================================
        // OmniKit Workstation Showcase
        // Component-pure shell (Header, Toolbar, Sidebar, ContentArea, StatusBar)
        // Self-docking borderless panels tile 100% of the viewport seamlessly.
        // ====================================================================
        OmniKitShowcase::RenderUI(main_scale);

        ImGui::Render();
        ImGuiDom::EndFrame();
        ImDrawData* draw_data = ImGui::GetDrawData();

        if (use_vector_backend) {
            ImGuiExt::Recorder::Instance().EndFrame();
        }

        // Stage 2: Deduplication check
        bool frame_changed = ImGuiExt::FrameDeduplicator::Instance().ShouldRenderFrame(draw_data);
        if (!frame_changed) {
            auto cpu_work_end = std::chrono::steady_clock::now();
            double cpu_work_time_ms = std::chrono::duration<double, std::milli>(cpu_work_end - cpu_work_start).count();
            auto frame_end = std::chrono::steady_clock::now();
            double frame_time_ms = std::chrono::duration<double, std::milli>(frame_end - frame_start).count();

            metrics.Update(frame_time_ms, cpu_work_time_ms, 0.0, io.Framerate);
            bench.RecordRender(metrics, cpu_work_time_ms);

            ImGuiExt::EventLoop::Instance().StepAfterRender();
            continue;
        }

        int display_w = 0, display_h = 0;
        SDL_GetRenderOutputSize(renderer, &display_w, &display_h);

        auto render_start = std::chrono::steady_clock::now();

        ImVec4 clear_col = ImColor(UiTheme::Get().palette.bgApp).Value;
        SDL_SetRenderDrawColor(renderer,
                               static_cast<Uint8>(clear_col.x * 255.0f),
                               static_cast<Uint8>(clear_col.y * 255.0f),
                               static_cast<Uint8>(clear_col.z * 255.0f),
                               255);
        SDL_RenderClear(renderer);

        if (use_vector_backend) {
            vector_renderer->SetClearColor(ImGuiExt::Color::FromImU32(UiTheme::Get().palette.bgApp));
            vector_renderer->Resize(display_w, display_h);
            vector_renderer->RenderDrawData(draw_data);
            const uint32_t* pixels = vector_renderer->GetPixelBuffer();

            if (!vector_texture || texture_w != display_w || texture_h != display_h) {
                if (vector_texture) {
                    SDL_DestroyTexture(vector_texture);
                }
                vector_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, display_w, display_h);
                texture_w = display_w;
                texture_h = display_h;
            }

            if (vector_texture && pixels) {
                SDL_UpdateTexture(vector_texture, nullptr, pixels, display_w * 4);
                SDL_RenderTexture(renderer, vector_texture, nullptr, nullptr);
            }
        } else {
            ImGui_ImplSDLRenderer3_RenderDrawData(draw_data, renderer);
        }

        auto cpu_work_end = std::chrono::steady_clock::now();
        double cpu_work_time_ms = std::chrono::duration<double, std::milli>(cpu_work_end - cpu_work_start).count();

        SDL_RenderPresent(renderer);

        auto render_end = std::chrono::steady_clock::now();
        double gpu_time_ms = std::chrono::duration<double, std::milli>(render_end - render_start).count();

        auto frame_end = std::chrono::steady_clock::now();
        double frame_time_ms = std::chrono::duration<double, std::milli>(frame_end - frame_start).count();

        metrics.Update(frame_time_ms, cpu_work_time_ms, gpu_time_ms, io.Framerate);
        bench.RecordRender(metrics, cpu_work_time_ms);

        ImGuiExt::EventLoop::Instance().StepAfterRender();
    }

    if (vector_texture) {
        SDL_DestroyTexture(vector_texture);
        vector_texture = nullptr;
    }

    if (vector_renderer) {
        vector_renderer->Shutdown();
        delete vector_renderer;
        vector_renderer = nullptr;
    }

    if (use_dom_server) {
        ImGuiDom::StopServer();
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
