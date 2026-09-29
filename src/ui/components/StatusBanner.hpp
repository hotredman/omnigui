#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <string>

// Семантический баннер статуса или фонового процесса
class StatusBanner {
public:
    static void Progress(const std::string& message, float progress = -1.0f) {
        const UiTheme& theme = UiTheme::Get();

        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(theme.palette.accent).Value);
        if (progress >= 0.0f) {
            ImGui::Text("%s: %.0f%%...", message.c_str(), progress * 100.0f);
        } else {
            ImGui::Text("%s...", message.c_str());
        }
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    static void Warning(const std::string& message) {
        const UiTheme& theme = UiTheme::Get();
        ImGui::PushStyleColor(ImGuiCol_Text, theme.GetVariantStyle(UiVariant::Warning).colText);
        ImGui::TextWrapped("%s", message.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
};
