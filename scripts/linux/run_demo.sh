#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/../.."

DEMO_BIN=""
for candidate in build/demo build/Release/demo; do
    if [ -f "$candidate" ]; then DEMO_BIN="$candidate"; break; fi
done

if [ -z "$DEMO_BIN" ]; then
    echo "[WARN] demo binary not found. Building project first..."
    "$(dirname "$0")/build.sh"
    for candidate in build/demo build/Release/demo; do
        if [ -f "$candidate" ]; then DEMO_BIN="$candidate"; break; fi
    done
fi

if [ -z "$DEMO_BIN" ]; then
    echo "[ERROR] Build failed, demo binary not found."
    exit 1
fi

echo "========================================================"
echo " Launching OmniGUI Demo (Default: ThorVG Vector Backend)"
echo " Tip: scripts/linux/run_desktop_sdl.sh for standard SDL3, scripts/linux/run_web_dom.sh for Web DOM Server"
echo "========================================================"

exec "$DEMO_BIN" "$@"