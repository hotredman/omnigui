@echo off
setlocal
cd /d "%~dp0\..\.."

set CONFIG=Release
if "%~1"=="Debug" set CONFIG=Debug
if "%~1"=="debug" set CONFIG=Debug

echo ========================================================
echo  Building Kvant Library ^& Demo (%CONFIG%)
echo ========================================================

if not exist build (
    echo [INFO] Generating CMake build configuration...
    cmake -B build -S .
    if errorlevel 1 (
        echo [ERROR] CMake configuration failed!
        pause
        exit /b 1
    )
)

echo [INFO] Compiling target: kvant_demo (%CONFIG%)...
cmake --build build --config %CONFIG% --target kvant_demo
if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================================
echo  Build succeeded!
echo  Binary located in: build\%CONFIG%\kvant_demo.exe
echo ========================================================
endlocal
