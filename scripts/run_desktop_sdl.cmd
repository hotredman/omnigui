@echo off
setlocal
cd /d "%~dp0\.."

if not exist "build\Release\demo.exe" (
    echo [WARN] demo.exe not found. Building project first...
    call "%~dp0build.cmd"
    if errorlevel 1 (
        echo [ERROR] Build failed, cannot launch demo.
        pause
        exit /b 1
    )
)

echo ========================================================
echo  Launching OmniGUI: ImGui + Standard SDL3 (Desktop)
echo  Canonical Dear ImGui raster triangles & texture atlas
echo ========================================================
start "" "build\Release\demo.exe" --backend=sdl %*
endlocal
