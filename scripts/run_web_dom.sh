#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

DEMO_BIN=""
if [ -f "build/demo" ]; then
    DEMO_BIN="build/demo"
elif [ -f "build/Release/demo" ]; then
    DEMO_BIN="build/Release/demo"
fi

if [ -z "$DEMO_BIN" ]; then
    echo "[WARN] demo binary not found. Building project first..."
    mkdir -p build
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release --target demo
    if [ -f "build/demo" ]; then
        DEMO_BIN="build/demo"
    elif [ -f "build/Release/demo" ]; then
        DEMO_BIN="build/Release/demo"
    else
        echo "[ERROR] Build failed, demo binary not found."
        exit 1
    fi
fi

echo "========================================================"
echo " Launching OmniGUI: ImGui + Web DOM Server"
echo " Browser will open automatically at http://localhost:8080"
echo "========================================================"

exec "$DEMO_BIN" --backend=dom --open-browser "$@"
