@echo off
setlocal
cd /d "%~dp0\..\.."

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
echo  Launching OmniGUI Demo (Default: ThorVG Vector Backend)
echo  Tip: Use scripts\windows\run_desktop_sdl.cmd for standard SDL3
echo       Use scripts\windows\run_web_dom.cmd for Web DOM Server
echo ========================================================
start "" "build\Release\demo.exe" %*
endlocal
