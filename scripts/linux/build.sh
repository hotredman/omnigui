#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/../.."

echo "========================================================"
echo " Building imthorgui (Release)"
echo "========================================================"

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

echo
echo "Build succeeded. Binaries are in build/ (or build/Release/)."