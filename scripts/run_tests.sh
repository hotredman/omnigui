#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

if [ ! -d build ]; then
    echo "[WARN] build/ not found. Building project first..."
    "$(dirname "$0")/build.sh"
fi

echo "========================================================"
echo " Running tests (ctest)"
echo "========================================================"

ctest --test-dir build -C Release --output-on-failure