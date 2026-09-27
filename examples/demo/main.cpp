#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <iomanip>
#include <cmath>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#endif

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

#ifdef _WIN32
    ULARGE_INTEGER last_kernel_time{0};
    ULARGE_INTEGER last_user_time{0};
    std::chrono::steady_clock::time_point last_cpu_check_time{};
    int num_processors = 1;
#endif

    void Init() {
        last_display_update_time = std::chrono::steady_clock::now();
#ifdef _WIN32
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        num_processors = (int)sys_info.dwNumberOfProcessors;

        FILETIME ftCreation, ftExit, ftKernel, ftUser;
        GetProcessTimes(GetCurrentProcess(), &ftCreation, &ftExit, &ftKernel, &ftUser);
        last_kernel_time.LowPart = ftKernel.dwLowDateTime;
        last_kernel_time.HighPart = ftKernel.dwHighDateTime;
        last_user_time.LowPart = ftUser.dwLowDateTime;
        last_user_time.HighPart = ftUser.dwHighDateTime;
        last_cpu_check_time = std::chrono::steady_clock::now();
#endif
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

#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            ram_working_set_mb = (double)pmc.WorkingSetSize / (1024.0 * 1024.0);
            ram_peak_working_set_mb = (double)pmc.PeakWorkingSetSize / (1024.0 * 1024.0);
        }

        double elapsed_sec = std::chrono::duration<double>(now - last_cpu_check_time).count();
        if (elapsed_sec >= 0.5) {
            FILETIME ftCreation, ftExit, ftKernel, ftUser;
            if (GetProcessTimes(GetCurrentProcess(), &ftCreation, &ftExit, &ftKernel, &ftUser)) {
                ULARGE_INTEGER cur_kernel, cur_user;
                cur_kernel.LowPart = ftKernel.dwLowDateTime;
                cur_kernel.HighPart = ftKernel.dwHighDateTime;
                cur_user.LowPart = ftUser.dwLowDateTime;
                cur_user.HighPart = ftUser.dwHighDateTime;

                ULONGLONG kernel_diff = cur_kernel.QuadPart - last_kernel_time.QuadPart;
                ULONGLONG user_diff = cur_user.QuadPart - last_user_time.QuadPart;
                ULONGLONG total_diff = kernel_diff + user_diff;

                double proc_time_sec = (double)total_diff / 10000000.0;
                cpu_usage_percent = (proc_time_sec / (elapsed_sec * num_processors)) * 100.0;

                last_kernel_time = cur_kernel;
                last_user_time = cur_user;
                last_cpu_check_time = now;
            }
        }
#endif
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

#ifdef _WIN32
    FILETIME start_kernel_time{0, 0};
    FILETIME start_user_time{0, 0};
#endif

    void Start(const std::string& session_name, double duration) {
        name = session_name;
        duration_sec = duration;
        samples.clear();
        samples.reserve((size_t)(duration * 200));
#ifdef _WIN32
        FILETIME ftCreation, ftExit;
        GetThreadTimes(GetCurrentThread(), &ftCreation, &ftExit, &start_kernel_time, &start_user_time);
#endif
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
#ifdef _WIN32
        FILETIME ftCreation, ftExit, cur_kernel, cur_user;
        GetThreadTimes(GetCurrentThread(), &ftCreation, &ftExit, &cur_kernel, &cur_user);
        ULARGE_INTEGER sk, su, ck, cu;
        sk.LowPart = start_kernel_time.dwLowDateTime; sk.HighPart = start_kernel_time.dwHighDateTime;
        su.LowPart = start_user_time.dwLowDateTime;   su.HighPart = start_user_time.dwHighDateTime;
        ck.LowPart = cur_kernel.dwLowDateTime;         ck.HighPart = cur_kernel.dwHighDateTime;
        cu.LowPart = cur_user.dwLowDateTime;           cu.HighPart = cur_user.dwHighDateTime;
        ULONGLONG thread_time_100ns = (ck.QuadPart - sk.QuadPart) + (cu.QuadPart - su.QuadPart);
        thread_cpu_time_ms = (double)thread_time_100ns / 10000.0;
        thread_cpu_percent = (thread_cpu_time_ms / (actual_duration * 1000.0)) * 100.0;
#endif

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
        std::cout << "| Режим / Сценарий | Кадров за 5с | FPS | CPU Work (мс/кадр) | Время потока CPU | Загрузка 1 ядра | Render/Present | RAM |\n";
        std::cout << "| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |\n";
        for (const auto& r : all_results) {
            std::cout << "| " << r.name << " | **" << r.frames_rendered << "** | "
                      << std::fixed << std::setprecision(1) << r.avg_fps << " | "
                      << std::fixed << std::setprecision(2) << r.avg_worktime_ms << " мс | "
                      << std::fixed << std::setprecision(1) << r.thread_cpu_time_ms << " мс | "
                      << std::fixed << std::setprecision(2) << r.thread_cpu_percent << " % | "
                      << std::fixed << std::setprecision(2) << r.avg_gpu_time_ms << " мс | "
                      << std::fixed << std::setprecision(1) << r.ram_mb << " МБ |\n";
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
    const char* font_path = "C:/Windows/Fonts/segoeui.ttf";
    float font_size = 19.0f * main_scale;
    ImFontConfig cfg;
    cfg.OversampleH = 1;
    cfg.OversampleV = 1;
    cfg.PixelSnapH = false;
    ImFont* font = io.Fonts->AddFontFromFileTTF(font_path, font_size, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    if (!font) {
        font_path = "C:/Windows/Fonts/arial.ttf";
        font = io.Fonts->AddFontFromFileTTF(font_path, font_size, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    }
    if (!font) {
        font = io.Fonts->AddFontDefault();
    }

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
    if (font) {
        vector_renderer->LoadFontFile(font_path);
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

        // Responsive side-by-side layout
        float pad = 12.0f * main_scale;
        float total_w = io.DisplaySize.x;
        float total_h = io.DisplaySize.y;

        float hud_w = (std::min)(560.0f * main_scale, (total_w - pad * 3.0f) * 0.44f);
        float hud_h = total_h - pad * 2.0f;
        float demo_x = pad + hud_w + pad;
        float demo_w = total_w - demo_x - pad;
        float demo_h = total_h - pad * 2.0f;

        // 1. Show standard ImGui Demo (top-right)
        float half_h = (demo_h - pad) * 0.48f;
        float osc_h = demo_h - half_h - pad;

        ImGui::SetNextWindowPos(ImVec2(demo_x, pad), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(demo_w, half_h), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow();

        // 2. Real-Time Oscilloscope Widget with LTTB (bottom-right)
        ImGui::SetNextWindowPos(ImVec2(demo_x, pad + half_h + pad), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(demo_w, osc_h), ImGuiCond_FirstUseEver);
        oscilloscope.RenderUI();

        // 2. Metrics & Benchmark HUD
        {
            ImGui::SetNextWindowPos(ImVec2(pad, pad), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(hud_w, hud_h), ImGuiCond_FirstUseEver);
            ImGuiDom::Begin("ImGui Vector Backend - Controls & Metrics", nullptr);
            ImGuiDom::Text("Dear ImGui %s + SDL3", IMGUI_VERSION);
            ImGuiDom::Separator();

            if (ImGuiDom::BeginTabBar("HudTabs")) {
                if (ImGuiDom::BeginTabItem("Backends")) {
                    ImGuiDom::Text("Active Render Backends (User Choice):");
                    if (ImGuiDom::Checkbox("1. ImGui - Thor - SDL (Vector Rasterizer)", &use_vector_backend)) {
                        ImGuiExt::SetVectorInterception(use_vector_backend);
                    }
                    if (use_vector_backend) {
                        ImGuiDom::SameLine();
                        ImGuiDom::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "[ACTIVE: Vector Primitives]");
                    } else {
                        ImGuiDom::SameLine();
                        ImGuiDom::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "[2. ImGui - SDL Triangles Active]");
                    }

                    if (ImGuiDom::Checkbox("3. ImGui - DOM (Native HTML5 Web Page)", &use_dom_server)) {
                        if (use_dom_server) {
                            ImGuiDom::DomContext::Instance().SetEventCallback([]() {
                                ImGuiExt::RequestRepaint(3);
                            });
                            ImGuiDom::StartServer(dom_port);
                        } else {
                            ImGuiDom::StopServer();
                        }
                    }
                    if (use_dom_server) {
                        ImGuiDom::SameLine();
                        ImGuiDom::TextColored(ImVec4(0.2f, 0.9f, 1.0f, 1.0f), "[http://localhost:%d]", dom_port);
                        ImGuiDom::SameLine();
                        if (ImGuiDom::Button("Open Web Client")) {
                            std::string url = "http://localhost:" + std::to_string(dom_port);
                            SDL_OpenURL(url.c_str());
                        }
                        ImGuiDom::SetItemTooltip("Opens browser to connect via full-duplex WebSocket");
                    }

                    ImGuiDom::Separator();
                    bool reactive = ImGuiExt::IsReactiveMode();
                    if (ImGuiDom::Checkbox("Reactive Event-Driven Loop (Stage 1)", &reactive)) {
                        ImGuiExt::SetReactiveMode(reactive);
                    }
                    ImGuiDom::SetItemTooltip("Suspends main thread when no events or animations are active");

                    bool dedup = ImGuiExt::IsFrameDeduplicationEnabled();
                    if (ImGuiDom::Checkbox("Frame Deduplication (Stage 2)", &dedup)) {
                        ImGuiExt::SetFrameDeduplication(dedup);
                    }
                    ImGuiDom::SetItemTooltip("Skips GPU rendering when draw data matches previous frame hash");

                    if (use_vector_backend) {
                        size_t total_cmds = 0, vector_cmds = 0, fallback_cmds = 0;
                        for (const auto& kv : ImGuiExt::Recorder::Instance().GetStreams()) {
                            total_cmds += kv.second.commands.size();
                            for (const auto& c : kv.second.commands) {
                                if (c.type == ImGuiExt::CmdType::FallbackMesh) fallback_cmds++;
                                else vector_cmds++;
                            }
                        }
                        ImGuiDom::Text("Recorded: %zu commands (%zu vector, %zu fallback mesh)", total_cmds, vector_cmds, fallback_cmds);
                    }

                    if (ImGuiDom::Button("Simulate Texture Update")) {
                        ImGuiExt::NotifyTextureUpdated();
                    }
                    ImGuiDom::SetItemTooltip("Forces texture cache invalidation and redraw");

                    ImGuiDom::EndTabItem();
                }

                if (ImGuiDom::BeginTabItem("Metrics & Benchmark")) {
                    bool dedup = ImGuiExt::IsFrameDeduplicationEnabled();
                    bool reactive = ImGuiExt::IsReactiveMode();

                    ImGuiDom::Text("FPS: %.1f (Total Frame: %.2f ms)", metrics.display_fps, metrics.display_cpu_frame_ms);
                    ImGuiDom::Text("CPU Work Time: %.2f ms / frame", metrics.display_cpu_work_ms);
                    ImGuiDom::Text("Render/Present: %.2f ms", metrics.display_gpu_frame_ms);
                    ImGuiDom::Text("CPU Usage (Process): %.2f %%", metrics.cpu_usage_percent);
                    ImGuiDom::Text("RAM Working Set: %.2f MB (Peak: %.2f MB)", metrics.ram_working_set_mb, metrics.ram_peak_working_set_mb);
                    if (dedup) {
                        ImGuiDom::Text("Deduplication: %llu frames skipped", (unsigned long long)metrics.display_skipped_frames);
                    }

                    ImGuiDom::Separator();
                    if (bench.running) {
                        ImGuiDom::TextColored(ImVec4(1, 1, 0, 1), "Benchmarking '%s'... (%.1fs left)",
                            bench.name.c_str(),
                            bench.duration_sec - std::chrono::duration<double>(std::chrono::steady_clock::now() - bench.start_time).count());
                    } else {
                        if (ImGuiDom::Button("Run 5s Idle Benchmark")) {
                            std::string label = reactive ? (dedup ? "Stage 2: Reactive+Dedup - Idle (5s)" : "Stage 1: Reactive - Idle (5s)") : "Stage 0: Continuous - Idle (5s)";
                            bench.Start(label, 5.0);
                        }
                        ImGuiDom::SetItemTooltip("Runs 5-second baseline benchmark without user input");

                        ImGuiDom::SameLine();
                        if (ImGuiDom::Button("Run 5s Active Benchmark")) {
                            std::string label = reactive ? (dedup ? "Stage 2: Reactive+Dedup - Mouse Motion (5s)" : "Stage 1: Reactive - Mouse Motion (5s)") : "Stage 0: Continuous - Mouse Motion (5s)";
                            bench.Start(label, 5.0);
                        }
                        ImGuiDom::SetItemTooltip("Synthesizes continuous mouse motion to test peak throughput");

                        if (ImGuiDom::Button("Run 5s Vector Benchmark")) {
                            use_vector_backend = true;
                            ImGuiExt::SetVectorInterception(true);
                            bench.Start("Stage 3: ThorVG Vector - Mouse Motion (5s)", 5.0);
                        }
                        ImGuiDom::SetItemTooltip("Benchmarks pure vector ThorVG primitive rendering");
                    }
                    ImGuiDom::EndTabItem();
                }

                if (ImGuiDom::BeginTabItem("Channels & Status")) {
                    ImGuiDom::Text("Real-Time Signal Channels:");
                    if (ImGuiDom::BeginTable("SignalChannels", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                        ImGuiDom::TableSetupColumn("Channel");
                        ImGuiDom::TableSetupColumn("Type");
                        ImGuiDom::TableSetupColumn("Rate");
                        ImGuiDom::TableHeadersRow();

                        ImGuiDom::TableNextRow();
                        ImGuiDom::TableNextColumn(); ImGuiDom::Text("CH-1 (Sine)");
                        ImGuiDom::TableNextColumn(); ImGuiDom::Text("Hardware ADC");
                        ImGuiDom::TableNextColumn(); ImGuiDom::Text("100 kS/s");

                        ImGuiDom::TableNextRow();
                        ImGuiDom::TableNextColumn(); ImGuiDom::Text("CH-2 (Pulse)");
                        ImGuiDom::TableNextColumn(); ImGuiDom::Text("Synthetic");
                        ImGuiDom::TableNextColumn(); ImGuiDom::Text("50 kS/s");

                        ImGuiDom::EndTable();
                    }
                    ImGuiDom::Separator();
                    static int selected_preset = 0;
                    static const char* presets[] = { "Default (Laboratory)", "High Speed Transients", "Audio Band (20Hz-20kHz)", "Low Power Telemetry" };
                    ImGuiDom::Text("Oscilloscope Preset Profile:");
                    ImGuiDom::ListBox("Presets", &selected_preset, presets, 4);
                    ImGuiDom::EndTabItem();
                }

                if (ImGuiDom::BeginTabItem("Appearance & Notes")) {
                    ImGuiDom::Text("Waveform Trace Color:");
                    static float trace_color[3] = { 0.0f, 0.95f, 0.8f };
                    ImGuiDom::ColorEdit3("Trace Color", trace_color);

                    ImGuiDom::Separator();
                    ImGuiDom::Text("Session Notes & Annotation:");
                    static char session_notes[512] = "Observation: Signal stable at 1kHz.\nNoise floor: -65dB.\nStatus: Nominal.";
                    ImGuiDom::InputTextMultiline("Notes", session_notes, sizeof(session_notes), ImVec2(0, 80));
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

                if (ImGuiDom::BeginTabItem("Layout")) {
                    if (ImGuiDom::Button("Reset Layout to Default")) {
                        ImGui::SetWindowPos("ImGui Vector Backend - Controls & Metrics", ImVec2(pad, pad));
                        ImGui::SetWindowSize("ImGui Vector Backend - Controls & Metrics", ImVec2(hud_w, hud_h));
                        ImGui::SetWindowPos("Dear ImGui Demo", ImVec2(demo_x, pad));
                        ImGui::SetWindowSize("Dear ImGui Demo", ImVec2(demo_w, half_h));
                        ImGui::SetWindowPos("Real-Time Oscilloscope & Signal Monitor", ImVec2(demo_x, pad + half_h + pad));
                        ImGui::SetWindowSize("Real-Time Oscilloscope & Signal Monitor", ImVec2(demo_w, osc_h));
                    }
                    ImGuiDom::SetItemTooltip("Snaps all windows to their initial grid layout");
                    ImGuiDom::EndTabItem();
                }

                ImGuiDom::EndTabBar();
            }
            ImGuiDom::End();
        }

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

        SDL_SetRenderDrawColor(renderer, 31, 31, 36, 255);
        SDL_RenderClear(renderer);

        if (use_vector_backend) {
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
