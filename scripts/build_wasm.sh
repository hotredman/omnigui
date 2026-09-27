#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

echo "========================================================"
echo " Building OmniGUI WebAssembly Targets"
echo " - demo_thorvg.html (ImGui + ThorVG Vector in WebGL Canvas)"
echo " - demo_sdl.html    (ImGui + SDL3 Triangles in WebGL Canvas)"
echo " - demo_dom.html    (ImGui + Native HTML5 DOM Bridge, No Canvas)"
echo "========================================================"

# 1. Activate emsdk environment
if command -v emcc &> /dev/null; then
    echo "[INFO] Emscripten compiler (emcc) found in PATH."
elif [ -f "tools/emsdk/emsdk_env.sh" ]; then
    source tools/emsdk/emsdk_env.sh
else
    echo "[WARN] Emscripten SDK not detected. Launching setup..."
    bash scripts/setup_emsdk.sh
    source tools/emsdk/emsdk_env.sh
fi

# 2. Generate web shells
node scripts/generate_dom_shell.mjs

# 3. Configure CMake with emcmake
mkdir -p build_wasm
echo "[INFO] Configuring CMake with emcmake..."
emcmake cmake -B build_wasm -S . -DCMAKE_BUILD_TYPE=Release

# 4. Compile WASM targets
echo "[INFO] Compiling WebAssembly targets..."
cmake --build build_wasm --config Release

# 5. Assemble web distribution in dist/web/
mkdir -p dist/web
cp -f web/index.html dist/web/index.html
cp -f build_wasm/demo_thorvg.* dist/web/ 2>/dev/null || true
cp -f build_wasm/demo_sdl.* dist/web/ 2>/dev/null || true
cp -f build_wasm/demo_dom.* dist/web/ 2>/dev/null || true

echo "========================================================"
echo " WebAssembly build succeeded!"
echo " Output files located in: dist/web/"
echo "========================================================"
echo " Run scripts/run_wasm.sh to preview in browser!"
