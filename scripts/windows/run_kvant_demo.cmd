@echo off
setlocal
cd /d "%~dp0\..\.."

set "KVANT_BIN="
if exist "build\Release\kvant_demo.exe" set "KVANT_BIN=build\Release\kvant_demo.exe"
if exist "build\Debug\kvant_demo.exe" (
    if not defined KVANT_BIN set "KVANT_BIN=build\Debug\kvant_demo.exe"
)

if not defined KVANT_BIN (
    echo [WARN] kvant_demo.exe not found. Building target first...
    call "%~dp0build_kvant.cmd" Debug
    if errorlevel 1 (
        echo [ERROR] Build failed, cannot launch kvant_demo.
        pause
        exit /b 1
    )
    if exist "build\Debug\kvant_demo.exe" set "KVANT_BIN=build\Debug\kvant_demo.exe"
    if exist "build\Release\kvant_demo.exe" set "KVANT_BIN=build\Release\kvant_demo.exe"
)

if not defined KVANT_BIN (
    echo [ERROR] kvant_demo.exe binary still not found.
    pause
    exit /b 1
)

echo ========================================================
echo  Launching Kvant Vector GUI Demo
echo  Executable: %KVANT_BIN%
echo ========================================================
start "" "%KVANT_BIN%" %*
endlocal
