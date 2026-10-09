#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/../.."

KVANT_BIN=""
for candidate in build/kvant_demo build/Release/kvant_demo build/Debug/kvant_demo; do
    if [ -f "$candidate" ]; then KVANT_BIN="$candidate"; break; fi
done

if [ -z "$KVANT_BIN" ]; then
    echo "[WARN] kvant_demo binary not found. Building target first..."
    "$(dirname "$0")/build_kvant.sh"
    for candidate in build/kvant_demo build/Release/kvant_demo build/Debug/kvant_demo; do
        if [ -f "$candidate" ]; then KVANT_BIN="$candidate"; break; fi
    done
fi

if [ -z "$KVANT_BIN" ]; then
    echo "[ERROR] Build failed, kvant_demo binary not found."
    exit 1
fi

echo "========================================================"
echo " Launching Kvant Vector GUI Demo"
echo " Executable: $KVANT_BIN"
echo "========================================================"

exec "$KVANT_BIN" "$@"
