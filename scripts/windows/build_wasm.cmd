@echo off
setlocal
cd /d "%~dp0\..\.."

echo ========================================================
echo  Building OmniGUI WebAssembly Targets
echo  - demo_thorvg.html (ImGui + ThorVG Vector in WebGL Canvas)
echo  - demo_sdl.html    (ImGui + SDL3 Triangles in WebGL Canvas)
echo  - demo_dom.html    (ImGui + Native HTML5 DOM Bridge, No Canvas)
echo ========================================================

REM 1. Activate emsdk environment
set EMSDK_FOUND=0
where emcc >nul 2>&1
if %errorlevel% equ 0 (
    set EMSDK_FOUND=1
) else if exist "tools\emsdk\emsdk_env.bat" (
    call "tools\emsdk\emsdk_env.bat"
    set EMSDK_FOUND=1
)

if %EMSDK_FOUND% equ 0 (
    echo [WARN] Emscripten SDK not detected in PATH or tools\emsdk.
    echo [INFO] Launching automatic emsdk setup...
    call "%~dp0setup_emsdk.cmd"
    if errorlevel 1 (
        echo [ERROR] Emscripten SDK setup failed. Cannot build WebAssembly.
        pause
        exit /b 1
    )
    call "tools\emsdk\emsdk_env.bat"
)

REM 2. Generate web shells
node "%~dp0..\generate_dom_shell.mjs"

REM 3. Configure CMake with emcmake
if not exist "build_wasm" mkdir build_wasm
echo [INFO] Configuring CMake with emcmake...
call emcmake cmake -B build_wasm -S . -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo [ERROR] emcmake configuration failed.
    pause
    exit /b 1
)

REM 4. Compile WASM targets
echo [INFO] Compiling WebAssembly targets (demo_thorvg, demo_sdl, demo_dom)...
call cmake --build build_wasm --config Release
if errorlevel 1 (
    echo [ERROR] WebAssembly build failed.
    pause
    exit /b 1
)

REM 5. Assemble web distribution in dist/web/
if not exist "dist\web" mkdir dist\web
copy /y "web\index.html" "dist\web\index.html" >nul
copy /y "build_wasm\demo_thorvg.*" "dist\web\" >nul
copy /y "build_wasm\demo_sdl.*" "dist\web\" >nul
copy /y "build_wasm\demo_dom.*" "dist\web\" >nul

echo ========================================================
echo  WebAssembly build succeeded!
echo  Output files located in: dist\web\
echo   - dist\web\index.html
echo   - dist\web\demo_thorvg.html (ThorVG Vector Canvas)
echo   - dist\web\demo_sdl.html    (SDL3 Triangles Canvas)
echo   - dist\web\demo_dom.html    (Native HTML5 DOM)
echo ========================================================
echo  Run scripts\windows\run_wasm.cmd to preview in browser!
endlocal
