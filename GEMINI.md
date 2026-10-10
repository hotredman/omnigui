# Repository Development Guidelines & Rules

## 1. Language Rules
- **Commit Messages**: All git commit messages (both summary title and body description) MUST be written in English.
- **Code Comments**: All source code comments, docstrings, Doxygen blocks, inline notes, and technical annotations in code files MUST be written in English.
- **User Communication**: Responses in conversation should align with the user's language (e.g. Russian), but all committed code, in-code comments, and git commit logs must strictly use English.

## 2. Git Workflow
- **Explicit Commit Approval**: NEVER create a git commit without an explicit instruction from the user (such as `"комить"` or `"commit"`).
- **English Commit Logs**: All commit messages must follow standard English conventional format.

## 3. Architecture & Code Quality
- **Focus on Dear ImGui Enhancements**: The repository is strictly focused on Dear ImGui core enhancements: ThorVG vector rasterizer (`renderer_thorvg`), Web DOM/HTML headless bridge (`imgui_dom`), frame deduplication (`frame_dedup`), and reactive event loop (`event_loop`).
- **Test Integrity**: Ensure all automated tests (`test_text_metrics`, `visual_diff`, `test_frame_dedup`, `test_lttb`, `test_modal`, and `run_tests.cmd` / `run_tests.sh`) pass 100% before finishing tasks.
