#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

if [ ! -f "dist/web/index.html" ]; then
    echo "[WARN] WebAssembly distribution not found in dist/web/"
    echo "[INFO] Running build_wasm.sh first..."
    bash scripts/build_wasm.sh
fi

echo "========================================================"
echo " Launching OmniGUI Web Showcase Local Server"
echo " Serving dist/web at http://localhost:8080/index.html"
echo "========================================================"

if command -v python3 &> /dev/null; then
    python3 -m http.server 8080 --directory dist/web
elif command -v python &> /dev/null; then
    python -m http.server 8080 --directory dist/web
else
    echo "Python not detected. Please serve dist/web using any HTTP server."
fi
