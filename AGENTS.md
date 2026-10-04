# Repository Development Guidelines & Rules

## 1. Language Rules
- **Commit Messages**: All git commit messages (both summary title and body description) MUST be written in English.
- **Code Comments**: All source code comments, docstrings, Doxygen blocks, inline notes, and technical annotations in code files MUST be written in English.
- **User Communication**: Responses in conversation should align with the user's language (e.g. Russian), but all committed code, in-code comments, and git commit logs must strictly use English.

## 2. Git Workflow
- **Explicit Commit Approval**: NEVER create a git commit without an explicit instruction from the user (such as `"комить"` or `"commit"`).
- **English Commit Logs**: All commit messages must follow standard English conventional format.

## 3. Architecture & Code Quality
- **Clean Architecture & Component Autonomy**: Domain and specialized subsystems (e.g., `image/`, `charts/`) must be autonomous modules depending only on Dear ImGui (`imgui.h`) and standard C++20. They must never directly include `UiTheme.hpp` or call `UiTheme::Get()`.
- **Adapter Pattern**: Use standalone style descriptors (`ChartStyle`, `ImageViewerStyle`, `ImageHistogramStyle`) with `Dark()` / `Light()` presets, and bridge `UiTheme` design tokens via adapter functions (`MakeChartStyle`, `MakeImageViewerStyle`, `MakeImageHistogramStyle`).
- **OmniKit Design System**: Application screens and demo pages must use OmniKit high-level components and avoid raw `ImGui::` widget calls.
- **Test Integrity**: Ensure all automated tests (`test_components`, `test_lttb`, and `run_tests.cmd` / `run_tests.sh`) pass 100% before finishing tasks.
