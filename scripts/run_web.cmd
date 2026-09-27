@echo off
setlocal
cd /d "%~dp0\.."

if not exist "build\Release\demo.exe" (
    echo [WARN] demo.exe not found. Building project first...
    call "%~dp0build.cmd"
    if errorlevel 1 (
        echo [ERROR] Build failed, cannot launch web demo.
        pause
        exit /b 1
    )
)

echo ========================================================
echo  Launching Dear ImGui Web DOM Backend...
echo  Browser will open automatically at http://localhost:8080
echo ========================================================

"build\Release\demo.exe" --open-browser %*
endlocal
