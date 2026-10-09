cmake_minimum_required(VERSION 3.20)
project(omnigui CXX C)

# Demo application sources: shell and pages (common for desktop and WASM targets)
set(DEMO_APP_SOURCES
    examples/demo/OmniKitShowcase.cpp
    examples/demo/pages/TestRunPage.cpp
    examples/demo/pages/CardsPage.cpp
    examples/demo/pages/ImageViewerPage.cpp
)

# Uniform C++20 standard across the project (designated initializers in component options)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Single-configuration generators (Ninja, Makefiles): default to Release
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Source files in UTF-8: MSVC otherwise assumes ANSI code page
if (MSVC)
    add_compile_options(/utf-8 /Zc:__cplusplus)
endif()

# --- SDL3 (submodule) ---
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/SDL)

# --- ThorVG ---
add_subdirectory(third_party/thorvg)

# --- ImGui (submodule) ---
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_compile_definitions(imgui PUBLIC "IMGUI_USER_CONFIG=\"imgui_ext/my_imgui_config.h\"")
target_include_directories(imgui PUBLIC
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL3::SDL3-static)
if (WIN32 AND NOT EMSCRIPTEN)
    target_link_libraries(imgui PUBLIC psapi)
endif()

# --- Threads (Cross-Platform) ---
if (NOT EMSCRIPTEN)
    find_package(Threads REQUIRED)
endif()

# --- OmniKit: Modern High-Level UI Component Library & Design System ---
set(OMNIKIT_SOURCES
    src/core/Assets.cpp
    src/core/ProcessStats.cpp
    src/core/units/Units.cpp
    src/omnikit/Badge.cpp
    src/omnikit/Button.cpp
    src/omnikit/Card.cpp
    src/omnikit/TextSuggest.cpp
    src/omnikit/CardFields.cpp
    src/omnikit/Carousel.cpp
    src/omnikit/Combo.cpp
    src/omnikit/ConfirmModal.cpp
    src/omnikit/ContentArea.cpp
    src/omnikit/ContextMenu.cpp
    src/omnikit/DeviceStatus.cpp
    src/omnikit/DisabledScope.cpp
    src/omnikit/EditableLabel.cpp
    src/omnikit/FilterBar.cpp
    src/omnikit/ColumnLayout.cpp
    src/omnikit/GridLayout.cpp
    src/omnikit/Bar.cpp
    src/omnikit/Header.cpp
    src/omnikit/Icon.cpp
    src/omnikit/Indicator.cpp
    src/omnikit/InputField.cpp
    src/omnikit/ItemRow.cpp
    src/omnikit/Text.cpp
    src/omnikit/List.cpp
    src/omnikit/Panel.cpp
    src/omnikit/PresetGrid.cpp
    src/omnikit/SearchInput.cpp
    src/omnikit/Sidebar.cpp
    src/omnikit/SidePanel.cpp
    src/omnikit/StatusBar.cpp
    src/omnikit/TabBar.cpp
    src/omnikit/Tag.cpp
    src/omnikit/ThemePalette.cpp
    src/omnikit/Toggle.cpp
    src/omnikit/Toolbar.cpp
    src/omnikit/ToolButton.cpp
    src/omnikit/UiTheme.cpp
    src/omnikit/ValueDisplay.cpp
    src/omnikit/ImageViewerControls.cpp
    src/omnikit/charts/AnalysisChart.cpp
    src/omnikit/charts/ChartCore.cpp
    src/omnikit/charts/Decimate.cpp
    src/omnikit/charts/RealtimeChart.cpp
    src/omnikit/image/ImageColormap.cpp
    src/omnikit/image/WindowLevel.cpp
    src/omnikit/image/ImageHistogram.cpp
    src/omnikit/image/ImageViewer.cpp
)

add_library(omnikit STATIC ${OMNIKIT_SOURCES})
# C++20 required for designated initializers in component options: Button({.label = "Save"});
target_compile_features(omnikit PUBLIC cxx_std_20)
target_include_directories(omnikit PUBLIC src)
target_link_libraries(omnikit PUBLIC imgui)

# --- OmniGUI Library (Recorder + Renderer + DOM Backend + OmniKit) ---
add_library(omnigui STATIC
    src/imgui_ext/recorder.cpp
    src/imgui_ext/renderer_thorvg.cpp
    src/imgui_dom/dom_context.cpp
    src/imgui_dom/dom_server.cpp
    src/imgui_dom/dom_hook.cpp
    src/imgui_dom/dom_transport.cpp
)
add_library(imthorgui ALIAS omnigui)
target_include_directories(omnigui PUBLIC
    src
    third_party/httplib
)
target_link_libraries(omnigui PUBLIC imgui thorvg omnikit)
if (NOT EMSCRIPTEN)
    target_link_libraries(omnigui PUBLIC Threads::Threads)
endif()
if (WIN32 AND NOT EMSCRIPTEN)
    target_link_libraries(omnigui PUBLIC ws2_32 crypt32)
endif()

# --- Kvant Autonomous Vector GUI Engine ---
set(KVANT_SOURCES
    src/kvant/Context.cpp
    src/kvant/backend/ThorCanvas.cpp
    src/kvant/graphics/Icon.cpp
    src/kvant/widgets/Button.cpp
    src/kvant/widgets/Checkbox.cpp
    src/kvant/widgets/Switch.cpp
    src/kvant/widgets/Text.cpp
)
add_library(kvant STATIC ${KVANT_SOURCES})
target_compile_features(kvant PUBLIC cxx_std_20)
target_include_directories(kvant PUBLIC src)
target_link_libraries(kvant PUBLIC thorvg)
if (NOT EMSCRIPTEN)
    target_link_libraries(kvant PUBLIC Threads::Threads)
endif()

# ========================================================
#  Desktop vs WebAssembly (Emscripten) Target Matrix
# ========================================================
if (NOT EMSCRIPTEN)
    # --- Desktop Demo Executables ---
    add_executable(demo
        examples/demo/main.cpp
        ${DEMO_APP_SOURCES}
    )
    target_link_libraries(demo PRIVATE imthorgui)

    add_executable(kvant_demo
        examples/kvant_demo/main.cpp
    )
    target_link_libraries(kvant_demo PRIVATE kvant SDL3::SDL3-static)

    # --- Tests ---
    add_executable(test_text_metrics
        tests/test_text_metrics.cpp
    )
    target_link_libraries(test_text_metrics PRIVATE imthorgui)

    add_executable(visual_diff
        tests/visual_diff.cpp
    )
    target_link_libraries(visual_diff PRIVATE imthorgui)

    add_executable(test_frame_dedup
        tests/test_frame_dedup.cpp
    )
    target_link_libraries(test_frame_dedup PRIVATE imthorgui)

    add_executable(test_lttb
        tests/test_lttb.cpp
    )
    target_link_libraries(test_lttb PRIVATE imthorgui)

    add_executable(profile_oscilloscope
        tests/profile_oscilloscope.cpp
    )
    target_link_libraries(profile_oscilloscope PRIVATE imthorgui)

    add_executable(test_modal
        tests/test_modal.cpp
    )
    target_link_libraries(test_modal PRIVATE imthorgui)

    add_executable(test_components
        tests/test_components.cpp
    )
    target_link_libraries(test_components PRIVATE imthorgui)

    add_executable(test_httplib
        tests/test_httplib.cpp
    )
    target_link_libraries(test_httplib PRIVATE imthorgui)

    add_executable(test_dom
        tests/test_dom.cpp
    )
    target_link_libraries(test_dom PRIVATE imthorgui)

    add_executable(test_kvant_core
        tests/test_kvant_core.cpp
    )
    target_link_libraries(test_kvant_core PRIVATE kvant imthorgui)

    add_executable(test_kvant_layout
        tests/test_kvant_layout.cpp
    )
    target_link_libraries(test_kvant_layout PRIVATE kvant imthorgui)

    add_executable(test_kvant_context
        tests/test_kvant_context.cpp
    )
    target_link_libraries(test_kvant_context PRIVATE kvant)

    # --- ctest: same validation suites as scripts/windows/run_tests.cmd ---
    enable_testing()
    add_test(NAME frame_dedup COMMAND test_frame_dedup)
    add_test(NAME text_metrics COMMAND test_text_metrics)
    add_test(NAME visual_diff COMMAND visual_diff)
    add_test(NAME lttb COMMAND test_lttb)
    add_test(NAME modal COMMAND test_modal)
    add_test(NAME components COMMAND test_components)
    add_test(NAME kvant_core COMMAND test_kvant_core)
    add_test(NAME kvant_layout COMMAND test_kvant_layout)
    add_test(NAME kvant_context COMMAND test_kvant_context)

else()
    # ========================================================
    #  WebAssembly (Emscripten) Targets
    # ========================================================
    set(COMMON_WASM_FLAGS
        "-sALLOW_MEMORY_GROWTH=1"
        "-sNO_EXIT_RUNTIME=1"
        "-sEXPORTED_FUNCTIONS=['_main','_omni_wasm_send_event','_malloc','_free']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8']"
    )

    set(CANVAS_WASM_FLAGS
        ${COMMON_WASM_FLAGS}
        "-sUSE_SDL=0"
        "-sMAX_WEBGL_VERSION=2"
        "-sMIN_WEBGL_VERSION=2"
        "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell_canvas.html"
    )

    # 1. Target: demo_thorvg.html (ImGui + ThorVG Vector in WebGL Canvas)
    add_executable(demo_thorvg
        examples/demo/main.cpp
        ${DEMO_APP_SOURCES}
    )
    target_compile_definitions(demo_thorvg PRIVATE OMNI_BUILD_WASM_THORVG=1)
    target_link_libraries(demo_thorvg PRIVATE imthorgui)
    set_target_properties(demo_thorvg PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "${CANVAS_WASM_FLAGS}"
    )

    # 2. Target: demo_sdl.html (ImGui + Standard SDL3 in WebGL Canvas)
    add_executable(demo_sdl
        examples/demo/main.cpp
        ${DEMO_APP_SOURCES}
    )
    target_compile_definitions(demo_sdl PRIVATE OMNI_BUILD_WASM_SDL=1)
    target_link_libraries(demo_sdl PRIVATE imthorgui)
    set_target_properties(demo_sdl PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "${CANVAS_WASM_FLAGS}"
    )

    # 3. Target: demo_dom.html (ImGui + Native HTML5 DOM Bridge, No Canvas)
    set(DOM_WASM_FLAGS
        ${COMMON_WASM_FLAGS}
        "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell_dom.html"
    )
    add_executable(demo_dom
        examples/demo/main.cpp
        ${DEMO_APP_SOURCES}
    )
    target_compile_definitions(demo_dom PRIVATE OMNI_BUILD_WASM_DOM=1)
    target_link_libraries(demo_dom PRIVATE imthorgui)
    set_target_properties(demo_dom PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "${DOM_WASM_FLAGS}"
    )
endif()
