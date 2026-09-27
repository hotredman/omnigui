@echo off
setlocal
cd /d "%~dp0\.."

if not exist "dist\web\index.html" (
    echo [WARN] WebAssembly distribution not found in dist\web\
    echo [INFO] Running build_wasm.cmd first...
    call "%~dp0build_wasm.cmd"
    if errorlevel 1 (
        echo [ERROR] WebAssembly build failed. Cannot preview.
        pause
        exit /b 1
    )
)

echo ========================================================
echo  Launching OmniGUI Web Showcase Local Server
echo  Opening browser at http://localhost:8080/index.html ...
echo ========================================================

start "" "http://localhost:8080/index.html"

python -m http.server 8080 --directory dist\web
endlocal
