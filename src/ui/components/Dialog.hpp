#pragma once

#include "ui/components/Button.hpp"
#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <string>

// Модальное диалоговое окно подтверждения опасных или необратимых действий (Удаление, Сброс)
class ConfirmDialog {
public:
    static bool Render(const char* id,
                       bool& isOpen,
                       const std::string& title,
                       const std::string& message,
                       const std::string& detail = "",
                       const char* confirmBtnText = "Confirm",
                       UiVariant confirmVariant = UiVariant::Danger,
                       Icon::Id confirmIcon = Icon::None)
    {
        if (isOpen && !ImGui::IsPopupOpen(id)) {
            ImGui::OpenPopup(id);
        }

        bool confirmed = false;
        const UiTheme& theme = UiTheme::Get();
        float scale = theme.GetScale();

        ImGui::SetNextWindowSize(ImVec2(440.0f * scale, 0.0f), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * scale, 16.0f * scale));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, theme.CornerRadius());

        if (ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }

            // Title
            ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", title.c_str());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Message
            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "%s", message.c_str());

            // Details
            if (!detail.empty()) {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImColor(theme.palette.accent).Value);
                ImGui::TextWrapped("%s", detail.c_str());
                ImGui::PopStyleColor();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Actions
            float btnW = 110.0f * scale;
            float totalBtnsW = btnW * 2.0f + 10.0f * scale;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalBtnsW - 18.0f * scale);

            if (Button("Cancel", {.variant = UiVariant::Secondary, .width = btnW / scale})) {
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine(0.0f, 10.0f * scale);

            if (Button(confirmBtnText, {.variant = confirmVariant, .icon = confirmIcon, .width = btnW / scale})) {
                confirmed = true;
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);
        return confirmed;
    }
};

// Модальное диалоговое окно ввода текста (Дублирование, Создание, Переименование)
class PromptDialog {
public:
    static bool Render(const char* id,
                       bool& isOpen,
                       const std::string& title,
                       const std::string& prompt,
                       std::string& value,
                       const char* confirmBtnText = "Confirm",
                       UiVariant confirmVariant = UiVariant::Primary,
                       Icon::Id confirmIcon = Icon::None)
    {
        if (isOpen && !ImGui::IsPopupOpen(id)) {
            ImGui::OpenPopup(id);
        }

        bool confirmed = false;
        const UiTheme& theme = UiTheme::Get();
        float scale = theme.GetScale();

        ImGui::SetNextWindowSize(ImVec2(450.0f * scale, 0.0f), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f * scale, 16.0f * scale));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, theme.CornerRadius());

        if (ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }

            // Title
            ImGui::TextColored(ImColor(theme.palette.textPrimary).Value, "%s", title.c_str());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Prompt
            ImGui::TextColored(ImColor(theme.palette.textSecondary).Value, "%s", prompt.c_str());
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

            if (Button("Cancel", {.variant = UiVariant::Secondary, .width = btnW / scale})) {
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine(0.0f, 10.0f * scale);

            bool canConfirm = (buf[0] != '\0');
            if (Button(confirmBtnText, {.variant = confirmVariant, .icon = confirmIcon, .width = btnW / scale, .disabled = !canConfirm}) || (enterPressed && canConfirm)) {
                value = buf;
                confirmed = true;
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);
        return confirmed;
    }
};
