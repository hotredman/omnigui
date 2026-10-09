#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/../.."

CONFIG="${1:-Release}"

echo "========================================================"
echo " Building Kvant Library & Demo ($CONFIG)"
echo "========================================================"

if [ ! -d "build" ]; then
    echo "[INFO] Generating CMake build configuration..."
    cmake -B build -S .
fi

echo "[INFO] Compiling target: kvant_demo ($CONFIG)..."
cmake --build build --config "$CONFIG" --target kvant_demo

echo ""
echo "========================================================"
echo " Build succeeded!"
echo "========================================================"
