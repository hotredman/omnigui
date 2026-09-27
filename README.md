# OmniGUI

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![ImGui](https://img.shields.io/badge/fork-imgui--hooked-orange.svg)](https://github.com/hotredman/imgui-hooked)
[![ThorVG](https://img.shields.io/badge/vector-ThorVG-purple.svg)](https://github.com/thorvg/thorvg)
[![SDL3](https://img.shields.io/badge/window-SDL3-red.svg)](https://github.com/libsdl-org/SDL)
[![WebAssembly](https://img.shields.io/badge/target-WebAssembly-green.svg)](https://webassembly.org/)

**OmniGUI** is a universal multi-backend immediate-mode GUI framework powered by [**imgui-hooked**](https://github.com/hotredman/imgui-hooked). It enables Dear ImGui code to run across every presentation paradigm: **infinite-DPI vector curves (ThorVG)**, **canonical GPU triangles (SDL3)**, and **native semantic HTML5 DOM (CSS/Web)**.

---

## 🌟 Architectural Presentation Matrix

```
                       ┌───────────────────────────────┐
                       │    OmniGUI (imgui-hooked)     │
                       └──────────────┬────────────────┘
                                      │
           ┌──────────────────────────┼──────────────────────────┐
           ▼                          ▼                          ▼
 1. ImGui + ThorVG          2. ImGui + SDL3            3. ImGui + DOM
    (Vector Canvas)            (GPU Triangles)            (Semantic HTML5)
    ├── Desktop (Native)       ├── Desktop (Native)       ├── Web (Remote Server)
    └── Web (WASM WebGL)       └── Web (WASM WebGL)       └── Web (WASM Direct DOM)
```

| Mode | Platform | Presentation Engine | Key Advantages |
|---|---|---|---|
| **1. ImGui + ThorVG** | **Desktop** | Vector Hooks $\rightarrow$ ThorVG $\rightarrow$ SDL3 | Infinite DPI crispness, smooth anti-aliased curves, zero font pixelation. |
| | **Web (WASM)** | ThorVG $\rightarrow$ WebGL2 Canvas | Crisp vector rendering on Retina / 4K mobile & desktop browsers. |
| **2. ImGui + SDL3** | **Desktop** | Native `SDL_Renderer` | Canonical Dear ImGui triangle streaming and font texture atlas. |
| | **Web (WASM)** | Dear ImGui $\rightarrow$ WebGL2 Canvas | Standard Dear ImGui WebAssembly baseline port. |
| **3. ImGui + DOM** | **Web (Remote)** | C++ Daemon $\rightarrow$ HTTP/WS $\rightarrow$ DOM | Remote UI for headless servers, Linux daemons, robotics. 15 KB client. |
| | **Web (WASM)** | **Direct JS DOM Bridge (Zero Canvas)** | **No Canvas!** C++ mutates live HTML5 DOM (`<div>`, `<button>`, CSS). F12 inspectable, selectable text, 0% CPU idle. |

---

## 🚀 Key Features

* **Non-Invasive Vector Interception:** Powered by [`imgui-hooked`](https://github.com/hotredman/imgui-hooked) with `IMGUI_DRAWLIST_HOOK_*` macros, intercepting geometry before tessellation.
* **True Native HTML5 DOM Backend:** Generates actual semantic HTML elements (`<button>`, `<input>`, `<progress>`, `<svg>`) synchronized with full-duplex WebSocket or in-browser WASM bridge.
* **Reactive Event Loop (Zero-CPU Idle):** Eliminates 100% busy-loop spinning; only updates when input events arrive or timers trigger.
* **LTTB (Largest Triangle Three Buckets) Decimation:** High-performance real-time oscilloscope downsampling 100k+ points to 800 px in under 0.15 ms.
* **100.00% Coordinate & Style Parity:** Automated CDP test suite auditing coordinate bounds and computed CSS colors matching Dear ImGui `StyleColorsDark`.

---

## ⚡ Quick Launch (Desktop)

Double-click any of the launcher scripts in [`scripts/`](scripts/) to run:

* **`scripts\run_desktop_thorvg.cmd`** &mdash; Launches Desktop with ThorVG Vector Backend.
* **`scripts\run_desktop_sdl.cmd`** &mdash; Launches Desktop with Canonical SDL3 Renderer.
* **`scripts\run_web_dom.cmd`** &mdash; Launches Web DOM server and automatically opens browser at `http://localhost:8080`.
* **`scripts\run_demo.cmd`** &mdash; Universal launcher (defaults to ThorVG).

*(Linux / macOS bash equivalents `.sh` are also available)*.

### Command Line Flags

```bash
# Desktop with ThorVG vector renderer (default)
demo.exe --backend=thorvg

# Desktop with canonical SDL3 triangle rasterizer
demo.exe --backend=sdl

# Headless Web DOM server with automatic browser launch
demo.exe --backend=dom --open-browser

# Custom port and software driver
demo.exe --backend=dom --port 9000 --software
```

---

## 🌐 WebAssembly (WASM) Compilation

OmniGUI provides automated scripts to compile all three web presentations:

```bash
# 1. Automatic Emscripten toolchain setup (installs to tools/emsdk)
scripts\setup_emsdk.cmd

# 2. Build demo_thorvg.html, demo_sdl.html, demo_dom.html into dist/web/
scripts\build_wasm.cmd

# 3. Preview web showcase in browser at http://localhost:8080
scripts\run_wasm.cmd
```

---

## 🛠️ Building from Source

### Prerequisites
* CMake 3.20+
* C++17 compliant compiler (MSVC 2022, GCC 11+, or Clang 13+)
* Git with submodules

### Clone & Build (Desktop)

```bash
git clone --recurse-submodules https://github.com/hotredman/omnigui.git
cd omnigui

# Configure and compile with CMake
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run automated tests
ctest --test-dir build -C Release --output-on-failure
```

---

## 🧪 Testing & Validation Suite

OmniGUI includes a comprehensive multi-tier test suite:

1. **`test_text_metrics`** &mdash; Sub-pixel text metric parity across Latin, Cyrillic, digits, and symbols (average drift 0.04 px).
2. **`test_frame_dedup`** &mdash; Verification of frame deduplication in both vector and triangle mesh modes.
3. **`test_lttb`** &mdash; LTTB downsampling accuracy and peak preservation benchmarks.
4. **`test_modal`** &mdash; Modal dimming background ordering verification.
5. **`test_dom`** &mdash; 16-step unit test verifying DOM tree serialization, two-way event synchronization, stacking order, and `IDomTransport` bridges.
6. **`tests/e2e/e2e_runner.mjs`** &mdash; End-to-end headless browser test using Chrome DevTools Protocol (CDP) auditing 7,800+ element pairs with 100.00% coordinate parity and 100.0% visual style parity.

---

## 📄 License

OmniGUI and its custom components are licensed under the MIT License. See [LICENSE](LICENSE) for details.
Dear ImGui is Copyright (c) 2014-2026 Omar Cornut.
ThorVG is Copyright (c) 2020-2026 ThorVG Authors.
SDL3 is Copyright (c) 1997-2026 Sam Lantinga.
