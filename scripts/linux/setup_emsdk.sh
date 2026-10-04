#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/../.."

echo "========================================================"
echo " OmniGUI: Setting up Emscripten SDK (emsdk)"
echo "========================================================"

mkdir -p tools

if [ ! -d "tools/emsdk/.git" ]; then
    echo "[INFO] Cloning emsdk repository into tools/emsdk..."
    git clone https://github.com/emscripten-core/emsdk.git tools/emsdk
else
    echo "[INFO] Existing emsdk repository detected in tools/emsdk."
fi

pushd tools/emsdk > /dev/null
echo "[INFO] Fetching latest emsdk toolchain versions..."
git pull

echo "[INFO] Installing latest Emscripten compiler..."
./emsdk install latest

echo "[INFO] Activating latest Emscripten compiler..."
./emsdk activate latest

popd > /dev/null

echo "========================================================"
echo " Emscripten SDK setup completed successfully!"
echo " You can now run: scripts/linux/build_wasm.sh"
echo "========================================================"
