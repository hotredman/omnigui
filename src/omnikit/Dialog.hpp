#pragma once

#include "omnikit/Button.hpp"
#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"
#include "omnikit/Utf8Text.hpp"
#include <imgui.h>
#include <string>

namespace OmniKit {

// Confirmation dialog options (designated initializers):
//
//     if (ConfirmDialog(showDelete, {.title = "Delete run?", .message = "This cannot be undone."})) { ... }
struct ConfirmDialogOptions {
    std::string title;
    std::string message;
    std::string detail;                              // additional line with accent color
    const char* confirmLabel = "Confirm";
    UiVariant   variant      = UiVariant::Danger;    // confirmation button color variant
    Icon        icon         = Icon::None;
};

// Modal confirmation dialog for dangerous or irreversible actions (Delete, Reset).
// The application holds the "open" state (bool, window identity derived from its address).
// Returns true on the frame when the operator confirms.
inline bool ConfirmDialog(bool& isOpen, const ConfirmDialogOptions& options)
{
    ImGui::PushID(&isOpen);
    static constexpr const char* kName = "##confirm_dialog";

    if (isOpen && !ImGui::IsPopupOpen(kName)) {
        ImGui::OpenPopup(kName);
    }

    bool confirmed = false;
    const UiTheme& theme = UiTheme::Get();
    float scale = theme.GetScale();

    ImGui::SetNextWindowSize(ImVec2(440.0f * scale, 0.0f), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * scale, 16.0f * scale));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, theme.CornerRadius());

    if (ImGui::BeginPopupModal(kName, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        // Title
        ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", options.title.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Message
        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "%s", options.message.c_str());

        // Details
        if (!options.detail.empty()) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(theme.palette.accent).Value);
            ImGui::TextWrapped("%s", options.detail.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Actions
        float btnW = 110.0f * scale;
        float totalBtnsW = btnW * 2.0f + 10.0f * scale;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalBtnsW - 18.0f * scale);

        if (Button({.label = "Cancel", .variant = UiVariant::Secondary, .width = btnW / scale})) {
            isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0f, 10.0f * scale);

        if (Button({.label = options.confirmLabel, .variant = options.variant, .icon = options.icon, .width = btnW / scale})) {
            confirmed = true;
            isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopID();
    return confirmed;
}

// Prompt dialog options
struct PromptDialogOptions {
    std::string title;
    std::string prompt;                              // explanatory text above input field
    const char* confirmLabel = "Confirm";
    UiVariant   variant      = UiVariant::Primary;
    Icon        icon         = Icon::None;
};

// Modal text input dialog (Duplicate, Create, Rename).
// value is read upon opening and updated upon confirmation.
// Returns true on the frame when the operator confirms.
inline bool PromptDialog(bool& isOpen, std::string& value, const PromptDialogOptions& options)
{
    ImGui::PushID(&isOpen);
    static constexpr const char* kName = "##prompt_dialog";

    if (isOpen && !ImGui::IsPopupOpen(kName)) {
        ImGui::OpenPopup(kName);
    }

    bool confirmed = false;
    const UiTheme& theme = UiTheme::Get();
    float scale = theme.GetScale();

    ImGui::SetNextWindowSize(ImVec2(450.0f * scale, 0.0f), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * scale, 16.0f * scale));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, theme.CornerRadius());

    if (ImGui::BeginPopupModal(kName, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        // Title
        ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", options.title.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Prompt
        ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "%s", options.prompt.c_str());
        ImGui::Spacing();

        // Input field
        static char buf[256];
        if (ImGui::IsWindowAppearing()) {
            CopyTruncated(buf, sizeof(buf), value);
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::PushItemWidth(-1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f * scale, 6.0f * scale));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.CornerRadius());

        bool enterPressed = ImGui::InputText("##prompt_input", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::PopStyleVar(2);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Actions
        float btnW = 120.0f * scale;
        float totalBtnsW = btnW * 2.0f + 10.0f * scale;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalBtnsW - 18.0f * scale);

        if (Button({.label = "Cancel", .variant = UiVariant::Secondary, .width = btnW / scale})) {
            isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0f, 10.0f * scale);

        bool canConfirm = (buf[0] != '\0');
        if (Button({.label = options.confirmLabel, .variant = options.variant, .icon = options.icon,
                    .width = btnW / scale, .disabled = !canConfirm}) || (enterPressed && canConfirm)) {
            value = buf;
            confirmed = true;
            isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopID();
    return confirmed;
}

} // namespace OmniKit
