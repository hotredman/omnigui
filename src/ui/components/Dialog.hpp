#pragma once

#include "ui/components/Button.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <string>

// Параметры диалога подтверждения (designated initializers):
//
//     if (ConfirmDialog(showDelete, {.title = "Delete run?", .message = "This cannot be undone."})) { ... }
struct ConfirmDialogOptions {
    std::string title;
    std::string message;
    std::string detail;                              // дополнительная строка акцентным цветом
    const char* confirmLabel = "Confirm";
    UiVariant   variant      = UiVariant::Danger;    // цвет подтверждающей кнопки
    Icon        icon         = Icon::None;
};

// Модальное диалоговое окно подтверждения опасных или необратимых действий (Удаление, Сброс).
// Состояние «открыто» хранит приложение (bool, идентичность окна — его адрес).
// Возвращает true в кадре, когда оператор подтвердил
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

// Параметры диалога ввода текста
struct PromptDialogOptions {
    std::string title;
    std::string prompt;                              // поясняющая строка над полем
    const char* confirmLabel = "Confirm";
    UiVariant   variant      = UiVariant::Primary;
    Icon        icon         = Icon::None;
};

// Модальное диалоговое окно ввода текста (Дублирование, Создание, Переименование).
// value читается при открытии и записывается при подтверждении.
// Возвращает true в кадре, когда оператор подтвердил
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
            strncpy_s(buf, sizeof(buf), value.c_str(), _TRUNCATE);
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
