@echo off
setlocal
cd /d "%~dp0\..\.."

echo ========================================================
echo  OmniGUI: Setting up Emscripten SDK (emsdk)
echo ========================================================

if not exist "tools" mkdir tools

if not exist "tools\emsdk\.git" (
    echo [INFO] Cloning emsdk repository into tools\emsdk...
    git clone https://github.com/emscripten-core/emsdk.git tools\emsdk
    if errorlevel 1 (
        echo [ERROR] Failed to clone emsdk repository.
        pause
        exit /b 1
    )
) else (
    echo [INFO] Existing emsdk repository detected in tools\emsdk.
)

pushd tools\emsdk
echo [INFO] Fetching latest emsdk toolchain versions...
call git pull

echo [INFO] Installing latest Emscripten compiler (this may take a few minutes)...
call emsdk.bat install latest
if errorlevel 1 (
    echo [ERROR] emsdk install failed.
    popd
    pause
    exit /b 1
)

echo [INFO] Activating latest Emscripten compiler...
call emsdk.bat activate latest
if errorlevel 1 (
    echo [ERROR] emsdk activate failed.
    popd
    pause
    exit /b 1
)

popd

echo ========================================================
echo  Emscripten SDK setup completed successfully!
echo  You can now run: scripts\windows\build_wasm.cmd
echo ========================================================
endlocal
