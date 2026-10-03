cmake_minimum_required(VERSION 3.20)
project(omnigui CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    src/core/units/Units.cpp
    src/ui/components/Badge.cpp
    src/ui/components/Button.cpp
    src/ui/components/Card.cpp
    src/ui/components/Carousel.cpp
    src/ui/components/Combo.cpp
    src/ui/components/ConfirmModal.cpp
    src/ui/components/ContentArea.cpp
    src/ui/components/ContextMenu.cpp
    src/ui/components/DeviceStatus.cpp
    src/ui/components/DisabledScope.cpp
    src/ui/components/EditableLabel.cpp
    src/ui/components/FilterBar.cpp
    src/ui/components/ColumnLayout.cpp
    src/ui/components/GridLayout.cpp
    src/ui/components/Bar.cpp
    src/ui/components/Header.cpp
    src/ui/components/Icon.cpp
    src/ui/components/Indicator.cpp
    src/ui/components/InputField.cpp
    src/ui/components/ItemRow.cpp
    src/ui/components/Text.cpp
    src/ui/components/List.cpp
    src/ui/components/Panel.cpp
    src/ui/components/PresetGrid.cpp
    src/ui/components/SearchInput.cpp
    src/ui/components/Sidebar.cpp
    src/ui/components/SidePanel.cpp
    src/ui/components/StatusBar.cpp
    src/ui/components/TabBar.cpp
    src/ui/components/Tag.cpp
    src/ui/components/ThemePalette.cpp
    src/ui/components/Toggle.cpp
    src/ui/components/Toolbar.cpp
    src/ui/components/ToolButton.cpp
    src/ui/components/UiTheme.cpp
    src/ui/components/ValueDisplay.cpp
    src/ui/components/charts/AnalysisChart.cpp
    src/ui/components/charts/ChartCore.cpp
    src/ui/components/charts/Decimate.cpp
    src/ui/components/charts/RealtimeChart.cpp
)

add_library(omnikit STATIC ${OMNIKIT_SOURCES})
# C++20 нужен ради designated initializers в опциях компонентов: Button("Save", {.variant = ...});
# остальной код (в т.ч. сторонний) остаётся на 17
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

# ========================================================
#  Desktop vs WebAssembly (Emscripten) Target Matrix
# ========================================================
if (NOT EMSCRIPTEN)
    # --- Desktop Demo Executable ---
    add_executable(demo
        examples/demo/main.cpp
        examples/demo/OmniKitShowcase.cpp
    )
    target_link_libraries(demo PRIVATE imthorgui)

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

    add_executable(test_httplib
        tests/test_httplib.cpp
    )
    target_link_libraries(test_httplib PRIVATE imthorgui)

    add_executable(test_dom
        tests/test_dom.cpp
    )
    target_link_libraries(test_dom PRIVATE imthorgui)

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
        examples/demo/OmniKitShowcase.cpp
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
        examples/demo/OmniKitShowcase.cpp
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
        examples/demo/OmniKitShowcase.cpp
    )
    target_compile_definitions(demo_dom PRIVATE OMNI_BUILD_WASM_DOM=1)
    target_link_libraries(demo_dom PRIVATE imthorgui)
    set_target_properties(demo_dom PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "${DOM_WASM_FLAGS}"
    )
endif()
